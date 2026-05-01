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
 *	State: works (31.03.2026)
 */

void handle_fmregister(int sock, char *query, int version) {
	int uin = 0;
	char email[128]   = {0};
	char pwd[64]      = {0};
	char qa[256]	  = {0};	// pytanie bezpieczeństwa - v5.0
	char code[32]     = {0};
	char tokenid[64]  = {0};
	char tokenval[32] = {0};

	
	
	LOG_INFO("FMREGISTER: handler received '%d' path", version);

	switch(version){
		case 5:	{	// Version 5
		
			// read parameters off HTTP request
			get_param(query, "pwd", pwd, sizeof(pwd));
			get_param(query, "code", code, sizeof(code));
			get_param(query, "qa", qa, sizeof(qa));
			
			// doprecyzuj, że to odbiera fmregister wersji 2 (fmregister2.asp)
			LOG_INFO("FMREGISTER2: received client request with code=%s", code);
			
			    // Validate required fields
			if (!qa[0] || !pwd[0]) {
				LOG_WARN("FMREGISTER2: Missing qa or pwd");
				http_send_response(sock, 400, "Bad Request",
					"Missing qa or pwd parameter\n");
				return;
			}
			
			// send error3 if pwd exceeds 16 characters
			if (strlen(pwd) > MAX_PASSWORD_LEN) {
				LOG_WARN("FMREGISTER2: Password too long (%zu chars) for code: %d",
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
		
		
		case 3: {	// Wersja 3.1 build 25
		
			
			get_param(query, "email", email, sizeof(email));
			get_param(query, "pwd", pwd, sizeof(pwd));
			get_param(query, "code", code, sizeof(code));	// chyba jedynie używany w pubdir

			// bezużyteczne. (EDIT: a jednak nie! - 1.05.2026)
			get_param(query, "tokenid", tokenid, sizeof(tokenid));
			get_param(query, "tokenval", tokenval, sizeof(tokenval));

			LOG_INFO("FMREGISTER: User registered, email=%s, code=%s",
				email[0] ? email : "?",
				code[0] ? code : "?");

			if(!email[0] || !pwd[0]){
				LOG_WARN("FMREGISTER: Missing email or password");
				http_send_response(sock, 400, "Bad Request", "Missing email or pwd parameter\n");
			}

			// sprawdzaj czy hasło przewyższa limit 16 znaków
			if (strlen(pwd) > MAX_PASSWORD_LEN) {
				LOG_WARN("FMREGISTER2: Password too long (%zu chars) for code: %d",
				strlen(pwd), code);
				http_send_response(sock, 200, "OK", "error3\n");
				return;
			}

			// zarejestruj do bazy danych
			uin = db_register(email, pwd, NULL);

			char body[64];
			if(uin > 0){
				snprintf(body, sizeof(body), "reg-success:%d\n", uin);
				LOG_OK("FMREGISTER: Registered UIN %d for email %s", uin, email);
				http_send_response(sock, 200, "OK", body);
			} else {
				LOG_WARN("FMREGISTER: email already registered: %s", email);
				http_send_response(sock, 200, "OK", "error2\n");
			}

			break;
		}
		
		case 6: { // wersja 6.0 build 142
			
			// kolejność według zapisu w podaniu
			get_param(query, "pwd", pwd, sizeof(pwd));
			get_param(query, "email", email, sizeof(email));
			get_param(query, "tokenid", tokenid, sizeof(tokenid));
			get_param(query, "tokenval", tokenval, sizeof(tokenval));
			get_param(query, "code", code, sizeof(code));
			
			LOG_INFO("FMREGISTER3: User registered, email=%s, code=%s",
				email[0] ? email : "?",
				code[0] ? code : "?");
				
			if(!email[0] || !pwd[0]){
				LOG_WARN("FMREGISTER: Missing email or password");
				http_send_response(sock, 400, "Bad Request", "Missing email or pwd parameter\n");
			}
			
			if (strlen(pwd) > MAX_PASSWORD_LEN) {
				LOG_WARN("FMREGISTER2: Password too long (%zu chars) for code: %d",
				strlen(pwd), code);
				http_send_response(sock, 200, "OK", "error3\n");
				return;
			}
			
			uin = db_register(email, pwd, NULL);
			
			char body[64];
			if(uin > 0){
				snprintf(body, sizeof(body), "reg-success:%d\n", uin);
				LOG_OK("FMREGISTER3: Registered UIN %d for email %s", uin, email);
				http_send_response(sock, 200, "OK", body);
			} else {
				LOG_WARN("FMREGISTER3: email already registered: %s", email);
				http_send_response(sock, 200, "OK", "error2\n");
			}

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