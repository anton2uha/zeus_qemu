/* SIMT CPU branch instructions.
 *
 * Copyright (c) 2025.
 *
 *
 */
#include "qemu/osdep.h"
#include "qemu/main-loop.h"
#include "cpu.h"
#include "exec/cpu-common.h"
#include "exec/cpu-interrupt.h"
#include "exec/helper-proto.h" // all helper functions prototypes

//#define DBG_VERGE_ENABLE

#ifdef DBG_VERGE_ENABLE
#define DBG_VERGE(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DBG_VERGE(fmt, ...)
#endif

enum {
    ZEUS_BRTYPE_IF = 0,
    ZEUS_BRTYPE_IF_ELSE = 1,
    ZEUS_BRTYPE_LOOP = 2
};

static
void simt_splits_add(CPUZeusState* env, uint64_t pc, uint64_t bpc, uint64_t rpc,
    simt_thread_mask_t thread_mask)
{
    uint8_t G_GNUC_UNUSED new_split_id = zeus_simt_splits_add(&env->simt_splits, pc, bpc, rpc);
    DBG_VERGE("[%u] NEW SIMT SPLIT ID:%u PC:%lx ELSE_PC:%lx RPC:%lx\n",
        env->split_id, new_split_id, pc, bpc, rpc);

    // when we back to executing this split we need PC+8
    env->simt_splits.splits[env->split_id].pc = pc + sizeof(uint64_t);
    env->simt_splits.splits[env->split_id].rpc = rpc; // if id != 0 ?

    ZeusSimtVergeStack* new_split_stack = &env->simt_splits.splits[new_split_id].verge_stack;
    zeus_verge_stack_push(new_split_stack, bpc/*env->pc*/, rpc,
        ZEUS_SIMT_VERGE_HINT_ELSE_ARM, thread_mask, false);
}

void helper_zeus__simt_diverge_if_else(CPUZeusState* env, uint32_t simt_thread_mask,
    uint32_t brtype, uint64_t bpc, uint64_t rpc)
{
    uint32_t negate = brtype >> 16;
    //brtype &= 0xFFFF;

    simt_thread_mask = negate ? ~simt_thread_mask : simt_thread_mask;

    DBG_VERGE("[%u] %u %*s IF-ELSE DIVERGE PC:%lx mask:%08x ELSE-PC:%lx RPC:%lx\n",
        env->split_id,
        simt_stack(env)->size, simt_stack(env)->size, ">",
        env->pc, simt_thread_mask, bpc, rpc);

    simt_thread_mask_t current_mask = zeus_verge_stack_get_current_mask(simt_stack(env));

    zeus_verge_stack_push(simt_stack(env), env->pc, rpc, 0, current_mask & simt_thread_mask, false);
    simt_splits_add(env, env->pc, bpc, rpc, current_mask & ~simt_thread_mask);
}

static void cpu_interrupt_exittb(CPUState *cs)
{
    cs->interrupt_request |= CPU_INTERRUPT_EXITTB;
#if 0
    //if (tcg_enabled()) {
        QEMU_IOTHREAD_LOCK_GUARD();
        cpu_interrupt(cs, CPU_INTERRUPT_EXITTB);
    //}
#endif
}

void helper_zeus__simt_if_arm_end(CPUZeusState* env)
{
    //zeus_verge_stack_pop(simt_stack(env));

    DBG_VERGE("[%u] %u %*s IF-ARM END PC:%lx mask:%08x\n",
        env->split_id,
        simt_stack(env)->size, simt_stack(env)->size, "<",
        env->pc, zeus_verge_stack_get_current_mask(simt_stack(env)));

    // When QEMU chains TBs it removes instructions were we handle Splits and Verges.
    // This interrupt prevents TB chaining (sets last_tb=NULL).
    cpu_interrupt_exittb(env_cpu(env));
}

void helper_zeus__simt_current_split_set_pc(CPUZeusState* env, uint64_t pc)
{
    DBG_VERGE("[%u] SPLIT SET PC:%lx\n", env->split_id, pc);

    env->simt_splits.splits[env->split_id].pc = pc;
}

