/* SIMT CPU internal functions.
 *
 * Copyright (c) 2025
 *
 *
 */
#pragma once

bool zeus_cpu_tlb_fill(
    CPUState *cs, vaddr address, int size,
    MMUAccessType access_type, int mmu_idx,
    bool probe, uintptr_t retaddr);
