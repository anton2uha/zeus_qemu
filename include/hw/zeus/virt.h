#pragma once

#include "qemu/osdep.h"
#include "target/zeus/cpu.h"

/* Machine name string.
 *
 * `$ qemu-system-zeus -M virt`
 */
#define TYPE_ZEUS_VIRT_MACHINE MACHINE_TYPE_NAME("virt")
 
#define ZEUS_VIRT_MACHINE(obj) \
    OBJECT_CHECK(ZeusVirtMachineState, (obj), TYPE_ZEUS_VIRT_MACHINE)

/* Zeus Virt QEMU machine.
 *
 */
typedef struct ZeusVirtMachineState {
    MachineState parent_obj;

    // DRAM installed on the board.
    MemoryRegion dram;
    
    ZeusCPU cpu;

} ZeusVirtMachineState;
