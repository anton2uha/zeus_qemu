

typedef void (*zeus_compare_and_set_imm_helper_t)(
    TCGv_env env, TCGv_ptr bcf, TCGv_ptr d,
    TCGv_ptr s1, TCGv_i32 s2, TCGv_i32 params);

static bool gen_zeus_compare_and_set_imm(
    DisasContext *ctx,
    arg_rdrs1imm_gbcf *a,
    zeus_compare_and_set_imm_helper_t gen_helper_func
)
{
    uint32_t bcf_ofs = offsetof(CPUZeusState, bcf);
    ZEUS_PTR_R(64, dst, rd, DST);
    ZEUS_PTR_R(64, src1, rs1, SRC);

    ZeusInsnParams params = {.bcf = a->gbcf_b30};
    TCGv_i32 insn_params = tcg_constant_i32(params.val);

    TCGv_ptr bcf = tcg_temp_ebb_new_ptr();
    TCGv_i32 src2 = tcg_constant_i32(a->imm32);

    tcg_gen_addi_ptr(bcf, cpu_env, bcf_ofs);

    gen_helper_func(cpu_env, bcf, dst, src1, src2, insn_params);

    tcg_temp_free_ptr(bcf);
    tcg_temp_free_ptr(dst);
    tcg_temp_free_ptr(src1);

    return true;
}

TRANS_ONE(iseqi2_u64) { return gen_zeus_compare_and_set_imm(ctx, a, gen_helper_mtia_zeus__iseqi2_u64); }
TRANS_ONE(islti2_u64) { return gen_zeus_compare_and_set_imm(ctx, a, gen_helper_mtia_zeus__islti2_u64); }
TRANS_ONE(isltei2_u64){ return gen_zeus_compare_and_set_imm(ctx, a, gen_helper_mtia_zeus__isltei2_u64); }

typedef void (*zeus_compare_and_set_helper_t)(
    TCGv_env env, TCGv_ptr bcf, TCGv_ptr d,
    TCGv_ptr s1, TCGv_ptr s2, TCGv_i32 params);

static bool gen_zeus_compare_and_set(
    DisasContext *ctx,
    arg_rdrs1rs2_gbcf *a,
    zeus_compare_and_set_helper_t gen_helper_func
)
{
    uint32_t bcf_ofs = offsetof(CPUZeusState, bcf);
    ZEUS_PTR_R(64, dst, rd, DST);
    ZEUS_PTR_R(64, src1, rs1, SRC);
    ZEUS_PTR_R(64, src2, rs2, SRC);

    ZeusInsnParams params = {.bcf = a->gbcf};
    TCGv_i32 insn_params = tcg_constant_i32(params.val);

    TCGv_ptr bcf = tcg_temp_ebb_new_ptr();
    tcg_gen_addi_ptr(bcf, cpu_env, bcf_ofs);

    gen_helper_func(cpu_env, bcf, dst, src1, src2, insn_params);

    tcg_temp_free_ptr(bcf);
    tcg_temp_free_ptr(dst);
    tcg_temp_free_ptr(src1);
    tcg_temp_free_ptr(src2);

    return true;
}

TRANS_ONE(iseq2_s64) { return gen_zeus_compare_and_set(ctx, a, gen_helper_mtia_zeus__iseq2_s64); }
TRANS_ONE(islt2_s64) { return gen_zeus_compare_and_set(ctx, a, gen_helper_mtia_zeus__islt2_s64); }
TRANS_ONE(islte2_s64){ return gen_zeus_compare_and_set(ctx, a, gen_helper_mtia_zeus__islte2_s64); }
