/* SIMT CPU.
 *
 * Copyright (c) 2025.
 *
 *
 */
#pragma once

#include <stdint.h>
//#include "qom/object.h"
#include "exec/cpu-common.h"
#include "exec/cpu-defs.h"
#include "cpu-qom.h"
#include "accel/tcg/cpu-ops.h"
//#include "hw/core/cpu.h"
//#include "exec/memory.h"

#define CPU_RESOLVING_TYPE TYPE_ZEUS_CPU

#include "simt_split.h"

enum {
    ZEUS_CPU_NR_THREADS = ZEUS_NR_THREADS,
    ZEUS_CPU_ALL_THREADS_MASK = ZEUS_ALL_THREADS_MASK,
    ZEUS_CPU_NR_GPR = 256,
    ZEUS_CPU_NR_32BIT_GPR = ZEUS_CPU_NR_GPR,
    ZEUS_CPU_NR_64BIT_GPR = ZEUS_CPU_NR_32BIT_GPR / 2,
    ZEUS_CPU_SIMD32_SZ = ZEUS_CPU_NR_THREADS * sizeof(uint32_t),
    ZEUS_CPU_SIMD64_SZ = ZEUS_CPU_NR_THREADS * sizeof(uint64_t),
};

typedef union ZeusInsnParams {
    uint32_t val;
    struct __attribute__((packed)) {
    uint8_t n0 : 1, n1 : 1, n2 : 1, n3 : 1;
    uint8_t i0 : 1, i1 : 1, i2 : 1, i3 : 1;
    uint8_t c0 : 1, c1 : 1, c2 : 1, c3 : 1;
    uint8_t bcf : 1;
    };
} ZeusInsnParams;

#if 0
// FIXME need to work around assertion that offsetof(env)==sizeof(CPUState)
#define SIMD_VEC_ATTR(count, type)  __attribute__ ((vector_size (count*sizeof(type))))

typedef uint32_t U32xTHR SIMD_VEC_ATTR(ZEUS_CPU_NR_THREADS, uint32_t);
typedef uint64_t U64xTHR SIMD_VEC_ATTR(ZEUS_CPU_NR_THREADS, uint64_t);
#else
typedef uint32_t U32xTHR[ZEUS_CPU_NR_THREADS];
typedef uint64_t U64xTHR[ZEUS_CPU_NR_THREADS];
#endif

// Regs from different threads form x86 SIMD vector:
// avx512 = {r1[thr0], r1[thr1],... r1[thr31]}
typedef union SimtCpuGpRegs {
    uint64_t r64[ZEUS_CPU_NR_64BIT_GPR][ZEUS_CPU_NR_THREADS];
    uint32_t r32[ZEUS_CPU_NR_32BIT_GPR][ZEUS_CPU_NR_THREADS];
    U64xTHR r64xThr[ZEUS_CPU_NR_64BIT_GPR];
    U32xTHR r32xThr[ZEUS_CPU_NR_32BIT_GPR];
} SimtCpuGpRegs;


typedef struct CPUArchState {
    uint64_t resetvec;
    uint64_t pc;

    uint32_t bcf; // 32 branch conditional flags, one per thread

    SimtCpuGpRegs gpr QEMU_ALIGNED(16);
    uint64_t zero[2][ZEUS_CPU_NR_THREADS];

    ZeusSimtSplits simt_splits;
    uint8_t split_id;

} CPUZeusState;

static inline ZeusSimtVergeStack* simt_stack(CPUZeusState *env) {
    return &env->simt_splits.splits[env->split_id].verge_stack;
}

/* Also known as ZeusCPU
 */
struct ArchCPU {
    CPUState parent_obj;

    CPUZeusState env;
};

// hack to show size in compile time as error message
//char checker(int);
//char check_sizeof[sizeof(CPUState)]={checker(&check_sizeof)};
//char check_sizeof[sizeof(CPUNegativeOffsetState)]={checker(&check_sizeof)};
//char check_sizeof[offsetof(CPUState, neg)]={checker(&check_sizeof)};
//char check_sizeof[offsetof(CPUState, neg_align)]={checker(&check_sizeof)};
//char check_sizeof[offsetof(ArchCPU, env)]={checker(&check_sizeof)};
//#define MY_OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
//char check_sizeof[MY_OFFSETOF(ArchCPU, env)]={checker(&check_sizeof)};

static_assert(offsetof(ArchCPU, parent_obj) == 0, "parent_obj must be first");
static_assert(offsetof(ArchCPU, env) == sizeof(CPUState), "env offset must be sizeof(CPUState)");

struct ZeusCPUClass {
    CPUClass parent_class;

    DeviceRealize parent_realize;
    ResettablePhases parent_phases;
};

// see cpu_tcg_ops.c
struct TCGCPUOps* zeus_cpu_get_tcg_ops(void);
bool zeus_cpu_has_work(CPUState *cs);
TCGTBCPUState zeus_get_tb_cpu_state(CPUState *cs);

void cpu_get_tb_cpu_state(
    CPUZeusState *env,
    target_ulong *pc,
    target_ulong *cs_base,
    uint32_t *flags
);

int cpu_mmu_index(CPUState *cs, bool ifetch);


