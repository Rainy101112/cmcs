#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "netbuf.h"
#include "varint.h"

netbuf *netbuf_new(size_t initial_capacity) {
    netbuf *buf = malloc(sizeof(netbuf));
    buf->data = malloc(initial_capacity);
    buf->capacity = initial_capacity;
    buf->size = 0;
    buf->offset = 0;
    buf->error = false;
    return buf;
}

void netbuf_free(netbuf *buf) {
    free(buf->data);
    free(buf);
}

void netbuf_expand(netbuf *buf, size_t min_additional) {
    size_t needed = buf->size + min_additional;
    if (needed <= buf->capacity) return;

    size_t new_capacity = buf->capacity;
    while (new_capacity < needed) {
        new_capacity *= 2;
    }

    uint8_t *new_data = realloc(buf->data, new_capacity);
    if (!new_data) {
        buf->error = true;
        return;
    }
    buf->data = new_data;
    buf->capacity = new_capacity;
}

uint8_t read_byte(netbuf *buf) {
    if (buf->error) return 0;

    if (buf->offset >= buf->size) {
        buf->error = true; // Out of boundary
        return 0;
    }
    return buf->data[buf->offset++];
}

void write_byte(netbuf *buf, uint8_t byte) {
    if (buf->error) return;

    /* Check if need expand */
    if (buf->offset >= buf->capacity) {
        size_t new_capacity = buf->capacity * 2;
        uint8_t *new_data = realloc(buf->data, new_capacity);
        if (!new_data) {
            buf->error = true;
            return;
        }
        buf->data = new_data;
        buf->capacity = new_capacity;
    }

    buf->data[buf->offset++] = byte;

    if (buf->offset > buf->size) {
        buf->size = buf->offset;
    }
}

/* Remove read data, move the rest of data to the start of buffer */
void netbuf_compact(netbuf *buf) {
    if (buf->offset > 0) {
        size_t remaining = buf->size - buf->offset;
        if (remaining > 0) {
            memmove(buf->data, buf->data + buf->offset, remaining);
        }
        buf->size = remaining;
        buf->offset = 0;
    }
}

void netbuf_send(int client_socket, netbuf *packet_buf) {
    netbuf *wrapper = netbuf_new(packet_buf->size + 5);
    write_varint(wrapper, (varint)packet_buf->size);
    size_t header_len = wrapper->offset;
    memcpy(wrapper->data + header_len, packet_buf->data, packet_buf->size);
    size_t total = header_len + packet_buf->size;

    printf("Real Bytes Sent: ");
    for(size_t i=0; i < (total < 10 ? total : 10); i++) {
        printf("%02X ", (uint8_t)wrapper->data[i]);
    }
    printf("\n");

    size_t sent = 0;
    while (sent < total) {
        ssize_t n = send(client_socket, wrapper->data + sent, total - sent, 0);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
            perror("send error");
            break;
        }
        sent += (size_t)n;
    }
    netbuf_free(wrapper);
}
