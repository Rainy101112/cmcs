#ifndef INCLUDE_HANDSHAKE_H_
#define INCLUDE_HANDSHAKE_H_

#include "varint.h"
#include "protocol.h"

typedef struct {
    varint protocol_version;
    char server_address[256]; // Exactly, we can make it dynamic
    uint16_t server_port;
    varint next_state;
} packet_handshake;

void handle_handshake(client_connection *conn, varint packet_id);

#endif // INCLUDE_HANDSHAKE_H_
