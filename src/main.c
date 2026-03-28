#include <stdio.h>
#include <stdlib.h>
#include "server.h"		// config for the server - TODO: make it with simple svr_conf.ini file
#include "platform.h"   // <-- double compatibility with Win32 and Linux
#include "database/users.h"
#include "chat/protocol.h"
#include "chat/client.h"
#include "config/config.h"

void* http_server_start(void* arg);
void* chat_server_start(void* arg);

ServerConf sHTTP, sCHAT;

int main() {
    // Initialize platform (required on Windows for Winsock)
	if (platform_init() != 0) {
		LOG_ERR("Failed to initialize network library");
        return 1;
    }

    // Load user local database
    if (db_init() != 0) {
        LOG_ERR("Failed to initialize user database");
        platform_cleanup();
        return 1;
    }

    printf("\n");
    printf("  +------------------------------+\n");
    printf("  |    Open Gadu Server 0.0.1    |\n");
    printf("  |    still in development      |\n");
    printf("  +------------------------------+\n");

#ifdef PLATFORM_WINDOWS
    printf("  |    Platform: Windows         |\n");
#else
    printf("  |    Platform: Linux           |\n");
#endif
    printf("  +------------------------------+\n\n");

    thread_t thread_http, thread_chat;
	
	ReadConfig(CONFIG_FILENAME, &sCHAT, &sHTTP);

    if (thread_create(&thread_http, http_server_start, NULL) != 0) {
        LOG_ERR("Failed to start HTTP server thread");
        platform_cleanup();
        return 1;
    }
    LOG_OK("HTTP server started on %s:%d", sHTTP.IPaddr, sHTTP.Port);

    if (thread_create(&thread_chat, chat_server_start, NULL) != 0) {
        LOG_ERR("Failed to start chat server thread");
        platform_cleanup();
        return 1;
    }
    LOG_OK("Chat server started on %s:%d", sCHAT.IPaddr, sCHAT.Port);

    LOG_INFO("Both servers running. Press Ctrl+C to stop.\n");

    thread_join(thread_http);
    thread_join(thread_chat);

    platform_cleanup();
    return 0;
}
