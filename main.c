#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "varint.h"
#include "handshake.h"
#include "protocol.h"
#include "packet.h"

#define PORT 25565

int start_server();
void run_event_loop(int server_fd);
void process_client_packets(client_connection *conn);

int start_server() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return -1;
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        return -1;
    }

    printf("Minecraft C Server started on port %d\n", PORT);
    return server_fd;
}

void run_event_loop(int server_fd) {
    int client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        printf("Client connected!\n");

        /* Initialize connection context */
        client_connection conn;
        conn.socket = client_socket;
        conn.state = STATE_HANDSHAKING;
        conn.in_buf = netbuf_new(4096);
        conn.player_name = NULL;
        conn.uuid = NULL;

        process_client_packets(&conn); 

        close(client_socket);
        netbuf_free(conn.in_buf);
        printf("Client disconnected.\n");
    }
}

void process_client_packets(client_connection *conn) {
    int client_socket = conn->socket;
    netbuf *in_buf = conn->in_buf;

    while (1) {
        /* Check buffer remaining space, do not read if too small,
         or we'll get out of boundary */
        if (in_buf->capacity - in_buf->size < 128) {
             // TODO: Expand capicity
             break; 
        }

        ssize_t bytes_received = recv(client_socket, in_buf->data + in_buf->size, 
                                     in_buf->capacity - in_buf->size, 0);
        
        if (bytes_received <= 0) break; 
        in_buf->size += bytes_received;

        while (1) {
            size_t save_offset = in_buf->offset;
            in_buf->error = false; 
            
            varint packet_len = read_varint(in_buf);

            if (in_buf->error) {
                in_buf->offset = save_offset;
                break;
            }

            if (in_buf->size - in_buf->offset < (size_t)packet_len) {
                in_buf->offset = save_offset;
                break;
            }

            size_t packet_end = in_buf->offset + packet_len;

            handle_packet(conn); 

            in_buf->offset = packet_end;
            if (in_buf->offset >= in_buf->size) break;
        }
        netbuf_compact(in_buf);
    }
}

int main(){
    int server_fd = start_server();
    if (server_fd < 0) {
        return 0;
    }

    run_event_loop(server_fd);

    return 0;
}
