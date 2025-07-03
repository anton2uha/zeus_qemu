/**
 * @brief SIMT CPU TCG functions
 * @copyright 2025
 */
#include "qemu/osdep.h"
#include "cpu.h"
#include "cpu-qom.h"
#include "internals.h"
#include "translate.h"
#include "qapi/error.h"
#include "accel/tcg/cpu-ops.h"

static void zeus_cpu_synchronize_from_tb(
    CPUState *cs,
    const TranslationBlock *tb
)
{
    ZeusCPU *cpu = ZEUS_CPU(cs);
    CPUZeusState *env = &cpu->env;

    (void)env;
    printf("FIXME zeus_cpu_synchronize_from_tb\n");
    //tcg_debug_assert(!(cs->tcg_cflags & CF_PCREL));
    //env->pc = tb->pc;
}

static void zeus_cpu_restore_state_to_opc(
    CPUState *cs,
    const TranslationBlock *tb,
    const uint64_t *data
)
{
    ZeusCPU *cpu = ZEUS_CPU(cs);
    CPUZeusState *env = &cpu->env;

    (void)env;
    printf("FIXME zeus_cpu_restore_state_to_opc\n");
    //env->pc = data[0];
}

static bool zeus_cpu_exec_interrupt(CPUState *cs, int interrupt_request)
{
    //g_assert_not_reached();
#if 0
    if (interrupt_request & CPU_INTERRUPT_HARD) {
        ZeusCPU *cpu = ZEUS_CPU(cs);
        CPUZeusState *env = &cpu->env;

        if (cpu_zeus_hw_interrupts_enabled(env) &&
            cpu_zeus_hw_interrupts_pending(env)) {
            /* Raise it */
            cs->exception_index = EXCCODE_INT;
            zeus_cpu_do_interrupt(cs);
            return true;
        }
    }
#endif
    return true;//false;
}

static void zeus_cpu_do_interrupt(CPUState *cs)
{
    g_assert_not_reached();
}

static void zeus_cpu_do_transaction_failed(
    CPUState *cs,
    hwaddr physaddr,
    vaddr addr,
    unsigned size,
    MMUAccessType access_type,
    int mmu_idx,
    MemTxAttrs attrs,
    MemTxResult response,
    uintptr_t retaddr
)
{
    ZeusCPU *cpu = ZEUS_CPU(cs);
    CPUZeusState *env = &cpu->env;

    (void)env;
    g_assert_not_reached();

    /*if (access_type == MMU_INST_FETCH) {
        do_raise_exception(env, EXCCODE_ADEF, retaddr);
    } else {
        do_raise_exception(env, EXCCODE_ADEM, retaddr);
    }*/
}

static struct TCGCPUOps g_instance_zeus_cpu_tcg_ops = {
    .initialize            = zeus_cpu_translate_init,
    .translate_code        = zeus_cpu_translate_code,
    .synchronize_from_tb   = zeus_cpu_synchronize_from_tb,
    .restore_state_to_opc  = zeus_cpu_restore_state_to_opc,

    .tlb_fill              = zeus_cpu_tlb_fill,
    .cpu_exec_interrupt    = zeus_cpu_exec_interrupt,
    .do_interrupt          = zeus_cpu_do_interrupt,
    .do_transaction_failed = zeus_cpu_do_transaction_failed,

    .mttcg_supported       = true,
};

struct TCGCPUOps* zeus_cpu_get_tcg_ops(void) {
    return &g_instance_zeus_cpu_tcg_ops;
}
