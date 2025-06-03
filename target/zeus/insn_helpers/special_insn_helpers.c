/* SIMT CPU special instructions.
 *
 * Copyright (c) .
 *
 *
 */
#include "qemu/osdep.h"
#include "cpu.h"
#include "exec/helper-proto.h" // all helper functions prototypes
//#include "isa/toolchain/isa/insn_simrsvd.h"

/*FIXME typedef union SimMessage {
    uint64_t val;
    InsnSimRsvd event;
} SimMessage;*/

void helper_mtia_zeus__simrsvd(CPUZeusState *env, uint64_t message)
{
#if 0
    //printf("SIM message:%lx\n", message);
    SimMessage sim = {.val = message};

    switch (sim.event.cmd) {
        case SIMRSVD_CMD_EXIT: {
            switch (sim.event.subcmd) {
                case SIMRSVD_EXIT_SIMULATION: {
                    printf("EXIT SIMULATION on SIMRSVD instruction\n");
                    exit(0);
                } break;
            }
        } break;
        case SIMRSVD_CMD_ASSERT: {
            switch (sim.event.subcmd) {
                case SIMRSVD_ASSERT_REGS_EQ: {
                    uint8_t reg1_id = sim.event.assert_regs_eq.r1;
                    uint8_t reg2_id = sim.event.assert_regs_eq.r2;
                    for_each_thread(simt_stack(env)) {
                        uint64_t val1 = env->gpr.r64[reg1_id >> 1][thr];
                        uint64_t val2 = env->gpr.r64[reg2_id >> 1][thr];
                        if (val1 != val2) {
                            printf("thread:%u assert r[%u]=%lx == r[%u]=%lx\n", thr, reg1_id, val1, reg2_id, val2);
                            exit(1);
                        }
                    }
                } break;
            }
        } break;
        case SIMRSVD_CMD_PRINT: {
            switch (sim.event.subcmd) {
                case SIMRSVD_PRINT_REG_GPR: {
                    uint8_t rid = sim.event.print_reg_gpr.rid & ~1;
                    uint64_t* reg_val = env->gpr.r64[rid >> 1];
                    printf("GPR %u:%u", rid + 1, rid);
                    for (uint8_t thread_id = 0; thread_id < ZEUS_CPU_NR_THREADS; thread_id++) {
                        uint64_t val = reg_val[thread_id];
                        if (thread_id % 4 == 0) printf("\n");
                        printf(" %2u %08x:%08x", thread_id, (uint32_t)(val >> 32), (uint32_t)val);
                    }
                    printf("\n");
                } break;
            }
        } break;
    }
#endif
}

enum {CSRR_THREAD_ID = 4};

void helper_mtia_zeus__csrr(CPUZeusState* env, void* d_, uint32_t csr_id, uint32_t hints)
{
    uint32_t* dst = d_;
    switch (csr_id) {
        case CSRR_THREAD_ID:
            for (uint8_t thread_id = 0; thread_id < ZEUS_CPU_NR_THREADS; thread_id++) {
                dst[thread_id * 2] = thread_id;
            }
            break;
        default: g_assert_not_reached(); break;
    }
}
