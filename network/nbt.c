#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include "nbt.h"

static void write_be16(netbuf *buf, uint16_t value) {
    write_byte(buf, (value >> 8) & 0xFF);
    write_byte(buf, value & 0xFF);
}

static void write_be32(netbuf *buf, uint32_t value) {
    write_byte(buf, (value >> 24) & 0xFF);
    write_byte(buf, (value >> 16) & 0xFF);
    write_byte(buf, (value >> 8) & 0xFF);
    write_byte(buf, value & 0xFF);
}

void nbt_write_header(netbuf *buf, nbt_tag_type type, const char *name) {
    write_byte(buf, (uint8_t)type);
    
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
    nbt_write_header(buf, TAG_Double, name); // TAG_Double 是 6
    write_double_to_buf(buf, value); // 调用你写好的那个写 8 字节的函数
}

void nbt_write_float(netbuf *buf, const char *name, float value) {
    nbt_write_header(buf, TAG_Float, name);  // 必须是 5
    write_float_to_buf(buf, value);          // 写 4 字节
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
    write_byte(buf, 0x00); // 长度高位
    write_byte(buf, 0x00); // 长度低位
}

void nbt_end_compound(netbuf *buf) {
    write_byte(buf, TAG_End);
}

void nbt_begin_list(netbuf *buf, const char *name, nbt_tag_type element_type, int32_t count) {
    nbt_write_header(buf, TAG_List, name);
    write_byte(buf, (uint8_t)element_type);
    write_be32(buf, (uint32_t)count);
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
