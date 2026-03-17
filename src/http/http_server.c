#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../server.h"
#include "../platform.h"   // <-- fix: replaces direct socket includes

void http_router(int klient_sock, char *method, char *path, char *query);

void* http_server_start(void* arg) {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        LOG_ERR("HTTP: Failed to create socket");
        return NULL;
    }

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    struct sockaddr_in address = {
        .sin_family      = AF_INET,
        .sin_port        = htons(PORT_HTTP),
        .sin_addr.s_addr = inet_addr(HOST),
    };

    if (bind(server_sock, (struct sockaddr*)&address, sizeof(address)) < 0) {
        LOG_ERR("HTTP: bind() failed — do you need sudo?");
        return NULL;
    }

    listen(server_sock, 10);

    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        int client_sock = accept(server_sock,
            (struct sockaddr*)&client_address, &client_len);

        if (client_sock < 0) continue;

        LOG_INFO("HTTP: New connection from %s", inet_ntoa(client_address.sin_addr));

        // Receive request
        char buffer[BUFOR_HTTP] = {0};
        recv(client_sock, buffer, BUFOR_HTTP - 1, 0);

        // Parse first line: "GET /path?query HTTP/1.1"
        char method[8]       = {0};
        char full_path[512]  = {0};
        sscanf(buffer, "%7s %511s", method, full_path);

        // Split path and query string
        char path[256]  = {0};
        char query[256] = {0};
        char *question_mark = strchr(full_path, '?');

        if (question_mark) {
            strncpy(path,  full_path, question_mark - full_path);
            strncpy(query, question_mark + 1, sizeof(query) - 1);
        } else {
            strncpy(path, full_path, sizeof(path) - 1);
        }

	    // If POST — extract body and append to query
        // GG3 sends registration params in POST body
        if (strcmp(method, "POST") == 0) {
            char *body_start = strstr(buffer, "\r\n\r\n");
            if (body_start) {
                body_start += 4;  // skip \r\n\r\n
                if (strlen(query) > 0) {
                    // Already have query params — append with &
                    strncat(query, "&", sizeof(query) - strlen(query) - 1);
                }
                strncat(query, body_start, sizeof(query) - strlen(query) - 1);
            }
        }

        LOG_INFO("HTTP: %s %s", method, path);

        http_router(client_sock, method, path, query);

        close(client_sock);
    }

    close(server_sock);
    return NULL;
}