#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../server.h"
#include "../platform.h"
#include "client.h"
#include "protocol.h"
#include "handlers.h"

void* client_thread(void* arg) {
    client_t *c = (client_t*) arg;

    // wyślij GG_WELCOME
    gg_welcome_t welcome;
    welcome.seed = c->seed;
    write_full_packet(c, GG_WELCOME, &welcome, sizeof(welcome));
    LOG_OK("CHAT: Sent GG_WELCOME to fd=%d seed=0x%08X", c->fd, c->seed);

    // pętla odbioru pakietów
    gg_header_t header;
    while (recv(c->fd, (char*)&header, sizeof(header), 0) > 0) {
        if (c->remove) break;

        // odczytaj dane pakietu
        void *data = NULL;
        if (header.length > 0) {
            data = malloc(header.length);
            if (!data) {
                LOG_ERR("CHAT: malloc failed for packet data");
                break;
            }
            int received = recv(c->fd, data, header.length, 0);
            if (received <= 0) {
                free(data);
                break;
            }
        }

        handle_input_packet(c, data, header.type, header.length);

        if (data) free(data);
        if (c->remove) break;
    }

    LOG_INFO("CHAT: Destroying session fd=%d UIN=%u", c->fd, c->uin);
    client_destroy(c);
    return NULL;
}

void* chat_server_start(void* arg) {
    clients_init();

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        LOG_ERR("CHAT: Failed to create socket");
        return NULL;
    }

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    struct sockaddr_in address = {
        .sin_family      = AF_INET,
        .sin_port        = htons(PORT_CHAT),
        .sin_addr.s_addr = inet_addr(HOST),
    };

    if (bind(server_sock, (struct sockaddr*)&address, sizeof(address)) < 0) {
        LOG_ERR("CHAT: bind() failed");
        return NULL;
    }

    listen(server_sock, 10);
    LOG_OK("CHAT: Listening on %s:%d", HOST, PORT_CHAT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_sock,
            (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) continue;

        LOG_INFO("CHAT: New connection from %s", inet_ntoa(client_addr.sin_addr));

        uint32_t seed = generate_seed();
        client_t *c = client_create(client_fd, client_addr, seed);
        if (!c) {
            close(client_fd);
            continue;
        }

        thread_t t;
        thread_create(&t, client_thread, c);
        thread_detach(t);
    }

    close(server_sock);
    return NULL;
}