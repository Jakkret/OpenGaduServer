#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../server.h"
#include "../platform.h"
#include "../database/users.h"
#include "http_utils.h"
#include "fmregister.h"

#define MAX_PASSWORD_LEN 16


/*
 *	TODO: Support client v3.1 and v5 login.
 *	Prioritize v5 since its more documented
 *
 *	State: Dosen't work (16.03.2026)
 */

void handle_fmregister(int sock, char *query, int version) {
	int uin = 0;
	char email[128]   = {0};
	char pwd[64]      = {0};
	char qa[256]	  = {0};
	char code[32]     = {0};

	
	
	LOG_INFO("FMREGISTER: handler received '%d' path", version);

	switch(version){
		case 5:	{	// Version 5
		
			// read parameters off HTTP request
			get_param(query, "pwd", pwd, sizeof(pwd));
			get_param(query, "code", code, sizeof(code));
			get_param(query, "qa", qa, sizeof(qa));
			
			LOG_INFO("FMREGISTER: received client request with code=%s", code);
			
			    // Validate required fields
			if (!qa[0] || !pwd[0]) {
				LOG_WARN("FMREGISTER: Missing qa or pwd");
				http_send_response(sock, 400, "Bad Request",
					"Missing qa or pwd parameter\n");
				return;
			}
			
			// send error3 if pwd exceeds 16 characters
			if (strlen(pwd) > MAX_PASSWORD_LEN) {
				LOG_WARN("FMREGISTER: Password too long (%zu chars) for code: %d",
				strlen(pwd), code);
				http_send_response(sock, 200, "OK", "error3\n");
				return;
			}
			
			// register user and assign UIN
			uin = db_register(
				NULL, 
				pwd,
				qa[0] ? qa : NULL
			);
			
			break;
		}	// end case for v5
		
		
		case 3: {
			
		    char email[128]   = {0};	// v3.1
			char tokenid[64]  = {0};	// v3.1
			char tokenval[32] = {0};	// v3.1
			
			// TODO: later implement support for v3.1 protocol
			LOG_WARN("FMREGISTER: GG 3.x not yet implemented");
			break;
		}
		
		default:
            LOG_WARN("FMREGISTER: Unknown version %d", version);
            http_send_response(sock, 400, "Bad Request", "Unknown version\n");
            return;
		
	}	// switch end



    char body[64];
    if (uin > 0) {
        snprintf(body, sizeof(body), "reg_success:%d\n", uin);
        LOG_OK("FMREGISTER: Registered user code %d with qa %s", code, qa);
        http_send_response(sock, 200, "OK", body);
    } else {
        // Email already exists
        LOG_WARN("FMREGISTER: Code already registered: %s", code);
        http_send_response(sock, 200, "OK", "error2\n");
    }
}