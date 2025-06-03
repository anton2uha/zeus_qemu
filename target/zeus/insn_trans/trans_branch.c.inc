
static bool trans_zeus_beqz_brtype_loop(DisasContext *ctx, arg_b_clr *a, bool negate)
{
    uint64_t rpc = ctx->base.pc_next + sign_extend24(a->rpc);
    uint64_t bpc = ctx->base.pc_next + sign_extend24(a->bpc);

    zeus_trans_verge_map_insert(
        ctx->verge_map,
        ctx->base.pc_next,
        rpc,
        rpc,
        ZEUS_TRANS_VERGE_RECONVERGE
    );

    gen_set_pc_imm(ctx, ctx->base.pc_next);
    TCGv_i32 brtype = tcg_constant_i32(2 | (negate << 16));
    TCGv_i64 runtime_rpc = tcg_constant_i64(rpc);
    TCGv_i64 runtime_bpc = tcg_constant_i64(bpc);
    gen_helper_zeus__simt_loop(cpu_env, cpu_bcf, brtype, runtime_rpc, runtime_bpc);

    TCGLabel *l = gen_new_label();

    TCGv_i32 zero = tcg_constant_i32(0);
    tcg_gen_brcond_i32(negate ? TCG_COND_NE : TCG_COND_EQ, zero, cpu_bcf, l);

    gen_goto_tb(ctx, 1/*jump slot*/, ctx->base.pc_next + sizeof(uint64_t));

    gen_set_label(l); /* branch taken */

    gen_goto_tb(ctx, 0/*jump slot*/, ctx->base.pc_next + sign_extend24(a->bpc));

    ctx->base.is_jmp = DISAS_NORETURN;

    return true;
}

static bool trans_zeus_beqz_brtype_if(DisasContext *ctx, arg_b_clr *a, bool negate)
{
    uint64_t bpc = ctx->base.pc_next + sign_extend24(a->bpc);
    uint64_t rpc = ctx->base.pc_next + sign_extend24(a->rpc);

    zeus_trans_verge_map_insert(
        ctx->verge_map,
        ctx->base.pc_next,
        bpc,
        rpc,
        ZEUS_TRANS_VERGE_RECONVERGE
    );

    gen_set_pc_imm(ctx, ctx->base.pc_next);
    TCGv_i32 brtype = tcg_constant_i32(0 | (negate << 16));
    TCGv_i64 runtime_rpc = tcg_constant_i64(rpc);
    gen_helper_zeus__simt_diverge_if(cpu_env, cpu_bcf, brtype, runtime_rpc);

    return true;
}

static bool trans_zeus_beqz_brtype_if_else(DisasContext *ctx, arg_b_clr *a, bool negate)
{
    g_assert(a->rpcv);

    uint64_t bpc = ctx->base.pc_next + sign_extend24(a->bpc);
    uint64_t rpc = ctx->base.pc_next + sign_extend24(a->rpc);

    zeus_trans_verge_map_insert(
        ctx->verge_map,
        ctx->base.pc_next,
        rpc,
        rpc,
        ZEUS_TRANS_VERGE_RECONVERGE
    );

    /*zeus_trans_verge_map_insert(
        ctx->verge_map,
        ctx->base.pc_next,
        bpc,
        rpc,
        ZEUS_TRANS_VERGE_2ND_ARM
    );*/

    gen_set_pc_imm(ctx, ctx->base.pc_next);
    TCGv_i32 brtype = tcg_constant_i32(1 | (negate << 16));
    TCGv_i64 runtime_bpc = tcg_constant_i64(bpc);
    TCGv_i64 runtime_rpc = tcg_constant_i64(rpc);
    gen_helper_zeus__simt_diverge_if_else(cpu_env, cpu_bcf, brtype,
        runtime_bpc, runtime_rpc);

    return true;
}

TRANS_ONE(b_clr)
{
    //printf("TRANS BEQZ brtype:%u rpcv:%u bpc:%x rpc:%x\n",
    //    a->brtype, a->rpcv, a->bpc, a->rpc);

    switch (a->brtype) {
        case 0: return trans_zeus_beqz_brtype_if(ctx, a, false); break;
        case 1: return trans_zeus_beqz_brtype_if_else(ctx, a, false); break;
        case 2: return trans_zeus_beqz_brtype_loop(ctx, a, false); break;
        default: g_assert_not_reached(); return false;
    }
}

TRANS_ONE(b_set)
{
    //printf("TRANS BNEQZ brtype:%u rpcv:%u bpc:%x rpc:%x\n",
    //    a->brtype, a->rpcv, a->bpc, a->rpc);

    switch (a->brtype) {
        case 0: return trans_zeus_beqz_brtype_if(ctx, a, true); break;
        case 1: return trans_zeus_beqz_brtype_if_else(ctx, a, true); break;
        case 2: return trans_zeus_beqz_brtype_loop(ctx, a, true); break;
        default: g_assert_not_reached(); return false;
    }
}

typedef struct JumpParams {
    uint64_t pc, target, start_of_else_pc;
} JumpParams;

static
gboolean find_jmp_inside_if_else(gpointer key, gpointer verges, gpointer user_data)
{
    JumpParams* jump = user_data;
    uint64_t pc = GPOINTER_TO_INT(key);

    if (pc < jump->pc) { return false; } // jump must be before ELSE-arm

    zeus_trans_verge_for_each(verges, iterator) {
        ZeusTransVerge* verge = iterator->data;
        if (
            verge->rpc == jump->target &&
            jump->pc > verge->src_pc
        ) {
            //printf("JAL %lx->%lx inside IF-ELSE %lx\n", jump->pc, jump->target, pc);
            jump->start_of_else_pc = pc;
            return true;
        }
    }

    return false;
}

TRANS_ONE(jal)
{
    gen_set_pc_imm(ctx, ctx->base.pc_next);

    uint64_t jump_target_addr = ctx->base.pc_next + sign_extend32(a->imm32);

    JumpParams jump_params = {
        .pc = ctx->base.pc_next,
        .target = jump_target_addr,
        .start_of_else_pc = 0
    };

    gpointer if_else_ptr = g_hash_table_find(ctx->verge_map,
        (GHRFunc)find_jmp_inside_if_else,
        &jump_params
    );

    if (if_else_ptr) {
        gen_helper_zeus__simt_if_arm_end(cpu_env);
        //gen_goto_tb(ctx, 0/*jump slot*/, jump_params.start_of_else_pc); // goto else-arm
        gen_goto_tb(ctx, 0/*jump slot*/, jump_target_addr);
        ctx->base.is_jmp = DISAS_SIMT;
    }
    else {
        gen_goto_tb(ctx, 0/*jump slot*/, jump_target_addr);
        ctx->base.is_jmp = DISAS_NORETURN;
    }

    return true;
}
