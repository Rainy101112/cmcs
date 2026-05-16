#include <stdio.h>
#include <string.h>

#include "handshake.h"
#include "protocol.h"

void handle_handshake(client_connection *conn, varint packet_id) {
    if (packet_id != 0x00) {
        printf("Error: Expected Handshake ID 0x00, got 0x%02x\n", packet_id);
        return;
    }

    packet_handshake packet;

    packet.protocol_version = read_varint(conn->in_buf);
    
    varint addr_len;
    char *addr_ptr = read_string(conn->in_buf, &addr_len);
    if (addr_ptr && addr_len < 256) {
        memcpy(packet.server_address, addr_ptr, addr_len);
        packet.server_address[addr_len] = '\0'; // Add terminator
    }

    packet.server_port = read_ushort(conn->in_buf);
    packet.next_state = read_varint(conn->in_buf);

    if (!conn->in_buf->error) {
        printf("Handshake Received!\n");
        printf("  Version: %d\n", packet.protocol_version);
        printf("  Address: %s\n", packet.server_address);
        printf("  Port:    %d\n", packet.server_port);
        printf("  Next:    %s\n", (packet.next_state == 1 ? "Status" : "Login"));
    }

    if (packet.next_state == STATE_STATUS || packet.next_state == STATE_LOGIN) {
        conn->state = packet.next_state;
    } else {
        printf("Invalid next_state: %d, disconnecting\n", packet.next_state);
        conn->in_buf->error = true;
    }
}
