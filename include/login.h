#ifndef INCLUDE_LOGIN_H_
#define INCLUDE_LOGIN_H_

#include "netbuf.h"
#include "protocol.h"

void write_minimal_registry(netbuf *buf);
void send_login_success(client_connection *conn);
void send_login_play(client_connection *conn);
void send_player_position(client_connection *conn);

#endif // INCLUDE_LOGIN_H_
