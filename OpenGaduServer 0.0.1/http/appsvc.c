#include <stdio.h>
#include <string.h>
#include "../server.h"
#include "../platform.h"
#include "http_utils.h"   // <-- fix: uses thread-safe get_param

/*
 *	TODO: direct v3.1 and v5 clients to chat server
 *	Prioritize v5 since its more documented
 *
 *	State: Unknown (16.03.2026)
 */

void handle_appsvc(int sock, char *query) {
    char number[16]  = {0};
    char lastbanner[32] = {0};

	// based off what wireshark said about GG 3.1's GET request
    get_param(query, "fmnumber", number,  sizeof(number));	// here lies the UIN
    get_param(query, "lastbanner",  lastbanner, sizeof(lastbanner));	// whatever this is

    LOG_INFO("APPSVC: Client %s (lastbanner %s) requesting chat server address",
        number[0] ? number  : "?",
        lastbanner[0] ? lastbanner : "?"
    );

    char body[64];
    snprintf(body, sizeof(body), "0 0 %s:%d %s\n", HOST, PORT_CHAT, FALLBACK_HOST);		// 0 0 127.0.0.1:8074 127.0.0.1

    char response[256];
    snprintf(response, sizeof(response),
        "HTTP/1.0 200 OK\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s", body
    );

    send(sock, response, strlen(response), 0);
    LOG_OK("APPSVC: Sent chat server address to client %s", number[0] ? number : "?");
}