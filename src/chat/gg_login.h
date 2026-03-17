#ifndef GG_LOGIN_H
#define GG_LOGIN_H

#include "../platform.h"	// for byte padding
#include <stdint.h>

typedef struct {
	uint32_t type;		// for example GG_WELCOME
	uint32_t length;	// sizeof(struct), or 0 if just header
} GG_PACKED GG_header ;

#define GG_WELCOME	0x0001

typedef struct {
	int seed;	// its a 32-bit random number / change to uint32_t
} GG_PACKED GG_welcome;

#define GG_LOGIN3 0x00000001 	// yes, it is GG_WELCOME / todo: reverse engineer the struct for v3
#define GG_LOGIN 0x000c			// libgadu/include/libgadu.h.in @ 1888 - GG_LOGIN struct

// Acording to wireshark, the client sends out 
// a packet of this type, but its 12 bytes in length.
typedef struct {
	uint32_t uin;
	uint32_t hash;
	uint32_t status;
} GG_PACKED GG_login3;

#define GG_LOGIN_FAILED 0x0009
#define GG_LOGIN_OK 0x0003

// ----- functions -----------------

void gg_send_welcome(int sock, uint32_t seed);
uint32_t gg_login_hash_old(const unsigned char *password, uint32_t seed);	// pls get rid of this
void gg_send_login_failed(int sock);
void gg_send_login_ok(int sock);
void handle_logging(int sock);
#endif