void helper_zeus__simt_diverge_if(CPUZeusState* env, uint32_t simt_thread_mask,
   uint32_t brtype, uint64_t rpc)
{
    uint32_t negate = brtype >> 16;
    //brtype &= 0xFFFF;

    simt_thread_mask = negate ? ~simt_thread_mask : simt_thread_mask;

    DBG_VERGE("[%u] %u %*s IF DIVERGE PC:%lx mask:%08x\n",
        env->split_id,
        simt_stack(env)->size, simt_stack(env)->size, ">",
        env->pc, simt_thread_mask);

    simt_thread_mask_t current_mask = zeus_verge_stack_get_current_mask(simt_stack(env));
    zeus_verge_stack_push(simt_stack(env), env->pc, rpc, 0, current_mask & simt_thread_mask, false);
}

void helper_zeus__simt_2nd_arm(CPUZeusState* env)
{
    DBG_VERGE("[%u] BEGIN ELSE-ARM PC:%lx\n", env->split_id, env->pc);
}

uint64_t helper_zeus__simt_reconverge(CPUZeusState* env)
{
    // @lesikigor: We can jump at the divergence point from outside of if/for,
    // in such case we should not pop stack; check that top of stack entry.rpc == env->pc.
    if (zeus_verge_stack_top_ref(simt_stack(env))->rpc != env->pc) {
        DBG_VERGE("[%u] %u %*s ACCIDENT JUMP TO RECONVERGE PC:%lx, TOP PC:%lx RPC:%lx\n",
            env->split_id,
            simt_stack(env)->size, simt_stack(env)->size, "<",
            env->pc,
            zeus_verge_stack_top_ref(simt_stack(env))->pc,
            zeus_verge_stack_top_ref(simt_stack(env))->rpc);
        return 0;
    }

    __auto_type G_GNUC_UNUSED top = zeus_verge_stack_pop(simt_stack(env));

    bool is_else_arm = top.hint & ZEUS_SIMT_VERGE_HINT_ELSE_ARM;

    DBG_VERGE("[%u] %u %*s RECONVERGE PC:%lx mask:%08x BPC:%lx\n",
        env->split_id,
        simt_stack(env)->size, simt_stack(env)->size, "<",
        env->pc, zeus_verge_stack_get_current_mask(simt_stack(env)),
        top.pc);

    if (env->split_id != 0)
    {
        if (simt_stack(env)->size == 0 || is_else_arm)
        {
            DBG_VERGE("[%u] EMPTY STACK, REMOVE THIS SPLIT, RPC:%lx\n", env->split_id, top.rpc);
            zeus_simt_splits_remove(&env->simt_splits, env->split_id);
            return top.rpc; // this indicates we exit TB
        }
    }

    return 0;
}

void helper_zeus__simt_loop(CPUZeusState* env, uint32_t simt_thread_mask,
    uint32_t brtype, uint64_t rpc, uint64_t bpc)
{
    uint32_t negate = brtype >> 16;
    //brtype &= 0xFFFF;

    simt_thread_mask = !negate ? ~simt_thread_mask : simt_thread_mask;

    if (zeus_verge_stack_empty(simt_stack(env)) ||
        env->pc != zeus_verge_stack_top_ref(simt_stack(env))->pc
    ) {
        DBG_VERGE("[%u] %u %*s LOOP DIVERGE PC:%lx mask:%08x\n",
            env->split_id,
            simt_stack(env)->size, simt_stack(env)->size, ">",
            env->pc, simt_thread_mask);
        simt_thread_mask_t parent_mask = zeus_verge_stack_get_current_mask(simt_stack(env));
        zeus_verge_stack_push(simt_stack(env), env->pc, rpc, 0, simt_thread_mask, true);
        zeus_verge_stack_top_ref(simt_stack(env))->parent_mask = parent_mask;
        env->simt_splits.splits[env->split_id].pc = (simt_thread_mask == 0)? rpc:bpc;
    }
    else {
        DBG_VERGE("[%u] %u %*s LOOP REPEAT PC:%lx mask:%08x\n",
            env->split_id,
            simt_stack(env)->size, simt_stack(env)->size, ">",
            env->pc, simt_thread_mask);
        zeus_verge_stack_top_ref(simt_stack(env))->mask =
            zeus_verge_stack_top_ref(simt_stack(env))->parent_mask & simt_thread_mask;
        env->simt_splits.splits[env->split_id].pc = (simt_thread_mask == 0)? rpc:bpc;
    }
}
