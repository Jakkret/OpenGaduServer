#include <stdio.h>
#include <string.h>
#include "../server.h"
#include "../platform.h"
#include "http_utils.h"
#include "fmregister.h"

// Declare handlers
void handle_appsvc(int sock, char *query);
void handle_fmregister(int sock, char *query);	// http/fmregister.c

void http_router(int client_sock, char *method, char *path, char *query) {

   /* Route to the correct handler 
	*
	* please make sure u add == 0 to any comparing argument, 
	* it makes a large mess. (15.03.2026)
	*/ 
	
	
    if (strcmp(path, "/appsvc/appmsg3.asp") == 0 || strcmp(path, "/appsvc/appmsg2.asp") == 0) {
        handle_appsvc(client_sock, query);

	// fmregister2.asp - v5 version request
	// fmregister.asp - v3.1 version request
	
    } else if (strcmp(path, "/appsvc/fmregister.asp") == 0 || strcmp(path, "appsvc/fmregister2.asp") == 0) {
        handle_fmregister(client_sock, query);

	} else {
        LOG_WARN("HTTP: Unknown path: %s", path);
        http_send_response(client_sock, 404, "Not Found",
            "404 - Unknown path\n");
    }
}