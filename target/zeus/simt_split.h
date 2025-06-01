/* SIMT CPU divergence IF-ELSE split.
 *
 * Copyright (c) 2025
 *
 *
 */
#pragma once

#include "verge_stack.h"

/* Split is a IF or ELSE warp/fiber in single-thread execution.
 * Splits run in round robin, split can exit/iterrupt on a sync barrier.
 * We track current PC in every Split.
 */
typedef struct ZeusSimtSplit {
    uint64_t pc; // current/last simulated PC inside split/warp
    uint64_t rpc;
    ZeusSimtVergeStack verge_stack;
} ZeusSimtSplit;

typedef uint64_t zeus_simt_splits_bitmap_t;
#define ZEUS_CPU_SIMT_SPLITS_MAX_NR (sizeof(zeus_simt_splits_bitmap_t) * 8)

typedef struct ZeusSimtSplits {
    ZeusSimtSplit splits[ZEUS_CPU_SIMT_SPLITS_MAX_NR];
    zeus_simt_splits_bitmap_t bitmap;
} ZeusSimtSplits;

static inline
void zeus_simt_splits_init(ZeusSimtSplits* warps, uint64_t pc) {
    warps->bitmap = 1; // 1 warp always exists
    warps->splits[0].pc = pc;
    warps->splits[0].rpc = 0;
    warps->splits[0].verge_stack.size = 0;
}

static inline
uint8_t zeus_simt_splits_size(ZeusSimtSplits* warps) {
    return __builtin_popcountl(warps->bitmap);
}

#define for_each_simt_split(splits) \
for (zeus_simt_splits_bitmap_t mask = splits->bitmap, split_id = __builtin_ctzl(mask); \
    mask; \
    mask &= mask - 1, split_id = __builtin_ctzl(mask))

static inline
uint8_t zeus_simt_splits_find_available(ZeusSimtSplits* splits) {
    return __builtin_ctzl(~splits->bitmap);
}

// Next function for traversing splits, can be used for round robin.
static inline
uint8_t zeus_simt_splits_next(ZeusSimtSplits* splits, uint8_t split_id) {
    g_assert(splits->bitmap >= 1);
    uint8_t next_split_id = ((splits->bitmap >> (split_id + 1)) == 0) ?
        __builtin_ctzl(splits->bitmap):
        split_id + 1 + __builtin_ctzl(splits->bitmap >> (split_id + 1));
    g_assert(splits->bitmap & (1ul << next_split_id));
    return next_split_id;
}

static inline
uint8_t zeus_simt_splits_add(ZeusSimtSplits* warps, uint64_t pc, uint64_t else_pc, uint64_t rpc)
{
    g_assert(zeus_simt_splits_size(warps) < ZEUS_CPU_SIMT_SPLITS_MAX_NR);

    uint8_t new_split_id = zeus_simt_splits_find_available(warps);
    warps->bitmap |= 1ul << new_split_id;
    ZeusSimtSplit* new_split = &warps->splits[new_split_id];
    new_split->pc = else_pc;
    new_split->rpc = rpc;

    return new_split_id;
}

static inline
void zeus_simt_splits_remove(ZeusSimtSplits* warps, uint8_t split_id) {
    warps->bitmap &= ~(1ul << split_id);
}
