#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "packet.h"
#include "protocol.h"
#include "handshake.h"
#include "uuid.h"
#include "login.h"

const char* status_json_template = 
    "{"
        "\"version\": {\"name\": \"1.20.1\", \"protocol\": 763},"
        "\"players\": {\"max\": 20, \"online\": 0},"
        "\"description\": {\"text\": \"Hello from C Server!\"}"
    "}";

void handle_packet(client_connection *conn) {
    varint packet_id = read_varint(conn->in_buf);

    if (conn->state == STATE_HANDSHAKING) {
        if (packet_id == 0x00) {
            handle_handshake(conn, packet_id); 
        }
    } else if (conn->state == STATE_STATUS) {
        if (packet_id == 0x00) {
            handle_status_request(conn);
        } else if (packet_id == 0x01) {
            handle_ping(conn);
        }
    } else if (conn->state == STATE_LOGIN) {
        if (packet_id == 0x00) {
            handle_login_start(conn);
        }
    } else if (conn->state == STATE_PLAY) {
        return;
    }
}

void handle_status_request(client_connection *conn) {
    netbuf *packet = netbuf_new(1024);

    write_varint(packet, 0x00);                 // Packet ID (Status Response = 0x00)
    write_string(packet, status_json_template); // JSON string
    netbuf_send(conn->socket, packet);          // Wrap and send

    netbuf_free(packet);
    printf("Sent Status Response JSON\n");
}

void handle_ping(client_connection *conn) {
    /* Get timestamp */
    varlong payload = read_varlong(conn->in_buf);
    
    /* Response packet */
    netbuf *packet = netbuf_new(16);
    write_varint(packet, 0x01);         // Ping Response ID
    write_varlong(packet, payload);
    
    netbuf_send(conn->socket, packet);
    netbuf_free(packet);
    printf("Sent Ping Response\n");
}

void handle_login_start(client_connection *conn) {
    /* Read player name */
    varint name_len;
    char *name_ptr = read_string(conn->in_buf, &name_len);
    
    /* Save player name to connection context */
    conn->player_name = malloc(name_len + 1);
    memcpy(conn->player_name, name_ptr, name_len);
    conn->player_name[name_len] = '\0';

    // 1.19.x+ 客户端会在 Login Start 后面带一个 UUID，这里简单跳过或读取
    // 离线模式通常由服务端自行根据玩家名生成一个 UUID

    printf("Player %s is logging in...\n", conn->player_name);

    send_login_success(conn);

    /* Change state to play */
    conn->state = STATE_PLAY;
    printf("Player %s joined the game!\n", conn->player_name);

    send_login_play(conn);
    send_world_loading(conn);
}
