#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "nbt.h"
#include "logger.h"

static void write_be32(netbuf *buf, uint32_t value) {
    write_byte(buf, (value >> 24) & 0xFF);
    write_byte(buf, (value >> 16) & 0xFF);
    write_byte(buf, (value >> 8) & 0xFF);
    write_byte(buf, value & 0xFF);
}

void nbt_write_header(netbuf *buf, nbt_tag_type type, const char *name) {
    // 列表项（name 为 NULL）不写 type byte
    if (name != NULL) {
        write_byte(buf, (uint8_t)type);
    }
    
    if (type == TAG_End) return;

    if (name != NULL) {
        // 有名字的情况：写 2 字节长度 + 名字
        uint16_t len = (uint16_t)strlen(name);
        write_byte(buf, (len >> 8) & 0xFF);
        write_byte(buf, len & 0xFF);
        for (int i = 0; i < len; i++) write_byte(buf, name[i]);
    } else {

    }
}

void nbt_write_byte(netbuf *buf, const char *name, int8_t value) {
    nbt_write_header(buf, TAG_Byte, name);
    write_byte(buf, (uint8_t)value);
}

void nbt_write_int(netbuf *buf, const char *name, int32_t value) {
    nbt_write_header(buf, TAG_Int, name);
    write_be32(buf, (uint32_t)value);
}

void nbt_write_double(netbuf *buf, const char *name, double value) {
    nbt_write_header(buf, TAG_Double, name);
    write_double_to_buf(buf, value);
}

void nbt_write_float(netbuf *buf, const char *name, float value) {
    nbt_write_header(buf, TAG_Float, name);
    write_float_to_buf(buf, value);
}

void nbt_write_string(netbuf *buf, const char *name, const char *value) {
    nbt_write_header(buf, TAG_String, name);
    uint16_t len = (uint16_t)strlen(value);
    write_byte(buf, (len >> 8) & 0xFF);
    write_byte(buf, len & 0xFF);
    for (int i = 0; i < len; i++) write_byte(buf, value[i]);
}

void nbt_begin_compound(netbuf *buf, const char *name) {
    nbt_write_header(buf, TAG_Compound, name);
}

void nbt_begin_root_compound(netbuf *buf) {
    write_byte(buf, TAG_Compound);
    write_byte(buf, 0x00);
    write_byte(buf, 0x00);
}

void nbt_end_compound(netbuf *buf) {
    write_byte(buf, TAG_End);
}

void nbt_write_string_value(netbuf *buf, const char *value) {
    uint16_t len = (uint16_t)strlen(value);
    write_byte(buf, (len >> 8) & 0xFF);
    write_byte(buf, len & 0xFF);
    for (int i = 0; i < len; i++) write_byte(buf, value[i]);
}

void nbt_begin_list(netbuf *buf, const char *name, nbt_tag_type element_type, int32_t count) {
    nbt_write_header(buf, TAG_List, name);
    write_byte(buf, (uint8_t)element_type);
    write_be32(buf, (uint32_t)count);
}

void nbt_end_list(netbuf *buf) {
    // 列表不需要特殊的结束标记，但为了代码对称性保留
    (void)buf;
}

// 写入大端序 32 位整数 (Int)
void write_be32_to_buf(netbuf *buf, uint32_t value) {
    write_byte(buf, (value >> 24) & 0xFF);
    write_byte(buf, (value >> 16) & 0xFF);
    write_byte(buf, (value >> 8) & 0xFF);
    write_byte(buf, value & 0xFF);
}

// 写入大端序 64 位整数 (用于 Hashed Seed)
void write_long_to_buf(netbuf *buf, int64_t value) {
    uint64_t u = (uint64_t)value;
    write_byte(buf, (u >> 56) & 0xFF);
    write_byte(buf, (u >> 48) & 0xFF);
    write_byte(buf, (u >> 40) & 0xFF);
    write_byte(buf, (u >> 32) & 0xFF);
    write_byte(buf, (u >> 24) & 0xFF);
    write_byte(buf, (u >> 16) & 0xFF);
    write_byte(buf, (u >> 8) & 0xFF);
    write_byte(buf, u & 0xFF);
}

