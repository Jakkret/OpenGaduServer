#include <stdio.h>
#include <string.h>
#include "http_utils.h"
#include "../platform.h"
#include "../server.h"

int get_param(const char *query, const char *param, char *out, int out_size) {
    if (!query || !param || !out || out_size <= 0) return 0;

    char search[64];
    snprintf(search, sizeof(search), "%s=", param);

    const char *start = strstr(query, search);
    if (!start) return 0;

    start += strlen(search);

    int i = 0;
    while (start[i] && start[i] != '&' && start[i] != ' ' && i < out_size - 1) {
        out[i] = start[i];
        i++;
    }
    out[i] = '\0';

    return 1;
}

void http_send_response(int sock, int status_code, const char *status_text,
                        const char *body) {
    char response[2048];
    snprintf(response, sizeof(response),
        "HTTP/1.0 %d %s\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        status_code, status_text, body ? body : ""
    );
    send(sock, response, strlen(response), 0);
}