/**
 * @brief SIMT CPU
 * @copyright 2025
 */
#include "qemu/osdep.h"
#include "cpu.h"
#include "qapi/error.h"

#define MMU_CODE_IDX 0
#define MMU_DATA_IDX 0

int cpu_mmu_index(CPUZeusState *env, bool ifetch)
{
    return ifetch ? MMU_CODE_IDX : MMU_DATA_IDX;
}

void cpu_get_tb_cpu_state(
    CPUZeusState *env,
    target_ulong *pc,
    target_ulong *cs_base,
    uint32_t *pflags)
{
    *pc = env->pc;
    *cs_base = 0;

    uint32_t flags = 0;
    *pflags = flags;

    zeus_simt_splits_bitmap_t splits_bitmap = env->simt_splits.bitmap;
    //printf("cpu_get_tb_cpu_state pc:%lx splits:%lx\n", env->pc, splits_bitmap);

    if (splits_bitmap > 1) {
        uint8_t next_split_id = zeus_simt_splits_next(&env->simt_splits, env->split_id);
        env->split_id = next_split_id;
        env->pc = env->simt_splits.splits[env->split_id].pc;
        *pc = env->pc;
        //printf("cpu_get_tb_cpu_state next_split_id:%u pc:%lx\n", env->split_id, env->pc);
    }
    else {
        env->split_id = 0;
        env->pc = env->simt_splits.splits[env->split_id].pc;
        *pc = env->pc;
        //printf("cpu_get_tb_cpu_state next_split_id:%u pc:%lx\n", env->split_id, env->pc);
    }
}

static void mtia_zeus_cpu_reset_hold(Object *obj)
{
    CPUState *cpu_state = CPU(obj);
    ZeusCPU *cpu = ZEUS_CPU(cpu_state);
    ZeusCPUClass *cpu_class = ZEUS_CPU_GET_CLASS(cpu);
    CPUZeusState *env = &cpu->env;

    if (cpu_class->parent_phases.hold) {
        cpu_class->parent_phases.hold(obj);
    }

    env->pc = env->resetvec;

    zeus_simt_splits_init(&env->simt_splits, env->pc);
    cpu->env.split_id = 0;
    // Do not have to, but always having this entry on the stack
    // will strongly bias branch `if(!empty_stack())` and improve performance.
    zeus_verge_stack_push(simt_stack(&cpu->env), ~0ul, 0, 0, ZEUS_CPU_ALL_THREADS_MASK, false);
}

static void mtia_zeus_cpu_cpu_realize(DeviceState *dev, Error **errp)
{
    CPUState *cpu_state = CPU(dev);
    ZeusCPUClass *zeus_class = ZEUS_CPU_GET_CLASS(dev);
    Error *local_err = NULL;

    cpu_exec_realizefn(cpu_state, &local_err);
    if (local_err != NULL) {
        error_propagate(errp, local_err);
        return;
    }

    //FIXME zeus_cpu_register_gdb_regs_for_features(cs);

    cpu_reset(cpu_state);
    qemu_init_vcpu(cpu_state);

    zeus_class->parent_realize(dev, errp);
}

static void mtia_zeus_cpu_define_properties(Object *obj)
{
    ZeusCPU *cpu = ZEUS_CPU(obj);

    cpu->env.resetvec = 0x1000; //FIXME
    object_property_add_uint64_ptr(obj, "resetvec", &cpu->env.resetvec, OBJ_PROP_FLAG_READWRITE);
    object_property_set_description(obj, "resetvec", "CPU cold start reset vector");
}

static void mtia_zeus_cpu_instance_init(Object *obj)
{
    ZeusCPU *cpu = ZEUS_CPU(obj);

    cpu_set_cpustate_pointers(cpu);

    mtia_zeus_cpu_define_properties(obj);

    zeus_simt_splits_init(&cpu->env.simt_splits, ~0ul);
    cpu->env.split_id = 0;
}

static void mtia_zeus_cpu_instance_finalize(Object *obj)
{
}

#include "hw/core/sysemu-cpu-ops.h"

static const struct SysemuCPUOps mtia_zeus_cpu_sysemu_ops = {
    .get_phys_page_debug = NULL, //zeus_cpu_get_phys_page_debug,
};

static void mtia_zeus_cpu_class_init(ObjectClass *klass, void *data)
{
    ZeusCPUClass *zeus_class = ZEUS_CPU_CLASS(klass);
    CPUClass      *cpu_class = CPU_CLASS(klass);
    DeviceClass   *dev_class = DEVICE_CLASS(klass);
    ResettableClass *rst_class = RESETTABLE_CLASS(klass);

    device_class_set_parent_realize(
        dev_class,
        mtia_zeus_cpu_cpu_realize,
        &zeus_class->parent_realize
    );
    resettable_class_set_parent_phases(
        rst_class, NULL,
        mtia_zeus_cpu_reset_hold,
        NULL,
        &zeus_class->parent_phases
    );

    cpu_class->sysemu_ops = &mtia_zeus_cpu_sysemu_ops;

#ifdef CONFIG_TCG
    cpu_class->tcg_ops = mtia_zeus_cpu_get_tcg_ops();
#else
    #error "MTIA SIMT CPU supports only TCG"
#endif
}

static const TypeInfo zeus_cpu_types[] = {
    {
        .name              = TYPE_ZEUS_CPU,
        .parent            = TYPE_CPU,
        .class_init        = mtia_zeus_cpu_class_init,
        .class_size        = sizeof(ZeusCPUClass),
        .instance_init     = mtia_zeus_cpu_instance_init,
        .instance_finalize = mtia_zeus_cpu_instance_finalize,
        .instance_size     = sizeof(ZeusCPU),
        .abstract          = false, //@lesikigor check
    },
};

DEFINE_TYPES(zeus_cpu_types);
