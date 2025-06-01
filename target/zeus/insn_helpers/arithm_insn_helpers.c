/* SIMT CPU arithmetic instructions.
 *
 * Copyright (c) 2025.
 *
 *
 */
#include "qemu/osdep.h"
#include "cpu.h"
#include "exec/helper-proto.h" // all helper functions prototypes

static uint64_t sign_extend32(uint32_t v) {
    return ((v & (1 << 31))? (~0ul << 32):0ul) | v;
}

void helper_mtia_zeus__lui(CPUZeusState* env, void* d_, uint32_t val, uint32_t hints)
{
    //printf("execute LUI\n");
    uint64_t* d = d_;

    for_each_thread(simt_stack(env)) {
        d[thr] = (((uint64_t)val) << 32) | (d[thr] & 0x00000000FFFFFFFF);
    }
}

void helper_mtia_zeus__iadd2(CPUZeusState* env, void* d_, void* s1_, void* s2_, uint32_t hints)
{
    //printf("execute ADD\n");
    uint64_t* d = d_;
    uint64_t* s1 = s1_;
    uint64_t* s2 = s2_;

    for_each_thread(simt_stack(env)) {
        d[thr] = s1[thr] + s2[thr];
    }
}

void helper_mtia_zeus__iaddi2(CPUZeusState* env, void* d_, void* s1_, uint64_t s2, uint32_t params_)
{
    //printf("execute ADDI\n");
    int64_t* d = d_;
    int64_t* s1 = s1_;
    ZeusInsnParams params = {.val = params_};
    int64_t sign = params.n1? -1l : 1l;

    for_each_thread(simt_stack(env)) {
        d[thr] = sign*s1[thr] + s2;
    }
}

void helper_mtia_zeus__imadd2(CPUZeusState* env, void* d_, void* s1_, void* s2_, void* s3_, uint32_t hints)
{
    //printf("execute MUL\n");
    uint64_t* d = d_;
    uint64_t* s1 = s1_;
    uint64_t* s2 = s2_;
    uint64_t* s3 = s3_;

    for_each_thread(simt_stack(env)) {
        d[thr] = s1[thr] * s2[thr] + s3[thr];
    }
}

/*
void helper_mtia_zeus__muli(CPUZeusState* env, void* d_, void* s1_, uint32_t s2_, uint32_t params_)
{
    //printf("execute MULI\n");
    int64_t* d = d_;
    int64_t* s1 = s1_;
    int64_t s2 = sign_extend32(s2_);

    for_each_thread(simt_stack(env)) {
        d[thr] = s1[thr] * s2;
    }
}

void helper_mtia_zeus__muliu(CPUZeusState* env, void* d_, void* s1_, uint32_t s2_, uint32_t params_)
{
    //printf("execute MULIU\n");
    uint64_t* d = d_;
    uint64_t* s1 = s1_;
    uint64_t s2 = (uint64_t)s2_;

    for_each_thread(simt_stack(env)) {
        d[thr] = s1[thr] * s2;
    }
}
*/
void helper_mtia_zeus__andi2(CPUZeusState* env, void* d_, void* s1_, uint32_t s2_, uint32_t params_)
{
    //printf("execute ANDI\n");
    uint64_t* d = d_;
    uint64_t* s1 = s1_;
    uint64_t s2 = (uint64_t)s2_;

    for_each_thread(simt_stack(env)) {
        d[thr] = s1[thr] & s2;
    }
}

void helper_mtia_zeus__ori2(CPUZeusState* env, void* d_, void* s1_, uint32_t s2_, uint32_t params_)
{
    //printf("execute ORI\n");
    uint64_t* d = d_;
    uint64_t* s1 = s1_;
    uint64_t s2 = (uint64_t)s2_;

    for_each_thread(simt_stack(env)) {
        d[thr] = s1[thr] | s2;
    }
}

void helper_mtia_zeus__xori2(CPUZeusState* env, void* d_, void* s1_, uint32_t s2_, uint32_t params_)
{
    //printf("execute XORI\n");
    uint64_t* d = d_;
    uint64_t* s1 = s1_;
    uint64_t s2 = (uint64_t)s2_;

    for_each_thread(simt_stack(env)) {
        d[thr] = s1[thr] ^ s2;
    }
}

/*
void helper_mtia_zeus__ishli2(CPUZeusState* env, void* d_, void* s1_, uint32_t s2_, uint32_t params_)
{
    //printf("execute SLLI\n");
    uint64_t* d = d_;
    uint64_t* s1 = s1_;
    uint8_t s2 = s2_ & 0b111111;

    for_each_thread(simt_stack(env)) {
        d[thr] = s1[thr] << s2;
    }
}

void helper_mtia_zeus__srli(CPUZeusState* env, void* d_, void* s1_, uint32_t s2_, uint32_t params_)
{
    //printf("execute SRLI\n");
    uint64_t* d = d_;
    uint64_t* s1 = s1_;
    uint8_t s2 = s2_ & 0b111111;

    for_each_thread(simt_stack(env)) {
        d[thr] = s1[thr] >> s2;
    }
}

void helper_mtia_zeus__srai(CPUZeusState* env, void* d_, void* s1_, uint32_t s2_, uint32_t params_)
{
    //printf("execute SRLI\n");
    int64_t* d = d_;
    int64_t* s1 = s1_;
    uint8_t s2 = s2_ & 0b111111;

    for_each_thread(simt_stack(env)) {
        d[thr] = s1[thr] >> s2;
    }
}

void helper_mtia_zeus__rori2(CPUZeusState* env, void* d_, void* s1_, uint32_t s2_, uint32_t params_)
{
    //printf("execute RORI\n");
    uint64_t* d = d_;
    uint64_t* s1 = s1_;
    uint8_t s2 = s2_ & 0b111111;

    for_each_thread(simt_stack(env)) {
        d[thr] = (s1[thr] >> s2) | (s1[thr] << (64 - s2));
    }
}
*/
