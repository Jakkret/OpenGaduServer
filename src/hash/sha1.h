#ifndef SHA1_H
#define SHA1_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

/*
 * Simple SHA-1 implementation - no external dependencies.
 * Works on both Windows and Linux.
 */

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    uint8_t  buffer[64];
} sha1_ctx_t;

static inline uint32_t sha1_rol(uint32_t v, unsigned n) {
    return (v << n) | (v >> (32U - n));
}

static void sha1_transform(uint32_t state[5], const uint8_t *buf) {
    uint32_t a, b, c, d, e, t, w[80];
    int i;

    for (i = 0; i < 16; i++) {
        w[i] = ((uint32_t)buf[i * 4]     << 24) |
               ((uint32_t)buf[i * 4 + 1] << 16) |
               ((uint32_t)buf[i * 4 + 2] <<  8) |
               ((uint32_t)buf[i * 4 + 3]);
    }

    for (i = 16; i < 80; i++) {
        w[i] = sha1_rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }

    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    for (i = 0; i < 80; i++) {
        if (i < 20) {
            t = sha1_rol(a, 5) + ((b & c) | (~b & d)) + e + w[i] + 0x5A827999U;
        } else if (i < 40) {
            t = sha1_rol(a, 5) + (b ^ c ^ d) + e + w[i] + 0x6ED9EBA1U;
        } else if (i < 60) {
            t = sha1_rol(a, 5) + ((b & c) | (b & d) | (c & d)) + e + w[i] + 0x8F1BBCDCU;
        } else {
            t = sha1_rol(a, 5) + (b ^ c ^ d) + e + w[i] + 0xCA62C1D6U;
        }

        e = d;
        d = c;
        c = sha1_rol(b, 30);
        b = a;
        a = t;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

static inline void sha1_init(sha1_ctx_t *ctx) {
    ctx->state[0] = 0x67452301U;
    ctx->state[1] = 0xEFCDAB89U;
    ctx->state[2] = 0x98BADCFEU;
    ctx->state[3] = 0x10325476U;
    ctx->state[4] = 0xC3D2E1F0U;
    ctx->count[0] = 0U;
    ctx->count[1] = 0U;
}

static inline void sha1_update(sha1_ctx_t *ctx, const uint8_t *data, size_t len) {
    size_t i, j;

    j = (ctx->count[0] >> 3) & 63U;

    if ((ctx->count[0] += (uint32_t)(len << 3)) < (uint32_t)(len << 3)) {
        ctx->count[1]++;
    }
    ctx->count[1] += (uint32_t)(len >> 29);

    if ((j + len) > 63U) {
        memcpy(&ctx->buffer[j], data, (i = 64U - j));
        sha1_transform(ctx->state, ctx->buffer);

        for (; i + 63U < len; i += 64U) {
            sha1_transform(ctx->state, &data[i]);
        }
        j = 0U;
    } else {
        i = 0U;
    }

    memcpy(&ctx->buffer[j], &data[i], len - i);
}

static inline void sha1_final(sha1_ctx_t *ctx, uint8_t out[20]) {
    uint8_t finalcount[8];
    int i;

    for (i = 0; i < 8; i++) {
        finalcount[i] = (uint8_t)(ctx->count[(i >= 4) ? 0 : 1] >> ((3 - (i & 3)) * 8));
    }

    sha1_update(ctx, (const uint8_t *)"\200", 1);

    while ((ctx->count[0] & 504U) != 448U) {
        sha1_update(ctx, (const uint8_t *)"\0", 1);
    }

    sha1_update(ctx, finalcount, 8);

    for (i = 0; i < 20; i++) {
        out[i] = (uint8_t)(ctx->state[i >> 2] >> ((3 - (i & 3)) * 8));
    }
}

#endif /* SHA1_H */