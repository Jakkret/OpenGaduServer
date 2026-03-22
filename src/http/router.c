#include <stdio.h>
#include <string.h>
#include "../server.h"
#include "../platform.h"
#include "http_utils.h"
#include "fmregister.h"

// Declare handlers
void handle_appsvc(int sock, char *query, int version);		// http/appsvc.c
void handle_fmregister(int sock, char *query, int gg_version);	// http/fmregister.c

void http_router(int client_sock, char *method, char *path, char *query) {

   /* Route to the correct handler 
	*
	* please make sure u add == 0 to any comparing argument, 
	* it makes a large mess. (15.03.2026)
	*
	* TODO: make it more flexible for routing for v5 and v3.1
	*/ 
	
	
    if (strcmp(path, "/appsvc/appmsg2.asp") == 0) {
        handle_appsvc(client_sock, query, 5);		// for now statically specified with v5 protocol

	// fmregister2.asp - v5 version request
	// fmregister.asp - v3.1 version request
	
    } else if (strcmp(path, "/appsvc/fmregister2.asp") == 0) {
        handle_fmregister(client_sock, query, 5);

	
	} else if (strcmp(path, "/getbanner.asp") == 0 ) {
		
		// serve a gif banner to client - experimental
		//
		// TODO: wyślij zdjęcie, a nie binarny zapis tego ._.
		FILE *f = fopen("img/test.png", "rb");
		if(!f) {
			http_send_response(client_sock, 404, "Not Found",
            "404 - Not Found\n");
			return;
		}
		
		// read file size
		fseek(f, 0, SEEK_END);
		long size = ftell(f);
		fseek(f, 0, SEEK_SET);
		
		// read file off buffer
		char *buf = malloc(size);
		fread(buf, 1, size, f);
		fclose(f);
		
		char header[256];
		snprintf(header, sizeof(header),
			"HTTP/1.0 200 OK\r\n",
			"Content-Type: image/gif\r\n"
			"Content-Length: %ld\r\n"
			"\r\n", size);
			
		send(client_sock, header, strlen(header), 0);
		
		send(client_sock, buf, size, 0);
		free(buf);
		
	} else {
        LOG_WARN("HTTP: Unknown path: %s", path);
        http_send_response(client_sock, 404, "Not Found",
            "404 - Unknown path\n");
    }
}
