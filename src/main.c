#include <stdio.h>
#include <stdlib.h>
#include "server.h"
#include "platform.h"   // <-- double compatibility with Win32 and Linux
#include "database/users.h"
#include "chat/protocol.h"
#include "chat/client.h"
#include "config/config.h"
#include "dashboard/dashboard.h"

void* http_server_start(void* arg);
void* chat_server_start(void* arg);
void* http_admin_start(void* arg);

ServerConf sHTTP, sCHAT, sADMIN;

int main() {
	
	int svr_count = 0;
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
    printf("  |    Open Gadu Server 0.0.2    |\n");
    printf("  |    Development Release 2     |\n");
    printf("  +------------------------------+\n");

#ifdef PLATFORM_WINDOWS
    printf("  |    Platform: Windows         |\n");
#else
    printf("  |    Platform: Linux           |\n");
#endif
    printf("  +------------------------------+\n\n");

    thread_t thread_http, thread_chat, thread_admin;
	
	sADMIN.IPaddr = HTTP_ADMIN_DEFAULT;
	sADMIN.Port   = HTTP_ADMIN_PORT;
	
	ReadConfig(CONFIG_FILENAME, &sCHAT, &sHTTP, &sADMIN);

    if (thread_create(&thread_http, http_server_start, NULL) != 0) {
        LOG_ERR("Failed to start HTTP server thread");
        platform_cleanup();
        return 1;
    }
    LOG_OK("HTTP service started on %s:%d", sHTTP.IPaddr, sHTTP.Port);
	svr_count++;

    if (thread_create(&thread_chat, chat_server_start, NULL) != 0) {
        LOG_ERR("Failed to start chat server thread");
        platform_cleanup();
        return 1;
    }
    LOG_OK("Chat service started on %s:%d", sCHAT.IPaddr, sCHAT.Port);
	svr_count++;
	
	if(thread_create(&thread_admin, http_admin_start, NULL) != 0){
		LOG_ERR("Failed to start web admin interface thread\n");
		platform_cleanup();
		return 1;
	}
	LOG_OK("Web Administer Interface (WAI) is running on: %s:%d\n", sADMIN.IPaddr, sADMIN.Port);
	svr_count++;

    LOG_INFO("%d servers (services) running. Press Ctrl+C to stop.\n", svr_count);

    thread_join(thread_http);
    thread_join(thread_chat);
	thread_join(thread_admin);

    platform_cleanup();
    return 0;
}
