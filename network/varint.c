#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "varint.h"

varint read_varint(netbuf *buf) {
    uint32_t result = 0;
    int num_read = 0;
    uint8_t read;

    do {
        read = read_byte(buf);
        if (buf->error) return 0; // Quick back

        uint32_t value = (read & 0x7F);
        result |= (value << (7 * num_read));

        num_read++;
        if (num_read > 5) {
            buf->error = true; // Protocol violation
            return 0; 
        }
    } while ((read & 0x80) != 0);

    return (varint)result;
}

varlong read_varlong(netbuf *buf) {
    uint64_t result = 0;
    int num_read = 0;
    uint8_t read;

    do {
        read = read_byte(buf);
        if (buf->error) return 0; // Quick back

        uint64_t value = (read & 0x7F);
        result |= (value << (7 * num_read));

        num_read++;
        if (num_read > 10) {
            buf->error = true; // Protocol violation
            return 0; 
        }
    } while ((read & 0x80) != 0);

    return (varlong)result;
}

uint16_t read_ushort(netbuf *buf) {
    if (buf->error || buf->offset + 2 > buf->size) {
        buf->error = true;
        return 0;
    }
    /* Minecraft Protocol is Big-endian */
    uint16_t val = (buf->data[buf->offset] << 8) | buf->data[buf->offset + 1];
    buf->offset += 2;
    return val;
}

char *read_string(netbuf *buf, varint *out_len) {
    *out_len = read_varint(buf);
    if (buf->error || buf->offset + *out_len > buf->size) {
        buf->error = true;
        return NULL;
    }
    char *str = (char *)&buf->data[buf->offset];
    buf->offset += *out_len;

    return str; 
}

void write_varint(netbuf *buf, varint value) {
    uint32_t u_value = (uint32_t)value;
    do {
        uint8_t temp = (uint8_t)(u_value & 0x7F);
        u_value >>= 7;
        if (u_value != 0) {
            temp |= 0x80;
        }
        write_byte(buf, temp);
    } while (u_value != 0);
}

void write_varlong(netbuf *buf, varlong value) {
    uint64_t u_value = (uint64_t)value;
    do {
        uint8_t temp = (uint8_t)(u_value & 0x7F);
        u_value >>= 7;
        if (u_value != 0) {
            temp |= 0x80;
        }
        write_byte(buf, temp);
    } while (u_value != 0);
}

void write_string(netbuf *buf, const char *str) {
    int len = strlen(str);
    write_varint(buf, len);
    for (int i = 0; i < len; i++) {
        write_byte(buf, str[i]);
    }
}
