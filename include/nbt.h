#ifndef NBT_H
#define NBT_H

#include <stdint.h>

#include "netbuf.h"

typedef enum {
    TAG_End = 0,
    TAG_Byte = 1,
    TAG_Short = 2,
    TAG_Int = 3,
    TAG_Long = 4,
    TAG_Float = 5,
    TAG_Double = 6,
    TAG_Byte_Array = 7,
    TAG_String = 8,
    TAG_List = 9,
    TAG_Compound = 10,
    TAG_Int_Array = 11,
    TAG_Long_Array = 12
} nbt_tag_type;

void nbt_write_header(netbuf *buf, nbt_tag_type type, const char *name);
void nbt_begin_compound(netbuf *buf, const char *name);
void nbt_begin_root_compound(netbuf *buf);
void nbt_end_compound(netbuf *buf);
void nbt_begin_list(netbuf *buf, const char *name, nbt_tag_type element_type, int32_t count);
void nbt_end_list(netbuf *buf);
void nbt_write_string_value(netbuf *buf, const char *value);

void nbt_write_byte(netbuf *buf, const char *name, int8_t value);
void nbt_write_short(netbuf *buf, const char *name, int16_t value);
void nbt_write_int(netbuf *buf, const char *name, int32_t value);
void nbt_write_long(netbuf *buf, const char *name, int64_t value);
void nbt_write_float(netbuf *buf, const char *name, float value);
void nbt_write_double(netbuf *buf, const char *name, double value);
void nbt_write_string(netbuf *buf, const char *name, const char *value);

void write_be32_to_buf(netbuf *buf, uint32_t value);
void write_long_to_buf(netbuf *buf, int64_t value);
void write_double_to_buf(netbuf *buf, double value);
void write_float_to_buf(netbuf *buf, float value);

// 调试工具
void nbt_debug_dump(const uint8_t *data, size_t size);

#endif