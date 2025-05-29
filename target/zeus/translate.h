/* MTIA SIMT CPU TCG translation functions.
 *
 * Copyright (c) 2024 Meta Platforms, Inc.
 *
 *
 */
#pragma once

#include "exec/translator.h"

#define TRANS(NAME_, FUNC_, ...) \
static bool trans_##NAME_(DisasContext *ctx, arg_##NAME_ *a) \
{ return FUNC_(ctx, a, ##__VA_ARGS__); }

#define TRANS_ONE(NAME_) \
static bool trans_##NAME_(DisasContext *ctx, arg_##NAME_ *a)

typedef uint64_t zeus_insn_t;

/*
 * If an operation is being performed on less than TARGET_LONG_BITS,
 * it may require the inputs to be sign- or zero-extended; which will
 * depend on the exact operation being performed.
 */
/*typedef enum {
    EXT_NONE,
    EXT_SIGN,
    EXT_ZERO,
} DisasExtend;*/

typedef enum ZeusTransVergeType {
    ZEUS_TRANS_VERGE_RECONVERGE = 0,
    ZEUS_TRANS_VERGE_2ND_ARM = 1,
} ZeusTransVergeType;

typedef struct ZeusTransVerge {
    uint64_t src_pc;
    uint64_t rpc;
    uint8_t type;
} ZeusTransVerge;

typedef struct DisasContext {
    DisasContextBase base;

    zeus_insn_t opcode;

    GHashTable* verge_map;

} DisasContext;

void zeus_cpu_translate_init(void);


static inline __attribute__ ((__warn_unused_result__))
GHashTable* zeus_trans_verge_map_new(void) {
    return g_hash_table_new_full(
        g_direct_hash, g_direct_equal, // uint64 direct key
        NULL, // no need to free keys
        NULL // do NOT free value which is GList*
    );
}

static inline
gpointer zeus_trans_verge_map_lookup(GHashTable *table, uint64_t pc) {
    return g_hash_table_lookup(table, GINT_TO_POINTER(pc));
}

void zeus_trans_verge_map_insert(GHashTable *table,
    uint64_t src_pc, uint64_t pc, uint64_t rpc, uint8_t type);

#define zeus_trans_verge_for_each(list_, iterator_) \
for (GSList* iterator_ = list_; iterator_; iterator_ = iterator_->next)