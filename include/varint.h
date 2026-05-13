#ifndef INCLUDE_VARINT_H_
#define INCLUDE_VARINT_H_

#include <stdint.h>

#include "netbuf.h"

typedef int32_t varint;
typedef int64_t varlong;

varint read_varint(netbuf *buf);
varlong read_varlong(netbuf *buf);
uint16_t read_ushort(netbuf *buf);
char *read_string(netbuf *buf, varint *out_len);

void write_varint(netbuf *buf, varint value);
void write_varlong(netbuf *buf, varlong value);
void write_string(netbuf *buf, const char *str);

#endif // INCLUDE_VARINT_H_
