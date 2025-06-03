#define ZEUS_OFFSETOF_R64(reg_name_) offsetof(CPUZeusState, gpr.r64xThr[a->reg_name_]);
#define ZEUS_OFFSETOF_R32(reg_name_) offsetof(CPUZeusState, gpr.r32xThr[a->reg_name_]);

#define ZEUS_PTR_R(n_, var_name_, reg_name_, SRC_DST_) \
    uint32_t var_name_##_ofs = (a->reg_name_ == 0)? \
    offsetof(CPUZeusState, zero[SRC_DST_]): \
    offsetof(CPUZeusState, gpr.r##n_##xThr[a->reg_name_]); \
    TCGv_ptr var_name_ = tcg_temp_ebb_new_ptr(); \
    tcg_gen_addi_ptr(var_name_, cpu_env, var_name_##_ofs);

enum {SRC = 0, DST = 1};

static bool gen_assert_not_reached(DisasContext *ctx, void *a)
{
    printf("ERROR: no translation for opcode %lx\n", ctx->opcode);
    g_assert_not_reached();
    TCGv message  = tcg_constant_tl(0/*=SIM_EXIT*/);
    gen_helper_zeus__simrsvd(cpu_env, message);
    return true;
}

TRANS_ONE(lui)
{
    ZEUS_PTR_R(64, dst, rd, DST);

    TCGv_i32 imm = tcg_constant_i32(a->imm32);
    TCGv_i32 hints = tcg_constant_i32(0);

    gen_helper_zeus__lui(cpu_env, dst, imm, hints);

    tcg_temp_free_ptr(dst);

    return true;
}

TRANS_ONE(iadd2)
{
    ZEUS_PTR_R(64, dst, rd, DST);
    ZEUS_PTR_R(64, src1, rs1, SRC);
    ZEUS_PTR_R(64, src2, rs2, SRC);

    TCGv_i32 hints = tcg_constant_i32(0);

    gen_helper_zeus__iadd2(cpu_env, dst, src1, src2, hints);

    tcg_temp_free_ptr(dst);
    tcg_temp_free_ptr(src1);
    tcg_temp_free_ptr(src2);

    return true;
}

TRANS_ONE(imadd2)
{
    ZEUS_PTR_R(64, dst, rd, DST);
    ZEUS_PTR_R(64, src1, rs1, SRC);
    ZEUS_PTR_R(64, src2, rs2, SRC);
    ZEUS_PTR_R(64, src3, rs3, SRC);

    TCGv_i32 hints = tcg_constant_i32(0);

    gen_helper_zeus__imadd2(cpu_env, dst, src1, src2, src3, hints);

    tcg_temp_free_ptr(dst);
    tcg_temp_free_ptr(src1);
    tcg_temp_free_ptr(src2);
    tcg_temp_free_ptr(src3);

    return true;
}

TRANS_ONE(iaddi2)
{
    ZEUS_PTR_R(64, dst, rd, DST);
    ZEUS_PTR_R(64, src1, rs1, SRC);

    TCGv src2 = tcg_constant_i64(sign_extend32(a->imm32));

    ZeusInsnParams params = {.n1 = a->n1_imm};
    TCGv_i32 insn_params = tcg_constant_i32(params.val);

    gen_helper_zeus__iaddi2(cpu_env, dst, src1, src2, insn_params);

    tcg_temp_free_ptr(dst);
    tcg_temp_free_ptr(src1);

    return true;
}

typedef void (*zeus_rdrs1imm_helper_t)(
    TCGv_env env, TCGv_ptr d,
    TCGv_ptr s1, TCGv_i32 s2, TCGv_i32 params);

static bool gen_zeus_insn_format_rdrs1imm(
    DisasContext *ctx,
    arg_rdrs1imm_gbcf *a,
    zeus_rdrs1imm_helper_t gen_helper_func
)
{
    ZEUS_PTR_R(64, dst, rd, DST);
    ZEUS_PTR_R(64, src1, rs1, SRC);
    TCGv_i32 src2 = tcg_constant_i32(a->imm32);

    ZeusInsnParams params;// = {.i1 = a->i1};
    TCGv_i32 insn_params = tcg_constant_i32(params.val);

    gen_helper_func(cpu_env, dst, src1, src2, insn_params);

    tcg_temp_free_ptr(dst);
    tcg_temp_free_ptr(src1);

    return true;
}

//TRANS_ONE(muli)  { return gen_zeus_insn_format_rdrs1imm(ctx, a, gen_helper_zeus__muli); }
//TRANS_ONE(muliu) { return gen_zeus_insn_format_rdrs1imm(ctx, a, gen_helper_zeus__muliu); }
TRANS_ONE(xori2)  { return gen_zeus_insn_format_rdrs1imm(ctx, a, gen_helper_zeus__xori2); }
TRANS_ONE(ori2)   { return gen_zeus_insn_format_rdrs1imm(ctx, a, gen_helper_zeus__ori2); }
TRANS_ONE(andi2)  { return gen_zeus_insn_format_rdrs1imm(ctx, a, gen_helper_zeus__andi2); }
//TRANS_ONE(ishli2)  { return gen_zeus_insn_format_rdrs1imm(ctx, a, gen_helper_zeus__ishli2); }
//TRANS_ONE(srli)  { return gen_zeus_insn_format_rdrs1imm(ctx, a, gen_helper_zeus__srli); }
//TRANS_ONE(srai)  { return gen_zeus_insn_format_rdrs1imm(ctx, a, gen_helper_zeus__srai); }
//TRANS_ONE(rori2)  { return gen_zeus_insn_format_rdrs1imm(ctx, a, gen_helper_zeus__rori2); }


TRANS(iadd         , gen_assert_not_reached)
TRANS(iaddi      , gen_assert_not_reached)
TRANS(imadd      , gen_assert_not_reached)
TRANS(imaddw_s32 , gen_assert_not_reached)
TRANS(imaddw_u32   , gen_assert_not_reached)
TRANS(xor2       , gen_assert_not_reached)
TRANS(xor         , gen_assert_not_reached)
TRANS(xori        , gen_assert_not_reached)
TRANS(or2         , gen_assert_not_reached)
TRANS(or          , gen_assert_not_reached)
TRANS(ori          , gen_assert_not_reached)
TRANS(and2         , gen_assert_not_reached)
TRANS(and          , gen_assert_not_reached)
TRANS(andi            , gen_assert_not_reached)
TRANS(islt_s32     , gen_assert_not_reached)
TRANS(islti2_s64   , gen_assert_not_reached)
TRANS(islti_s32    , gen_assert_not_reached)
TRANS(islt2_u64    , gen_assert_not_reached)
TRANS(islt_u32     , gen_assert_not_reached)
TRANS(islti_u32    , gen_assert_not_reached)
TRANS(islte_s32    , gen_assert_not_reached)
TRANS(isltei_s32   , gen_assert_not_reached)
TRANS(islte2_u64   , gen_assert_not_reached)
TRANS(isltei2_s64   , gen_assert_not_reached)
TRANS(islte_u32    , gen_assert_not_reached)
TRANS(isltei_u32   , gen_assert_not_reached)
TRANS(iseq_s32     , gen_assert_not_reached)
TRANS(iseqi2_s64   , gen_assert_not_reached)
TRANS(iseqi_s32    , gen_assert_not_reached)
TRANS(iseq2_u64    , gen_assert_not_reached)
TRANS(iseq_u32     , gen_assert_not_reached)
TRANS(iseqi_u32    , gen_assert_not_reached)
TRANS(ishl2        , gen_assert_not_reached)
TRANS(ishl         , gen_assert_not_reached)
TRANS(ishli2       , gen_assert_not_reached)
TRANS(ishli        , gen_assert_not_reached)
TRANS(ishr2_u64    , gen_assert_not_reached)
TRANS(ishr_u32     , gen_assert_not_reached)
TRANS(ishr2_s64    , gen_assert_not_reached)
TRANS(ishr_s32     , gen_assert_not_reached)
TRANS(ishri2_u64   , gen_assert_not_reached)
TRANS(ishri_u32    , gen_assert_not_reached)
TRANS(ishri2_s64   , gen_assert_not_reached)
TRANS(ishri_s32    , gen_assert_not_reached)
TRANS(fmadd        , gen_assert_not_reached)
TRANS(fadd         , gen_assert_not_reached)
TRANS(fmadd2       , gen_assert_not_reached)
TRANS(fadd2        , gen_assert_not_reached)
TRANS(fsgnj        , gen_assert_not_reached)
TRANS(fsgnjx       , gen_assert_not_reached)
TRANS(fsgnj2       , gen_assert_not_reached)
TRANS(fsgnjx2      , gen_assert_not_reached)
TRANS(fclass       , gen_assert_not_reached)
TRANS(fclass2      , gen_assert_not_reached)
TRANS(fmin         , gen_assert_not_reached)
TRANS(fmax         , gen_assert_not_reached)
TRANS(fmin2        , gen_assert_not_reached)
TRANS(fmax2        , gen_assert_not_reached)
TRANS(feq          , gen_assert_not_reached)
TRANS(flt          , gen_assert_not_reached)
TRANS(fle          , gen_assert_not_reached)
TRANS(feq2         , gen_assert_not_reached)
TRANS(flt2         , gen_assert_not_reached)
TRANS(fle2         , gen_assert_not_reached)
TRANS(fcvt_type1   , gen_assert_not_reached)
TRANS(fcvt_type2   , gen_assert_not_reached)
TRANS(fcvt_type3   , gen_assert_not_reached)
TRANS(fcvt_type4   , gen_assert_not_reached)
TRANS(cpop         , gen_assert_not_reached)
TRANS(cpop_w       , gen_assert_not_reached)
TRANS(min          , gen_assert_not_reached)
TRANS(min_w        , gen_assert_not_reached)
TRANS(minu         , gen_assert_not_reached)
TRANS(minu_w       , gen_assert_not_reached)
TRANS(max          , gen_assert_not_reached)
TRANS(max_w        , gen_assert_not_reached)
TRANS(maxu         , gen_assert_not_reached)
TRANS(maxu_w       , gen_assert_not_reached)
TRANS(rol          , gen_assert_not_reached)
TRANS(rol_w        , gen_assert_not_reached)
TRANS(ror          , gen_assert_not_reached)
TRANS(ror_w        , gen_assert_not_reached)
TRANS(rori         , gen_assert_not_reached)
TRANS(rori_w       , gen_assert_not_reached)
TRANS(shiadd       , gen_assert_not_reached)
TRANS(shiadd_wu    , gen_assert_not_reached)
TRANS(zext_d_h     , gen_assert_not_reached)
TRANS(zext_w_h     , gen_assert_not_reached)
TRANS(sext_d_h     , gen_assert_not_reached)
TRANS(sext_w_h     , gen_assert_not_reached)
TRANS(sext_d_w     , gen_assert_not_reached)
TRANS(csel2        , gen_assert_not_reached)
TRANS(csel         , gen_assert_not_reached)
TRANS(cseli2       , gen_assert_not_reached)
TRANS(cseli        , gen_assert_not_reached)
TRANS(exit         , gen_assert_not_reached)
TRANS(break        , gen_assert_not_reached)
TRANS(sleep        , gen_assert_not_reached)
TRANS(sleepi       , gen_assert_not_reached)
TRANS(csrr2        , gen_assert_not_reached)
TRANS(lds          , gen_assert_not_reached)
TRANS(lds2         , gen_assert_not_reached)
TRANS(sts          , gen_assert_not_reached)
TRANS(sts2         , gen_assert_not_reached)
TRANS(checkdc      , gen_assert_not_reached)
TRANS(ldg_s8       , gen_assert_not_reached)
TRANS(ldg_u8       , gen_assert_not_reached)
TRANS(ldg_s16      , gen_assert_not_reached)
TRANS(ldg_u16      , gen_assert_not_reached)
TRANS(ldg_32       , gen_assert_not_reached)
TRANS(ldg2         , gen_assert_not_reached)
TRANS(stg_s8       , gen_assert_not_reached)
TRANS(stg_16       , gen_assert_not_reached)
TRANS(stg_32       , gen_assert_not_reached)
TRANS(stg2         , gen_assert_not_reached)
TRANS(jalr          , gen_assert_not_reached)
TRANS(auipc        , gen_assert_not_reached)
TRANS(syncweft     , gen_assert_not_reached)
TRANS(syncwefti    , gen_assert_not_reached)
TRANS(syncweft_reduce_add   , gen_assert_not_reached)
TRANS(syncwefti_reduce_add  , gen_assert_not_reached)
TRANS(syncweft_reduce_xor   , gen_assert_not_reached)
TRANS(syncwefti_reduce_xor  , gen_assert_not_reached)
TRANS(syncweft_reduce_and   , gen_assert_not_reached)
TRANS(syncwefti_reduce_and  , gen_assert_not_reached)
TRANS(syncweft_reduce_or    , gen_assert_not_reached)
TRANS(syncwefti_reduce_or   , gen_assert_not_reached)
TRANS(syncweft_reduce_min   , gen_assert_not_reached)
TRANS(syncwefti_reduce_min  , gen_assert_not_reached)
TRANS(syncweft_reduce_max   , gen_assert_not_reached)
TRANS(syncwefti_reduce_max  , gen_assert_not_reached)
TRANS(syncweft_reduce_minu  , gen_assert_not_reached)
TRANS(syncwefti_reduce_minu , gen_assert_not_reached)
TRANS(syncweft_reduce_maxu  , gen_assert_not_reached)
TRANS(syncwefti_reduce_maxu , gen_assert_not_reached)
TRANS(syncweft_shuffle      , gen_assert_not_reached)
TRANS(syncweft_shuffle_up   , gen_assert_not_reached)
TRANS(syncweft_shuffle_down , gen_assert_not_reached)
TRANS(syncweft_shuffle_xor  , gen_assert_not_reached)
TRANS(sadd2          , gen_assert_not_reached)
TRANS(sadd           , gen_assert_not_reached)
TRANS(add2_sop       , gen_assert_not_reached)
TRANS(add_sop        , gen_assert_not_reached)
TRANS(addw_sop       , gen_assert_not_reached)
TRANS(sadd2_vop      , gen_assert_not_reached)
TRANS(sadd_vop       , gen_assert_not_reached)
TRANS(lds_sop        , gen_assert_not_reached)
TRANS(lds2_sop       , gen_assert_not_reached)
TRANS(sts_sop        , gen_assert_not_reached)
TRANS(sts2_sop       , gen_assert_not_reached)
TRANS(ldg_sop_s8     , gen_assert_not_reached)
TRANS(ldg_sop_u8     , gen_assert_not_reached)
TRANS(ldg_sop_s16    , gen_assert_not_reached)
TRANS(ldg_sop_u16    , gen_assert_not_reached)
TRANS(ldg_sop_32     , gen_assert_not_reached)
TRANS(ldg2_sop       , gen_assert_not_reached)
TRANS(stg_sop_s8     , gen_assert_not_reached)
TRANS(stg_sop_16     , gen_assert_not_reached)
TRANS(stg_sop_32     , gen_assert_not_reached)
TRANS(stg2_sop       , gen_assert_not_reached)
