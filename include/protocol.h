#ifndef INCLUDE_PROTOCOL_H_
#define INCLUDE_PROTOCOL_H_

#include "netbuf.h"

typedef enum {
    STATE_HANDSHAKING,
    STATE_STATUS,
    STATE_LOGIN,
    STATE_PLAY
} connection_state;

typedef struct {
    int socket;
    connection_state state;
    netbuf *in_buf;
    char *player_name;
    char *uuid;
} client_connection;

#endif // INCLUDE_PROTOCOL_H_
