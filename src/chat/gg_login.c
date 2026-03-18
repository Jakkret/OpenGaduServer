#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "../server.h"
#include "../database/users.h"
#include "gg_login.h"

/*
 *	TODO: Support client v3.1 and v5 login.
 *	TODO: please clean up this mess
 *	Prioritize v5 since its more documented
 *
 *	State: partially works - sends out GG_WELCOME and logs in (17.03.2026 @ 21:05)
 */

void gg_send_welcome(int sock, uint32_t seed){
	GG_header header;
	GG_welcome welcome;
	
	header.type = GG_WELCOME;
	header.length = sizeof(GG_welcome);
	
	welcome.seed = seed;
	
	send(sock, (const char*)&header, sizeof(header), 0);
	send(sock, (const char*)&welcome, sizeof(welcome), 0);
	
	LOG_OK("CHAT: Sent GG_WELCOME with seed: 0x%08X", seed);
	
}

void gg_send_login_failed(int sock){
	
	GG_header header;
	header.type = GG_LOGIN_FAILED;
	header.length = 0;		// only header
	
	send(sock, (const char*)&header, sizeof(header), 0);
	LOG_WARN("CHAT: Sent GG_LOGIN_FAILED");
}

void gg_send_login_ok(int sock){
	
	GG_header header;
	header.type = GG_LOGIN_OK;
	header.length = 0;		// only header
	
	send(sock, (const char*)&header, sizeof(header), 0);
	LOG_OK("CHAT: Sent GG_LOGIN_OK");
}

// generate a random seed
uint32_t generate_seed() {
    return ((uint32_t)rand() << 16) ^ (uint32_t)rand();
}

// GG32 hash algorithm (source: libgadu/src/libgadu.c @ 209)
unsigned int gg_login_hash(const unsigned char *password, uint32_t seed){
	unsigned int x, y, z;

	y = seed;

	for (x = 0; *password; password++) {
		x = (x & 0xffffff00) | *password;
		y ^= x;
		y += x;
		x <<= 8;
		y ^= x;
		x <<= 8;
		y -= x;
		x <<= 8;
		y ^= x;

		z = y & 0x1F;
		y = (y << z) | (y >> (32 - z));
	}

	return y;
}


// verifies the information by counting hash (code based off USG)
int verify_login(int sock, GG_login5 usr, uint32_t seed) {
    User *searched = db_find_by_uin(usr.uin);
    if (!searched || !searched->password[0]) {
        LOG_WARN("CHAT: UIN %u not found or empty password", usr.uin);
        return 0;
    }
	
	unsigned long hash = gg_login_hash(
		(const unsigned char*)searched->password, seed);
	
	return (hash == usr.hash);
}

void handle_logging(int sock) {
    uint32_t seed = generate_seed();
    gg_send_welcome(sock, seed);

    GG_header header;
    recv(sock, (char*)&header, sizeof(header), 0);

    GG_login5 login;
	recv(sock, (char*)&login, sizeof(login), 0);

	// TODO optional: fix version string to be readable
	LOG_INFO("CHAT: UIN=%u hash=0x%08X status=0x%08X version=0x%08X",
		login.uin, login.hash, login.status, login.version);

    // Verify and respond
    if (verify_login(sock, login, seed)) {
        LOG_OK("CHAT: Login OK for UIN %u", login.uin);
        gg_send_login_ok(sock);

		// NOTE: v5.0 client loops through its saved users and
		// attempts to log in each one - that's why you might see 
		// more than one user trying to be logged - check the UINs!!
		
		GG_header next_header;
        while (recv(sock, (char*)&next_header, sizeof(next_header), 0) > 0) {
        LOG_INFO("CHAT: Packet type=0x%08X length=%u", 
                 next_header.type, next_header.length);


		}
        // TODO: support packets after logging (17.03.2026 @ 21:08)
        
    } else {	// if unsuccesful
        LOG_WARN("CHAT: Login FAILED for UIN %u", login.uin);
        gg_send_login_failed(sock);
    }
}