// 写入大端序 64 位浮点数 (Double)
void write_double_to_buf(netbuf *buf, double value) {
    union { double d; uint64_t i; } u;
    u.d = value;
    write_long_to_buf(buf, u.i);
}

// 写入大端序 32 位浮点数 (Float)
void write_float_to_buf(netbuf *buf, float value) {
    union { float f; uint32_t i; } u;
    u.f = value;
    write_be32_to_buf(buf, u.i);
}

// ========== NBT 调试工具 ==========

static const char* nbt_tag_type_name(nbt_tag_type type) {
    switch (type) {
        case TAG_End: return "TAG_End";
        case TAG_Byte: return "TAG_Byte";
        case TAG_Short: return "TAG_Short";
        case TAG_Int: return "TAG_Int";
        case TAG_Long: return "TAG_Long";
        case TAG_Float: return "TAG_Float";
        case TAG_Double: return "TAG_Double";
        case TAG_Byte_Array: return "TAG_Byte_Array";
        case TAG_String: return "TAG_String";
        case TAG_List: return "TAG_List";
        case TAG_Compound: return "TAG_Compound";
        case TAG_Int_Array: return "TAG_Int_Array";
        case TAG_Long_Array: return "TAG_Long_Array";
        default: return "UNKNOWN";
    }
}

typedef struct {
    const uint8_t *data;
    size_t size;
    size_t pos;
    int indent;
} nbt_debug_ctx;

static uint8_t dbg_read_byte(nbt_debug_ctx *ctx) {
    if (ctx->pos >= ctx->size) return 0;
    return ctx->data[ctx->pos++];
}

static int16_t dbg_read_short(nbt_debug_ctx *ctx) {
    uint8_t hi = dbg_read_byte(ctx);
    uint8_t lo = dbg_read_byte(ctx);
    return (int16_t)((hi << 8) | lo);
}

static int32_t dbg_read_int(nbt_debug_ctx *ctx) {
    uint8_t b0 = dbg_read_byte(ctx);
    uint8_t b1 = dbg_read_byte(ctx);
    uint8_t b2 = dbg_read_byte(ctx);
    uint8_t b3 = dbg_read_byte(ctx);
    return (int32_t)((b0 << 24) | (b1 << 16) | (b2 << 8) | b3);
}

static int64_t dbg_read_long(nbt_debug_ctx *ctx) {
    uint32_t hi = (uint32_t)dbg_read_int(ctx);
    uint32_t lo = (uint32_t)dbg_read_int(ctx);
    return (int64_t)(((uint64_t)hi << 32) | lo);
}

static float dbg_read_float(nbt_debug_ctx *ctx) {
    uint32_t v = (uint32_t)dbg_read_int(ctx);
    float f;
    memcpy(&f, &v, 4);
    return f;
}

static double dbg_read_double(nbt_debug_ctx *ctx) {
    uint64_t v = (uint64_t)dbg_read_long(ctx);
    double d;
    memcpy(&d, &v, 8);
    return d;
}

static void dbg_read_string(nbt_debug_ctx *ctx, char *out, size_t max_len) {
    int16_t len = dbg_read_short(ctx);
    if (len <= 0 || ctx->pos + (size_t)len > ctx->size) {
        out[0] = '\0';
        return;
    }
    size_t copy_len = ((size_t)len < max_len - 1) ? (size_t)len : max_len - 1;
    memcpy(out, &ctx->data[ctx->pos], copy_len);
    out[copy_len] = '\0';
    ctx->pos += len;
}

