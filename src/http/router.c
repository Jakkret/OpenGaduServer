#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../server.h"
#include "../platform.h"
#include "http_utils.h"
#include "fmregister.h"
#include "../hash/md5.h"
#include "../config/config.h"

// Declare handlers
void handle_appsvc(int sock, char *query, int version);			// http/appsvc.c
void handle_fmregister(int sock, char *query, int gg_version);	// http/fmregister.c
void handle_regtoken(int sock); 								// http/regtoken.c

void http_router(int client_sock, char *method, char *path, char *query) {

   /* Route to the correct handler 
	*
	* please make sure u add == 0 to any comparing argument, 
	* it makes a large mess. (15.03.2026)
	*
	* TODO: make it more flexible for routing for v5 and v3.1
	*/ 
	
	// Porządki - 1.05.2026
	
	// PRZEKIEROWANIE
    if(strcmp(path, "/appsvc/appmsg.asp") == 0) {
        handle_appsvc(client_sock, query, 3);
	
    } else if(strcmp(path, "/appsvc/appmsg2.asp") == 0) {
        handle_appsvc(client_sock, query, 5);
		
	} else if(strcmp(path, "/appsvc/appmsg4.asp") == 0){
		handle_appsvc(client_sock, query, 6);
		
	}
	
	/* REJESTRACJA */ 
	else if(strcmp(path, "/appsvc/fmregister.asp") == 0) {
        handle_fmregister(client_sock, query, 3);

	} else if(strcmp(path, "/appsvc/fmregister2.asp") == 0) {
        handle_fmregister(client_sock, query, 5);
		
	} else if(strcmp(path, "/appsvc/regtoken.asp") == 0){
		handle_regtoken(client_sock);
		
	} else if(strcmp(path, "/appsvc/fmregister3.asp") == 0){
		handle_fmregister(client_sock, query, 6);
	} 
	
	/* KATALOG PUBLICZNY */
    else if(strcmp(path, "/appsvc/fmpubdetails2.asp") == 0){
		//h_pubdir_details_46(client_sock, query);
		return;
	} else {
        LOG_WARN("HTTP: Unknown path: %s", path);
        http_send_response(client_sock, 404, "Not Found",
            "404 - Unknown path\n");
    }
}
