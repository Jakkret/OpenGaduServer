#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../server.h"
#include "../platform.h"   // <-- fix: replaces direct socket includes
#include "gg_login.h"

void* chat_server_start(void* arg){
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        LOG_ERR("CHAT: Failed to create socket");
        return NULL;
    }

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

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

    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        int client_sock = accept(server_sock,
            (struct sockaddr*)&client_address, &client_len);

        if (client_sock < 0) continue;

        LOG_INFO("CHAT: New connection from %s", inet_ntoa(client_address.sin_addr));
		
		handle_logging(client_sock);
        close(client_sock);
    }

    close(server_sock);
    return NULL;
}