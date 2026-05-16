#ifndef INCLUDE_NETBUF_H_
#define INCLUDE_NETBUF_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    uint8_t *data;
    size_t capacity; // Real capacity
    size_t size;     // Vaild data size
    size_t offset;   // R/W pointer offset
    bool error;      // Error sign
} netbuf;

netbuf *netbuf_new(size_t initial_capacity);
void netbuf_free(netbuf *buf);
void netbuf_expand(netbuf *buf, size_t min_additional);

uint8_t read_byte(netbuf *buf);
void write_byte(netbuf *buf, uint8_t byte);
void netbuf_compact(netbuf *buf);
void netbuf_send(int client_socket, netbuf *packet_buf);

#endif // INCLUDE_NETBUF_H_
