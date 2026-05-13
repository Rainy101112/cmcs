#ifndef INCLUDE_PACKET_H_
#define INCLUDE_PACKET_H_

#include "varint.h"
#include "protocol.h"

void handle_packet(client_connection *conn);
void handle_status_request(client_connection *conn);
void handle_ping(client_connection *conn);

#endif // INCLUDE_PACKET_H_
