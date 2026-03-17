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
 *	State: Dosen't work (16.03.2026)
 */

// Safe recv - ensures all bytes are read.. like why dont you read it properly???
static int recv_all(int sock, char *buf, int len) {
    int total = 0;
    while (total < len) {
        int r = recv(sock, buf + total, len - total, 0);
        if (r <= 0) return r;
        total += r;
    }
    return total;
}

void gg_send_welcome(int sock, uint32_t seed){
	GG_header header;
	GG_welcome welcome;
	
	header.type = GG_WELCOME;
	header.length = sizeof(GG_welcome);
	
	welcome.seed = htonl(seed);;
	
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
	LOG_WARN("CHAT: Sent GG_LOGIN_FAILED");
}

// generate a random seed
static uint32_t generate_seed() {
    return ((uint32_t)rand() << 16) ^ (uint32_t)rand();
}

// GG32 hash algorithm (change to original one - check libgadu/src/libgadu.c @ 209)
uint32_t gg_login_hash_old(const unsigned char *password, uint32_t seed) {
    uint32_t hash = seed;
    while (*password) {
        hash ^= (uint32_t)*password++;
        hash += (hash << 1);
        hash ^= (hash >> 3);
        hash += (hash << 5);
        hash ^= (hash >> 7);
    }
    return hash;
}


// checks if credentials match with what client gave out on GG_LOGIN
int verify_login(int sock, GG_login3 usr, uint32_t seed) {
    User *searched = db_find_by_uin(usr.uin);
    if (!searched) {
        LOG_WARN("CHAT: UIN %u not found", usr.uin);
        return 0;
    }

    // TODO: fix hash verification later
    LOG_WARN("CHAT: Hash check skipped - accepting UIN %u", usr.uin);
    return 1;
}

// needs a real cleanup

void handle_logging(int sock) {
    uint32_t seed = generate_seed();
    gg_send_welcome(sock, seed);

    GG_header header;
    recv_all(sock, (char*)&header, sizeof(header));

    unsigned char raw[64] = {0};
    int len = recv_all(sock, (char*)raw, header.length);

    LOG_INFO("CHAT: Raw login bytes (%d):", len);
    for (int i = 0; i < len; i++) {
        printf("%02X ", raw[i]);
    }
    printf("\n");

	uint32_t *fields = (uint32_t*)raw;
	LOG_INFO("CHAT: field[0]=0x%08X", fields[0]); // uin
	LOG_INFO("CHAT: field[1]=0x%08X", fields[1]); // hash?
	LOG_INFO("CHAT: field[2]=0x%08X", fields[2]); // status?

	// Also print byte by byte
	LOG_INFO("CHAT: byte[4]=0x%02X byte[5]=0x%02X byte[6]=0x%02X byte[7]=0x%02X",
		raw[4], raw[5], raw[6], raw[7]);


    GG_login3 *login = (GG_login3*)raw;
    LOG_INFO("CHAT: UIN=%u hash=0x%08X status=0x%08X",
        login->uin, login->hash, login->status);

    // Verify and respond
    if (verify_login(sock, *login, seed)) {
        LOG_OK("CHAT: Login OK for UIN %u", login->uin);
        gg_send_login_ok(sock);

        // Keep connection open and watch what client sends next
        unsigned char next[256] = {0};
        GG_header next_header;
        int r = recv_all(sock, (char*)&next_header, sizeof(next_header));
        if (r > 0) {
            LOG_INFO("CHAT: After LOGIN_OK client sent type=0x%08X length=%u",
                next_header.type, next_header.length);
        }
    } else {
        LOG_WARN("CHAT: Login FAILED for UIN %u", login->uin);
        gg_send_login_failed(sock);
    }
}
