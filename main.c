#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#include "protocol.h"
#include "varint.h"
#include "netbuf.h"
#include "packet.h"
#include "logger.h"

#define MAX_EVENTS 64
#define PORT 25565

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int start_server() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        plog(LOG_ERROR "socket failed: %s", strerror(errno));
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        plog(LOG_ERROR "setsockopt failed: %s", strerror(errno));
        close(server_fd);
        return -1;
    }
    
    if (set_nonblocking(server_fd) < 0) {
        plog(LOG_ERROR "set_nonblocking failed: %s", strerror(errno));
        close(server_fd);
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        plog(LOG_ERROR "bind failed: %s", strerror(errno));
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        plog(LOG_ERROR "listen failed: %s", strerror(errno));
        close(server_fd);
        return -1;
    }

    plog(LOG_INFO "Minecraft server started on port %d", PORT);
    return server_fd;
}

int process_client_packets(client_connection *conn) {
    int client_socket = conn->socket;
    netbuf *in_buf = conn->in_buf;

    while (1) {
        /* Ensure at least 128 bytes of remaining space */
        if (in_buf->capacity - in_buf->size < 128) {
            netbuf_expand(in_buf, 128);
            if (in_buf->error) break;
        }

        ssize_t bytes_received = recv(client_socket, 
                                     in_buf->data + in_buf->size, 
                                     in_buf->capacity - in_buf->size, 0);
        
        if (bytes_received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                /* Read out */
                break; 
            }
            perror("recv error");
            return -1; // Real network error
        } else if (bytes_received == 0) {
            return 0; // Disconnect
        }

        in_buf->size += bytes_received;
    }

    while (1) {
        size_t save_offset = in_buf->offset;
        in_buf->error = false; 
        
        /* Get length */
        varint packet_len = read_varint(in_buf);

        if (in_buf->error) {
            in_buf->offset = save_offset;
            break;
        }

        if (in_buf->size - in_buf->offset < (size_t)packet_len) {
            in_buf->offset = save_offset;
            break;
        }

        /* Record current packet boundary */
        size_t packet_end = in_buf->offset + packet_len;

        /* Handle... */
        handle_packet(conn); 

        /* Force align */
        in_buf->offset = packet_end;

        /* Read out */
        if (in_buf->offset >= in_buf->size) {
            break;
        }
    }

    netbuf_compact(in_buf);

    return 1;
}

void run_epoll_loop(int server_fd) {
    int epoll_fd = epoll_create1(0);
    struct epoll_event ev, events[MAX_EVENTS];

    ev.events = EPOLLIN;
    ev.data.fd = server_fd; 
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == server_fd) {
                /* Process new connection */
                struct sockaddr_in client_addr;
                socklen_t addr_len = sizeof(client_addr);
                int client_sock = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
                
                if (client_sock == -1) continue;

                set_nonblocking(client_sock);
                
                /* Initialize connection context */
                client_connection *conn = malloc(sizeof(client_connection));
                conn->socket = client_sock;
                conn->state = STATE_HANDSHAKING;
                conn->in_buf = netbuf_new(4096);
                conn->player_name = NULL;
                conn->uuid = NULL;

                ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
                ev.data.ptr = conn;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sock, &ev);
                
                plog(LOG_INFO "New connection: FD %d", client_sock);

            } else {
                /* Process client data */
                client_connection *conn = (client_connection *)events[i].data.ptr;

                /* Check if disconnect event */
                if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                    goto disconnect;
                }

                int status = process_client_packets(conn);
                
                if (status <= 0) {
                disconnect:
                    plog(LOG_INFO "Client FD %d disconnected.", conn->socket);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conn->socket, NULL);
                    close(conn->socket);
                    if (conn->player_name) free(conn->player_name);
                    if (conn->uuid) free(conn->uuid);
                    netbuf_free(conn->in_buf);
                    free(conn);
                }
            }
        }
    }
}

static int g_server_fd = -1;

static void signal_handler(int sig) {
    (void)sig;
    plog(LOG_INFO "Server shutting down...");
    if (g_server_fd >= 0) close(g_server_fd);
    plog_close();
    exit(0);
}

int main() {
    plog_init("server.log");
    plog(LOG_INFO "Starting Minecraft server...");

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    int server_fd = start_server();
    if (server_fd < 0) {
        plog_close();
        return 1;
    }
    g_server_fd = server_fd;

    run_epoll_loop(server_fd);

    plog_close();
    return 0;
}
