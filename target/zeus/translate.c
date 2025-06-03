#include "qemu/osdep.h"
#include "cpu.h"
#include "tcg/tcg-op.h"
#include "exec/translator.h"
#include "exec/helper-proto.h"
#include "exec/helper-gen.h"

#include "exec/log.h"
#include "qemu/qemu-print.h"
#include "fpu/softfloat.h"
#include "translate.h"
#include "exec/translation-block.h"
//#include "internals.h"
#include "tcg/tcg-temp-internal.h"

#define DISAS_STOP        DISAS_TARGET_0
#define DISAS_EXIT        DISAS_TARGET_1
#define DISAS_EXIT_UPDATE DISAS_TARGET_2
#define DISAS_SIMT        DISAS_TARGET_3

// Global register indices.
static TCGv cpu_pc;
static TCGv_i32 cpu_bcf;

static void generate_exception(DisasContext *ctx, int excp)
{
    g_assert_not_reached();
    //tcg_gen_movi_tl(cpu_pc, ctx->base.pc_next);
    //gen_helper_raise_exception(cpu_env, tcg_constant_i32(excp));
    ctx->base.is_jmp = DISAS_NORETURN;
}

static void zeus_tr_init_disas_context(
    DisasContextBase *dcbase,
    CPUState *cs
)
{
    //DisasContext *ctx = container_of(dcbase, DisasContext, base);

    //for (uint8_t thread_id = 0; thread_id < ZEUS_CPU_NR_THREADS; thread_id++) {
    //    ctx->zero[thread_id] = tcg_constant_tl(0);
    //}
}

static void zeus_tr_disas_log(
    const DisasContextBase *dcbase,
    CPUState *cpu,
    FILE *logfile
)
{
    qemu_log("IN: %s\n", lookup_symbol(dcbase->pc_first));
    target_disas(logfile, cpu, dcbase);
}

static void zeus_tr_tb_start(DisasContextBase *dcbase, CPUState *cs)
{
}

static void gen_set_pc_imm(DisasContext *ctx, target_ulong dest)
{
    tcg_gen_movi_tl(cpu_pc, dest);
}

static void lookup_and_goto_ptr(DisasContext *ctx)
{
    tcg_gen_lookup_and_goto_ptr();
}

static void gen_goto_tb(DisasContext *ctx, int n, target_ulong dest)
{
    TCGv_i64 runtime_dest = tcg_temp_new_i64();
    tcg_gen_movi_tl(runtime_dest, dest);
    gen_helper_mtia_zeus__simt_current_split_set_pc(cpu_env, runtime_dest);
    tcg_temp_free_i64(runtime_dest);

    if (translator_use_goto_tb(&ctx->base, dest)) {
        tcg_gen_goto_tb(n);
        gen_set_pc_imm(ctx, dest);
        tcg_gen_exit_tb(ctx->base.tb, n);
    } else {
        gen_set_pc_imm(ctx, dest);
        lookup_and_goto_ptr(ctx);
    }
}

static uint64_t sign_extend24(uint32_t v) {
    return ((v & (1 << 23))? (~0ul << 24):0ul) | v;
}

static uint64_t sign_extend32(uint32_t v) {
    return ((v & (1 << 31))? (~0ul << 32):0ul) | v;
}

#include "decode-insns.c.inc" // decode()
#include "insn_trans/trans_arithmetic.c.inc"
#include "insn_trans/trans_compare.c.inc"
#include "insn_trans/trans_branch.c.inc"
#include "insn_trans/trans_special.c.inc"

static void zeus_tr_translate_insn(DisasContextBase *dcbase, CPUState *cs)
{
    CPUZeusState *env = cs->env_ptr;
    DisasContext *ctx = container_of(dcbase, DisasContext, base);

    ctx->opcode = translator_ldq(env, &ctx->base, ctx->base.pc_next);
    //printf("TRANS PC:%lx\n", ctx->base.pc_next);

    gpointer verges = zeus_trans_verge_map_lookup(ctx->verge_map, ctx->base.pc_next);
    if (verges) {
        gen_set_pc_imm(ctx, ctx->base.pc_next);
        zeus_trans_verge_for_each(verges, iterator) {
            ZeusTransVerge* verge = iterator->data;
            //printf("TRANS VERGE PC:%lx type:%u\n", ctx->base.pc_next, verge->type);
            switch (verge->type) {
                case ZEUS_TRANS_VERGE_RECONVERGE: {
                    TCGv_i64 else_arm_end = tcg_temp_new_i64();
                    gen_helper_mtia_zeus__simt_reconverge(else_arm_end, cpu_env);
                    TCGLabel *lbl = gen_new_label();
                    TCGv_i64 zero = tcg_constant_i64(0);
                    tcg_gen_brcond_i64(TCG_COND_EQ, zero, else_arm_end, lbl);
                    gen_goto_tb(ctx, 1/*jump slot*/, ctx->base.pc_next);
                    gen_set_label(lbl); /* branch taken */
                    tcg_temp_free_i64(else_arm_end);
                } break;
                case ZEUS_TRANS_VERGE_2ND_ARM:
                    gen_helper_mtia_zeus__simt_2nd_arm(cpu_env); break;
                default: g_assert_not_reached(); break;
            }
        }
    }

    // decode() is defined in decode-insns.c.inc
    if (!decode(ctx, ctx->opcode)) {
        qemu_log_mask(
            LOG_UNIMP, "Error: unknown opcode. "
            TARGET_FMT_lx ": 0x%lx\n",
            ctx->base.pc_next, ctx->opcode);
        g_assert_not_reached();
        //generate_exception(ctx, EXCCODE_INE);
    }

    ctx->base.pc_next += sizeof(zeus_insn_t);
}

