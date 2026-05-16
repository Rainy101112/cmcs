#ifndef INCLUDE_LOGIN_H_
#define INCLUDE_LOGIN_H_

#include "netbuf.h"
#include "protocol.h"

void write_minimal_registry(netbuf *buf);
void send_login_success(client_connection *conn);
void send_login_play(client_connection *conn);
void send_player_position(client_connection *conn);
void send_keep_alive(client_connection *conn);
void send_player_info_update(client_connection *conn);
void send_game_event(client_connection *conn, uint8_t event, float value);
void send_chunk_data(client_connection *conn, int32_t chunk_x, int32_t chunk_z);
void send_chunk_center(client_connection *conn, int32_t chunk_x, int32_t chunk_z);
void send_world_loading(client_connection *conn);

#endif // INCLUDE_LOGIN_H_
