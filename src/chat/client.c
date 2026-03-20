#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../server.h"
#include "../platform.h"
#include "client.h"
#include "protocol.h"



static client_t *clients[MAX_CLIENTS];
static int client_count = 0;

void clients_init(){
	memset(clients, 0, sizeof(clients));
	client_count = 0;
}

client_t** client_get_all() {
    return clients;
}

// create new session
client_t* client_create(int fd, struct sockaddr_in addr, uint32_t seed){
	if(client_count >= MAX_CLIENTS){
		LOG_WARN("CLIENT: max clients reached!");
		return NULL;
	}
	
	client_t *c = calloc(1, sizeof(client_t));
	if(!c){
		LOG_ERR("CLIENT: calloc failed");
		return NULL;
	}
	
	c->fd           = fd;
	c->addr         = addr;
	c->seed         = seed;
	c->state        = STATE_LOGIN;
	c->timeout      = time(NULL) + TIMEOUT_DEFAULT;
	c->last_ping    = time(NULL);
	c->status       = 0;
	c->status_descr = NULL;
	c->remove       = 0;
	c->friends      = NULL;
	c->friend_count    = 0;
	c->friend_capacity = 0;
	
	// add to the table
	for(int i = 0; i < MAX_CLIENTS; i++){
		if(!clients[i]){
			clients[i] = c;
			client_count++;
			break;
		}
	}
	
	LOG_INFO("CLIENT: Created session for fd=%d", fd);
	return c;
}

void client_destroy(client_t *c){
	if (!c) return;

    // usuń z tablicy
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == c) {
            clients[i] = NULL;
            client_count--;
            break;
        }
    }

    // zwolnij pamięć
    if (c->status_descr) free(c->status_descr);
    if (c->friends)      free(c->friends);

    LOG_INFO("CLIENT: Destroyed session for UIN %u", c->uin);
    free(c);
}

client_t* client_find(uint32_t uin) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] && clients[i]->uin == uin)
            return clients[i];
    }
    return NULL;
}


void client_add_friend(client_t *c, uint32_t uin, uint8_t flags) {
    //
    if (c->friend_count >= c->friend_capacity) {
        int new_cap = c->friend_capacity == 0 ? 16 : c->friend_capacity * 2;
        friend_t *new_friends = realloc(c->friends, new_cap * sizeof(friend_t));
        if (!new_friends) {
            LOG_ERR("CLIENT: realloc failed in client_add_friend");
            return;
        }
        c->friends = new_friends;
        c->friend_capacity = new_cap;
    }

    c->friends[c->friend_count].uin   = uin;
    c->friends[c->friend_count].flags = flags;
    c->friend_count++;
}

void client_remove_friend(client_t *c, uint32_t uin) {
    for (int i = 0; i < c->friend_count; i++) {
        if (c->friends[i].uin == uin) {
            // 
            memmove(&c->friends[i], &c->friends[i+1],
                (c->friend_count - i - 1) * sizeof(friend_t));
            c->friend_count--;
            return;
        }
    }
}

friend_t* client_find_friend(client_t *c, uint32_t uin) {
    for (int i = 0; i < c->friend_count; i++) {
        if (c->friends[i].uin == uin)
            return &c->friends[i];
    }
    return NULL;
}

void write_client(client_t *c, const void *data, int len) {
    if (!c || c->fd < 0) return;
    send(c->fd, data, len, 0);
}

void write_full_packet(client_t *c, uint32_t type, const void *data, uint32_t len) {
    struct {
        uint32_t type;
        uint32_t length;
    } header;

    header.type   = type;
    header.length = len;

    write_client(c, &header, sizeof(header));
    if (data && len > 0)
        write_client(c, data, len);
}