static void zeus_tr_tb_stop(DisasContextBase *dcbase, CPUState *cs)
{
    DisasContext *ctx = container_of(dcbase, DisasContext, base);

    switch (ctx->base.is_jmp) {
    case DISAS_STOP:
        g_assert_not_reached();
        //tcg_gen_movi_tl(cpu_pc, ctx->base.pc_next);
        tcg_gen_lookup_and_goto_ptr();
        break;
    case DISAS_TOO_MANY:
        //g_assert_not_reached();
        gen_goto_tb(ctx, 0, ctx->base.pc_next);
        break;
    case DISAS_NORETURN:
        break;
    case DISAS_SIMT:
        //tcg_gen_exit_tb(ctx->base.tb, 0);
        //tcg_gen_exit_tb(NULL, 0);
        break;
    case DISAS_EXIT_UPDATE:
        g_assert_not_reached();
        //tcg_gen_movi_tl(cpu_pc, ctx->base.pc_next);
        QEMU_FALLTHROUGH;
    case DISAS_EXIT:
        tcg_gen_exit_tb(NULL, 0);
        break;
    default:
        g_assert_not_reached();
    }
}

static void zeus_tr_insn_start(DisasContextBase *dcbase, CPUState *cs)
{
    DisasContext *ctx = container_of(dcbase, DisasContext, base);

    tcg_gen_insn_start(ctx->base.pc_next);
}


static const TranslatorOps zeus_translation_ops_instance = {
    .init_disas_context = zeus_tr_init_disas_context,
    .tb_start           = zeus_tr_tb_start,
    .insn_start         = zeus_tr_insn_start,
    .translate_insn     = zeus_tr_translate_insn,
    .tb_stop            = zeus_tr_tb_stop,
    .disas_log          = zeus_tr_disas_log,
};

static
GHashTable* trans_verge_map_instance;

// declared in exec/translator.h
void gen_intermediate_code(
    CPUState *cs,
    TranslationBlock *tb,
    int *max_insns,
    target_ulong pc,
    void *host_pc
)
{
    printf("gen_intermediate_code cs:%p tb:%p pc:%lx host_pc:%p\n", cs, tb, pc, host_pc);

    DisasContext ctx;
    ctx.verge_map = trans_verge_map_instance;

    translator_loop(cs, tb, max_insns, pc, host_pc,
        &zeus_translation_ops_instance, &ctx.base);
}

void zeus_cpu_translate_init(void)
{
    trans_verge_map_instance = zeus_trans_verge_map_new();

    cpu_pc = tcg_global_mem_new(
        cpu_env,
        offsetof(CPUZeusState, pc),
        "pc"
    );
    cpu_bcf = tcg_global_mem_new_i32(
        cpu_env,
        offsetof(CPUZeusState, bcf),
        "bcf"
    );
}


void zeus_trans_verge_map_insert(GHashTable *table,
    uint64_t src_pc, uint64_t pc, uint64_t rpc, uint8_t type)
{
    ZeusTransVerge* new_record = g_new(ZeusTransVerge, 1);
    new_record->src_pc = src_pc;
    new_record->rpc = rpc;
    new_record->type = type;

    gpointer verges = g_hash_table_lookup(table, GINT_TO_POINTER(pc));

    if (verges == NULL) {
        verges = g_slist_append(verges, new_record);
        g_hash_table_insert(table, GINT_TO_POINTER(pc), verges);
    }
    else {
        bool already_translated = false;
        zeus_trans_verge_for_each(verges, iterator) {
            ZeusTransVerge* verge = iterator->data;
            if (verge->src_pc == new_record->src_pc) {
                already_translated = true;
                break;
            }
        }

        if (!already_translated) {
            verges = g_slist_append(verges, new_record);
            g_hash_table_insert(table, GINT_TO_POINTER(pc), verges);
        }
    }
}
