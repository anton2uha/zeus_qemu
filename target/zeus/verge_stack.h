/* Meta SIMT CPU Deconvergence/Reconvergence stack.
 *
 * Copyright (c) 2024 Meta Platforms, Inc.
 *
 *
 */
#pragma once

#define ZEUS_NR_THREADS 32
#define ZEUS_ALL_THREADS_MASK 0xFFFFFFFF

enum {
    ZEUS_SIMT_VERGE_HINT_NONE = 0,
    ZEUS_SIMT_VERGE_HINT_ELSE_ARM = 1 << 0,
};

enum {ZEUS_SIMT_VERGE_STACK_SIZE = 16};

typedef uint32_t simt_thread_mask_t;

typedef struct ZeusSimtVergeStackEntry {
    uint64_t pc;
    uint64_t rpc;
    simt_thread_mask_t mask;
    simt_thread_mask_t parent_mask;
    uint8_t hint;
} ZeusSimtVergeStackEntry;

typedef struct ZeusSimtVergeStack {
    ZeusSimtVergeStackEntry entry[ZEUS_SIMT_VERGE_STACK_SIZE];
    uint32_t size;
} ZeusSimtVergeStack;

static inline
bool zeus_verge_stack_empty(ZeusSimtVergeStack *stack) {
    return stack->size == 0;
}

static inline
ZeusSimtVergeStackEntry zeus_verge_stack_top(ZeusSimtVergeStack *stack) {
    g_assert(stack->size > 0);
    return stack->entry[stack->size - 1];
}

static inline
ZeusSimtVergeStackEntry* zeus_verge_stack_top_ref(ZeusSimtVergeStack *stack) {
    g_assert(stack->size > 0);
    return &stack->entry[stack->size - 1];
}

static inline
simt_thread_mask_t zeus_verge_stack_get_current_mask(ZeusSimtVergeStack *stack) {
    return zeus_verge_stack_empty(stack) ?
        ZEUS_ALL_THREADS_MASK : zeus_verge_stack_top(stack).mask;
}

static inline
void zeus_verge_stack_push(ZeusSimtVergeStack *stack,
    uint64_t pc, uint64_t rpc, uint8_t hint, simt_thread_mask_t mask, bool merge)
{
    g_assert(stack->size < (ZEUS_SIMT_VERGE_STACK_SIZE - 1));
    stack->entry[stack->size].pc = pc;
    stack->entry[stack->size].rpc = rpc;
    stack->entry[stack->size].mask = merge ?
        (zeus_verge_stack_get_current_mask(stack) & mask) : mask;
    stack->entry[stack->size].hint = hint;
    stack->size++;
}

static inline
ZeusSimtVergeStackEntry zeus_verge_stack_pop(ZeusSimtVergeStack *stack) {
    g_assert(stack->size > 0);
    stack->size--;
    return stack->entry[stack->size];
}

static inline
bool simt_thread_in_mask(simt_thread_mask_t mask, uint8_t thread_id) {
    return mask & (1 << thread_id);
}

// @lesikigor "clang loop vectorize(enable) interleave(enable)" makes simulation slower
// "clang loop unroll(full)" makes simulation faster
#if defined __GNUC__ && __GNUC__ >= 8
    #define SIMT_FOR_EACH_THREAD_PRAGMA //_Pragma("GCC unroll 128") _Pragma("GCC ivdep")
#elif defined __clang__
    #define SIMT_FOR_EACH_THREAD_PRAGMA _Pragma("clang loop unroll(full)")//_Pragma("clang loop vectorize(enable) interleave(enable)")
#else
    #define SIMT_FOR_EACH_THREAD_PRAGMA
#endif

#define SIMT_FOR_EACH_THREAD_ALGORITHM 1

#if (SIMT_FOR_EACH_THREAD_ALGORITHM == 1)

#define for_each_thread(stack) \
SIMT_FOR_EACH_THREAD_PRAGMA \
for (simt_thread_mask_t mask = zeus_verge_stack_get_current_mask(stack), \
    thr = 0, all_thr = (mask == ZEUS_ALL_THREADS_MASK); \
    thr < ZEUS_NR_THREADS; \
    thr++) if (all_thr || (mask & (1u << thr)))

#else

#define for_each_thread(stack) \
for (simt_thread_mask_t mask = zeus_verge_stack_get_current_mask(stack), \
    thr = __builtin_ctz(mask); \
    mask; \
    mask &= mask - 1, thr = __builtin_ctz(mask))

#endif