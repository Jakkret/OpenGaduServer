#include <stdio.h>
#include <string.h>
#include "../server.h"
#include "../platform.h"
#include "http_utils.h"   // <-- fix: uses thread-safe get_param

/*
 *	TODO: direct v3.1 and v5 clients to chat server [partial]
 *	Prioritize v5 since its more documented
 
 *	TODO: separate appsvc handlers for each ver [completed]
 
 *	State: serves properly - success (17.03.2026)
 */

void handle_appsvc(int sock, char *query, int version) {
	char fmnumber[16]  = {0};

	switch(version){
		case 5: {		// version 5 - rev 2
			
			char tmpver[64] = {0};				// buffer --- urlencoded
			char clientVersion[64] = {0};		// here lies the "decoded" version string
			char lastmsg[64]= {0};
			
			get_param(query, "fmnumber", fmnumber, sizeof(fmnumber));
			get_param(query, "version", tmpver, sizeof(tmpver));
			url_decode(tmpver, clientVersion, sizeof(tmpver));
			
			// the important stuff are there.. rest to be added
			
			LOG_INFO("APPSVC v5: Client %s (ver. '%s') requests the IP addresses", 
				fmnumber[0] ? fmnumber : "?",
				clientVersion[0] ? clientVersion : "?"
			);
			
			break;
		} // end case
		
		case 3: {		// v3.1 -- incomplete
			
			char lastbanner[32] = {0};
			
			// TODO: add support for v3.1
			
			// based off what wireshark said about GG 3.1's GET request
			get_param(query, "fmnumber", fmnumber,  sizeof(fmnumber));	// here lies the UIN
			get_param(query, "lastbanner",  lastbanner, sizeof(lastbanner));	// whatever this is

			LOG_INFO("APPSVC: Client %s (lastbanner %s) requesting chat server address",
				fmnumber[0] ? fmnumber  : "?",
				lastbanner[0] ? lastbanner : "?"
			);
			
			break;
		}
		default:
			LOG_WARN("APPSVC: Unknown version %d", version);
            http_send_response(sock, 400, "Bad Request", "Unknown version\n");
            return;
		
	}



	/* Na to wychodzi że w czasach GG v5.0 używano
	 * dwóch stron appmsg - appmsg był dla v3.1
	 * a appmsg2 był dla v5.0.
	 *
	 * więcej w blogu z dnia 22.03.2026
	 */

    char body[64];
    snprintf(body, sizeof(body), "0 0 %s %s\n", HOST, HOST_UNAVAIL);		
	// currently it is:			  0 0 192.168.137.1 notoperating

    char response[256];
    snprintf(response, sizeof(response),
        "HTTP/1.0 200 OK\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s", body
    );

    send(sock, response, strlen(response), 0);
    LOG_OK("APPSVC: Sent chat server address to client %s", fmnumber[0] ? fmnumber : "?");
	
	
}
