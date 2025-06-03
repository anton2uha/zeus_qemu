/* SIMT CPU compare and set conditional flag instructions.
 *
 * Copyright (c) 2025
 *
 *
 */
#include "qemu/osdep.h"
#include "cpu.h"
#include "exec/helper-proto.h" // all helper functions prototypes

#define ZEUS_CMP_SET(OP_, env, bcf_, d_, s1_, s2_, params_) \
    uint32_t* bcf = bcf_; \
    uint64_t *d = d_, *s1 = s1_, *s2 = s2_; \
    ZeusInsnParams params = {.val = params_}; \
    for_each_thread(simt_stack(env)) { d[thr] = s1[thr] OP_ s2[thr]; } \
    if (params.bcf) { \
        *bcf = 0; \
        for_each_thread(simt_stack(env)) { \
            *bcf |= ((d[thr] != 0)? 1:0) << thr; \
        } \
    }

void helper_zeus__iseq2_s64(CPUZeusState* env, void* bcf_, void* d_, void* s1_, void* s2_, uint32_t params_) {
    ZEUS_CMP_SET(==, env, bcf_, d_, s1_, s2_, params_)
}
void helper_zeus__islt2_s64(CPUZeusState* env, void* bcf_, void* d_, void* s1_, void* s2_, uint32_t params_) {
    ZEUS_CMP_SET(<, env, bcf_, d_, s1_, s2_, params_)
}
void helper_zeus__islte2_s64(CPUZeusState* env, void* bcf_, void* d_, void* s1_, void* s2_, uint32_t params_) {
    ZEUS_CMP_SET(<=, env, bcf_, d_, s1_, s2_, params_)
}

void helper_zeus__iseqi2_u64(CPUZeusState* env, void* bcf_, void* d_, void* s1_, uint32_t s2_, uint32_t params_)
{
    //printf("execute SEQUI\n");
    uint32_t* bcf = bcf_;
    uint64_t* d = d_;
    uint64_t* s1 = s1_;
    uint64_t  s2 = s2_;
    ZeusInsnParams params = {.val = params_};

    for_each_thread(simt_stack(env)) {
        d[thr] = s1[thr] == s2;
    }

    if (params.bcf) {
        *bcf = 0;
        for_each_thread(simt_stack(env)) {
            *bcf |= ((d[thr] != 0)? 1:0) << thr;
        }
        //printf("BCF %x\n", *bcf);
    }
}

void helper_zeus__islti2_u64(CPUZeusState* env, void* bcf_, void* d_, void* s1_, uint32_t s2_, uint32_t params_)
{
    //printf("execute SLTUI\n");
    uint32_t* bcf = bcf_;
    uint64_t* d = d_;
    uint64_t* s1 = s1_;
    uint64_t  s2 = s2_;
    ZeusInsnParams params = {.val = params_};

    for_each_thread(simt_stack(env)) {
        d[thr] = s1[thr] < s2;
    }

    if (params.bcf) {
        *bcf = 0;
        for_each_thread(simt_stack(env)) {
            *bcf |= ((d[thr] != 0)? 1:0) << thr;
        }
        //printf("SLTUI BCF %x\n", *bcf);
    }
}

void helper_zeus__isltei2_u64(CPUZeusState* env, void* bcf_, void* d_, void* s1_, uint32_t s2_, uint32_t params_)
{
    //printf("execute SLTEUI\n");
    uint32_t* bcf = bcf_;
    uint64_t* d = d_;
    uint64_t* s1 = s1_;
    uint64_t  s2 = s2_;
    ZeusInsnParams params = {.val = params_};

    for_each_thread(simt_stack(env)) {
        d[thr] = s1[thr] <= s2;
    }

    if (params.bcf) {
        *bcf = 0;
        for_each_thread(simt_stack(env)) {
            *bcf |= ((d[thr] != 0)? 1:0) << thr;
        }
        //printf("SLTEUI BCF %x\n", *bcf);
    }
}
