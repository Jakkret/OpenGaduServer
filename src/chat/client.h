#ifndef CLIENT_H
#define CLIENT_H

#include <stdint.h>
#include <time.h>
#include "../platform.h"

// client state
#define STATE_LOGIN     0   // waiting for GG_LOGIN
#define STATE_LOGIN_OK  1   // currently logged in

#define TIMEOUT_DEFAULT 180  // seconds

#define MAX_CLIENTS 1024

// ruff contact list
typedef struct {
	uint32_t uin;
	uint8_t  flags;
} friend_t;

// client session
typedef struct client_t {
	int 	fd;                    // socket
	struct 	sockaddr_in addr;        // client address
	
	uint32_t uin;                   // logged in UIN
	uint32_t seed;                  // welcome seed
	uint32_t status;                // current status
	char	*status_descr;         // status description (malloc'd)
	int		status_private;        // friends-only mode

	uint32_t version;               // client version
	uint32_t image_size;            // max image size
	
	int		state;                 // STATE_LOGIN / STATE_LOGIN_OK
    int		remove;                // 1 = disconnect this client
	
	time_t	timeout;               // disconnect if time() > timeout
	time_t	last_ping;             // last ping time
	
	friend_t *friends;              // contact list (dynamic array)
	int      friend_count;
	int      friend_capacity;

    // function pointers (like USG)
	void (*status_write)(struct client_t *to, struct client_t *who);
	void (*notify_reply)(struct client_t *c, uint32_t uin);
	void (*msg_send)(struct client_t *c, void *msg);
} client_t;

// functions

client_t* client_create(int fd, struct sockaddr_in addr, uint32_t seed);
client_t* client_find(uint32_t uin);
friend_t* client_find_friend(client_t *c, uint32_t uin);
client_t** client_get_all();

void clients_init();
void client_destroy(client_t *c);
void client_add_friend(client_t *c, uint32_t uin, uint8_t flags);
void client_remove_friend(client_t *c, uint32_t uin);
void write_client(client_t *c, const void *data, int len);
void write_full_packet(client_t *c, uint32_t type, const void *data, uint32_t len);

#endif