static void dbg_indent(nbt_debug_ctx *ctx) {
    char buf[128] = {0};
    int pos = 0;
    for (int i = 0; i < ctx->indent && pos < 126; i++) {
        buf[pos++] = ' ';
        buf[pos++] = ' ';
    }
    plog_noprefix("%s", buf);
}

static void dbg_parse_tag(nbt_debug_ctx *ctx, int is_list_elem, nbt_tag_type list_elem_type);

static void dbg_parse_tag(nbt_debug_ctx *ctx, int is_list_elem, nbt_tag_type list_elem_type) {
    if (ctx->pos >= ctx->size) return;
    
    nbt_tag_type type;
    char name[256] = {0};
    
    if (is_list_elem) {
        type = list_elem_type;
    } else {
        type = (nbt_tag_type)dbg_read_byte(ctx);
        if (type == TAG_End) {
            dbg_indent(ctx);
            plog_noprefix("TAG_End");
            ctx->indent--;
            return;
        }
        dbg_read_string(ctx, name, sizeof(name));
    }
    
    dbg_indent(ctx);
    
    switch (type) {
        case TAG_Byte:
            plog_noprefix("%s(\"%s\"): %d", nbt_tag_type_name(type), name, (int8_t)dbg_read_byte(ctx));
            break;
        case TAG_Short:
            plog_noprefix("%s(\"%s\"): %d", nbt_tag_type_name(type), name, dbg_read_short(ctx));
            break;
        case TAG_Int:
            plog_noprefix("%s(\"%s\"): %d", nbt_tag_type_name(type), name, dbg_read_int(ctx));
            break;
        case TAG_Long:
            plog_noprefix("%s(\"%s\"): %ld", nbt_tag_type_name(type), name, (long)dbg_read_long(ctx));
            break;
        case TAG_Float:
            plog_noprefix("%s(\"%s\"): %f", nbt_tag_type_name(type), name, dbg_read_float(ctx));
            break;
        case TAG_Double:
            plog_noprefix("%s(\"%s\"): %f", nbt_tag_type_name(type), name, dbg_read_double(ctx));
            break;
        case TAG_String: {
            char str[1024];
            dbg_read_string(ctx, str, sizeof(str));
            plog_noprefix("%s(\"%s\"): \"%s\"", nbt_tag_type_name(type), name, str);
            break;
        }
        case TAG_Compound:
            plog_noprefix("%s(\"%s\") {", nbt_tag_type_name(type), name);
            ctx->indent++;
            while (ctx->pos < ctx->size) {
                if (ctx->data[ctx->pos] == TAG_End) {
                    ctx->pos++;
                    ctx->indent--;
                    dbg_indent(ctx);
                    plog_noprefix("}");
                    break;
                }
                dbg_parse_tag(ctx, 0, TAG_End);
            }
            break;
        case TAG_List: {
            nbt_tag_type elem_type = (nbt_tag_type)dbg_read_byte(ctx);
            int32_t count = dbg_read_int(ctx);
            plog_noprefix("%s(\"%s\"): %s[%d]", nbt_tag_type_name(type), name, nbt_tag_type_name(elem_type), count);
            ctx->indent++;
            for (int32_t i = 0; i < count && ctx->pos < ctx->size; i++) {
                dbg_parse_tag(ctx, 1, elem_type);
            }
            ctx->indent--;
            break;
        }
        default:
            plog_noprefix("%s(\"%s\"): <unsupported type %d>", nbt_tag_type_name(type), name, type);
            break;
    }
}

void nbt_debug_dump(const uint8_t *data, size_t size) {
    nbt_debug_ctx ctx = {
        .data = data,
        .size = size,
        .pos = 0,
        .indent = 0
    };
    
    plog(LOG_INFO "=== NBT Debug Dump (size=%zu) ===", size);
    while (ctx.pos < ctx.size) {
        dbg_parse_tag(&ctx, 0, TAG_End);
    }
    plog(LOG_INFO "=== End NBT Dump ===");
}
