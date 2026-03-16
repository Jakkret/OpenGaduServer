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

void handle_fmregister(int sock, char *query) {
    char email[128]   = {0};	// v3.1
    char pwd[64]      = {0};	// neccesary
    char code[32]     = {0};	// neccesary
    char tokenid[64]  = {0};	// 3.1
    char tokenval[32] = {0};	// v3.1
	char qa[256]	  = {0};	// v5 

    // Parse all fields from POST body
    get_param(query, "email",    email,    sizeof(email));
    get_param(query, "pwd",      pwd,      sizeof(pwd));
    get_param(query, "code",     code,     sizeof(code));	// only used in pubdir.. not really having a use for now
	get_param(query, "qa", 		 qa,	   sizeof(qa));		// useless

    LOG_INFO("FMREGISTER: email=%s code=%s tokenid=%s tokenval=%s",
        email[0]    ? email    : "?",
        code[0]     ? code     : "?",
		qa[0] 		? qa 	   : "?"
    );

    // Validate required fields
    if (!email[0] || !pwd[0]) {
        LOG_WARN("FMREGISTER: Missing email or pwd");
        http_send_response(sock, 400, "Bad Request",
            "Missing email or pwd parameter\n");
        return;
    }

    // GG3 protocol: password longer than 16 chars = error3
    if (strlen(pwd) > MAX_PASSWORD_LEN) {
        LOG_WARN("FMREGISTER: Password too long (%zu chars) for email: %s",
            strlen(pwd), email);
        http_send_response(sock, 200, "OK", "error3\n");
        return;
    }

    // Token: we accept it always, no validation needed
    LOG_INFO("FMREGISTER: Token accepted (tokenid=%s)", tokenid[0] ? tokenid : "?");

    // Register user in database
    int uin = db_register(email, pwd);

    char body[64];
    if (uin > 0) {
        snprintf(body, sizeof(body), "reg_success:%d\n", uin);
        LOG_OK("FMREGISTER: Registered UIN %d for email %s", uin, email);
        http_send_response(sock, 200, "OK", body);
    } else {
        // Email already exists
        LOG_WARN("FMREGISTER: Email already registered: %s", email);
        http_send_response(sock, 200, "OK", "error2\n");
    }
}