/* MTIA SIMT CPU MMU TLB helpers.
 *
 * Copyright (c) 2024 Meta Platforms, Inc.
 *
 *
 */
#include "qemu/osdep.h"

#include "cpu.h"
#include "internals.h"
#include "exec/exec-all.h"

bool zeus_cpu_tlb_fill(
    CPUState *cs, vaddr address, int size,
    MMUAccessType access_type, int mmu_idx,
    bool probe, uintptr_t retaddr
)
{
    address &= TARGET_PAGE_MASK;

    hwaddr paddr = address;
    int prot = PAGE_READ | PAGE_WRITE | PAGE_EXEC;
    uint64_t page_size = TARGET_PAGE_SIZE;

    tlb_set_page(cs, address, paddr, prot, mmu_idx, page_size);

    return true;
}