
TRANS_ONE(simrsvd)
{
    TCGv message = tcg_constant_tl(a->simrsvd);
    gen_helper_zeus__simrsvd(tcg_env, message);
    ctx->base.is_jmp = (a->simrsvd == 0)? DISAS_EXIT : DISAS_NEXT;

    return true;
}

TRANS_ONE(csrr)
{
    ZEUS_PTR_R(32, dst, rd_w, 1);
    TCGv_i32 csr_id = tcg_constant_i32(a->imm32);
    TCGv_i32 hints = tcg_constant_i32(0);

    gen_helper_zeus__csrr(tcg_env, dst, csr_id, hints);

    tcg_temp_free_ptr(dst);

    return true;
}