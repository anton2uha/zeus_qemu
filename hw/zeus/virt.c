#include "qemu/osdep.h"
#include "qemu/units.h"
#include "qapi/error.h"
#include "hw/boards.h"
#include "hw/qdev-core.h"
#include "qemu/log.h"
#include "hw/loader.h"
#include "elf.h"
#include "hw/qdev-properties.h"
#include "system/reset.h"
#include "target/zeus/cpu.h"
#include "hw/zeus/virt.h"
#include "system/address-spaces.h"
#include <inttypes.h>

static
void zeus_load_program(ZeusVirtMachineState *board, const char *filename)
{
    g_assert_nonnull(filename);

    hwaddr pentry G_GNUC_UNUSED = 0;

    ssize_t prg_size G_GNUC_UNUSED = load_elf_as(
        /* filename     */ filename,
        /* elf_note     */ NULL,
        /* translate    */ NULL,
        /* trans opaque */ NULL,
        /* pentry       */ &pentry,
        /* lowaddr      */ NULL,
        /* highaddr     */ NULL,
        /* pflags       */ NULL,
        /* big endian   */ 0,
        /* elf_machine  */ EM_NONE,
        /* clear_lsb    */ 1,
        /* data_swab    */ 0,
        /* address space*/ &address_space_memory
        ///* load_rom     */ false
    );

    g_assert(prg_size > 0);

    qemu_log("Program %s loaded, size:%ld entry:%"PRIx64"\n", filename, prg_size, pentry);
}

static void zeus_cpu_realize(
    ZeusCPU *cpu,
    const char *prg_filename
)
{
    uint64_t resetvec = 0;

    if (prg_filename) {
        union {Elf64_Ehdr h64; Elf32_Ehdr h32;} hdr;
        bool is64;
        load_elf_hdr(prg_filename, &hdr, &is64, NULL); //FIXME check error if !elf
        resetvec = hdr.h64.e_entry;
    }
    printf("CPU reset vector: %lx\n", resetvec);
    qdev_prop_set_uint64(DEVICE(cpu), "resetvec", resetvec);

    qemu_register_reset((void*)cpu_reset, cpu);
    qdev_realize(DEVICE(cpu), NULL, &error_abort);
}

static void zeus_virt_machine_init(MachineState *machine)
{
    ZeusVirtMachineState *board = ZEUS_VIRT_MACHINE(machine);

    uint64_t ram_size = 1 * GiB;

    // User can request larger RAM size from command line
    machine->ram_size = (ram_size > machine->ram_size)?
        ram_size :
        machine->ram_size;

    memory_region_init_ram_nomigrate(
        &board->dram,      // DRAM installed on the board
        NULL,              // no owner
        "dram",
        machine->ram_size,
        &error_abort
    );
    
    memory_region_add_subregion(
        get_system_memory(),
        0x1000,
        &board->dram
    );

    object_initialize_child(
        OBJECT(board),
        "cpu",
        &board->cpu,
        TYPE_ZEUS_CPU
    );

    zeus_cpu_realize(&board->cpu, machine->kernel_filename);

    if (machine->kernel_filename) {
        zeus_load_program(board, machine->kernel_filename);
    }
}

static void zeus_virt_machine_class_init(ObjectClass *oc, const void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc             = "Zeus Virt Board";
    mc->init             = zeus_virt_machine_init;
    mc->min_cpus         = 1;
    mc->max_cpus         = mc->min_cpus;
    mc->default_cpus     = mc->min_cpus;
    mc->default_cpu_type = ZEUS_CPU_TYPE_NAME("base");
    mc->default_ram_size = 1 * GiB;
}

static const TypeInfo types[] = {
    {
        .name          = TYPE_ZEUS_VIRT_MACHINE,
        .parent        = TYPE_MACHINE,
        .class_init    = zeus_virt_machine_class_init,
        .instance_size = sizeof(ZeusVirtMachineState),
    },
};

DEFINE_TYPES(types);
