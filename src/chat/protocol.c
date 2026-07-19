#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../server.h"
#include "../platform.h"
#include "../database/users.h"
#include "../hash/sha1.h"
#include "client.h"
#include "protocol.h"


// seed
uint32_t generate_seed() {
    return ((uint32_t)rand() << 16) ^ (uint32_t)rand();
}

// GG32 hashing algorithm
unsigned int gg_login_hash(const unsigned char *password, uint32_t seed) {
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
        z = y & 0x1f;
        y = (y << z) | (y >> (32 - z));
    }
    return y;
}


// implementacja hasha SHA1 w hash/sha1.h
void gg_login_hash_sha1(const char *pass, uint32_t seed, uint8_t *result){
	sha1_ctx_t ctx;
	
    sha1_init(&ctx);
    sha1_update(&ctx, (const uint8_t*)pass, strlen(pass));
    sha1_update(&ctx, (uint8_t *)&seed, 4);
    sha1_final(&ctx, result);
}

// authorization (for older versions pre-sha1)
int authorize(uint32_t uin, uint32_t seed, uint32_t hash) {
    User user;
    if (!db_find_by_uin(uin, &user) || !user.password[0]) {
        LOG_WARN("AUTH: UIN %u not found or empty password", uin);
        return 0;
    }
    unsigned int computed = gg_login_hash((const unsigned char*)user.password, seed);
    return (computed == hash);
}

int authorize70(uint32_t uin, uint32_t seed, uint8_t *response) {
    User user;
    uint8_t hash[20];
    int i;

    if (!db_find_by_uin(uin, &user) || !user.password[0]) {
        LOG_WARN("AUTH70: UIN %u not found or empty password", uin);
        return 0;
    }

    gg_login_hash_sha1((const char *)user.password, seed, hash);
    for (i = 0; i < 20; i++)
        if (response[i] != hash[i]) return 0;
    return 1;
}
