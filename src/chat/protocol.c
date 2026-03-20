#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../server.h"
#include "../platform.h"
#include "../database/users.h"
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

// authorization (for older versions pre-sha1)
int authorize(uint32_t uin, uint32_t seed, uint32_t hash) {
    User *user = db_find_by_uin(uin);
    if (!user || !user->password[0]) {
        LOG_WARN("AUTH: UIN %u not found or empty password", uin);
        return 0;
    }

    unsigned int computed = gg_login_hash(
        (const unsigned char*)user->password, seed);

    return (computed == hash);
}