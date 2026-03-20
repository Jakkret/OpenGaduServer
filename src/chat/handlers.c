#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../server.h"
#include "../platform.h"
#include "../database/users.h"
#include "client.h"
#include "protocol.h"
#include "handlers.h"

// Login handler for v5
static int gg_login_handler(client_t *c, void *data, uint32_t len) {
    if (len < sizeof(gg_login5_t)) return -2;
    if (c->state != STATE_LOGIN) {
        LOG_WARN("HANDLER: gg_login_handler() unexpected state %d", c->state);
        return -3;
    }

    gg_login5_t *l = (gg_login5_t*) data;

    LOG_INFO("HANDLER: Login attempt from UIN %u", l->uin);

    if (!authorize(l->uin, c->seed, l->hash)) {
        LOG_WARN("HANDLER: Login FAILED for UIN %u", l->uin);
        write_full_packet(c, GG_LOGIN_FAILED, NULL, 0);
        c->remove = 1;
        return -1;
    }

    // duplikat — rozłącz stare połączenie
    client_t *old = client_find(l->uin);
    if (old) {
        LOG_WARN("HANDLER: Duplicate client UIN %u, removing old", l->uin);
        write_full_packet(old, GG_DISCONNECTING, NULL, 0);
        old->remove = 1;
    }

    c->uin     = l->uin;
    c->state   = STATE_LOGIN_OK;
    c->status  = l->status;
    c->version = l->version;
    c->timeout = time(NULL) + TIMEOUT_DEFAULT;

    LOG_OK("HANDLER: Login OK for UIN %u", c->uin);
    write_full_packet(c, GG_LOGIN_OK, NULL, 0);

    return 0;
}

// First Notify Handler (gg_notify_first)
static int gg_notify_handler(client_t *c, void *data, uint32_t len) {
    if (c->state != STATE_LOGIN_OK) return -3;

    LOG_INFO("HANDLER: Received notify list from UIN %u", c->uin);

    while (len >= sizeof(gg_notify_t)) {
        gg_notify_t *n = (gg_notify_t*) data;
        client_add_friend(c, n->uin, n->type);
        len  -= sizeof(gg_notify_t);
        data += sizeof(gg_notify_t);
    }

    return 0;
}

// Last notify handler (gg_notify_last)
static int gg_notify_end_handler(client_t *c, void *data, uint32_t len) {
    if (gg_notify_handler(c, data, len) != 0) return -1;

    // reply with contact statuses
    for (int i = 0; i < c->friend_count; i++) {
        uint32_t uin = c->friends[i].uin;
        client_t *friend = client_find(uin);

        gg_notify_reply_t reply;
        reply.uin    = uin;
        reply.status = friend ? friend->status : GG_STATUS_NOT_AVAIL;

        write_full_packet(c, GG_NOTIFY_REPLY, &reply, sizeof(reply));
        LOG_INFO("HANDLER: Contact %u -> %s", uin, friend ? "ONLINE" : "OFFLINE");
    }

    return 0;
}

// empty list
static int gg_list_empty_handler(client_t *c, void *data, uint32_t len) {
    if (c->state != STATE_LOGIN_OK) return -3;
    LOG_INFO("HANDLER: UIN %u has empty contact list", c->uin);
    return 0;
}

// show clients the change
void changed_status(client_t *c) {
	client_t **all = client_get_all();
	
    // look through all online clients
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_t *other = all[i];
        if (!other || other == c) continue;
        if (other->state != STATE_LOGIN_OK) continue;

        // check if other has c in its contacts
        if (!client_find_friend(other, c->uin)) continue;

        //send status
        gg_notify_reply_t reply;
        reply.uin    = c->uin;
        reply.status = c->status;
        write_full_packet(other, GG_STATUS, &reply, sizeof(reply));

		LOG_INFO("HANDLER: Sending status packet to UIN %u: uin=%u status=0x%08X size=%zu",
			other->uin, reply.uin, reply.status, sizeof(reply));
    }
}

//	New status
static int gg_new_status_handler(client_t *c, void *data, uint32_t len) {
    if (c->state != STATE_LOGIN_OK) return -3;
    if (len < sizeof(gg_new_status_t)) return -2;

    gg_new_status_t *s = (gg_new_status_t*) data;
    int status = s->status;

    // obsługa trybu tylko dla przyjaciół - ciekawe czy zadziała w GG5
    if (status & GG_STATUS_FRIENDS_MASK) {
        status &= ~GG_STATUS_FRIENDS_MASK;
        c->status_private = 1;
    } else {
        c->status_private = 0;
    }

    c->status = status;

    // opis statusu
    if (c->status_descr) {
        free(c->status_descr);
        c->status_descr = NULL;
    }
    if (len > sizeof(gg_new_status_t)) {
        uint32_t descr_len = len - sizeof(gg_new_status_t);
        c->status_descr = malloc(descr_len + 1);
        memcpy(c->status_descr, data + sizeof(gg_new_status_t), descr_len);
        c->status_descr[descr_len] = 0;
    }

    LOG_INFO("HANDLER: UIN %u status -> 0x%08X descr=\"%s\"",
             c->uin, c->status, c->status_descr ? c->status_descr : "");

    // powiadom kontakty
    // TODO: changed_status(c)
	changed_status(c);

    // rozłącz jeśli niedostępny
    if (c->status == GG_STATUS_NOT_AVAIL || c->status == GG_STATUS_NOT_AVAIL_DESCR) {
        write_full_packet(c, GG_DISCONNECTING, NULL, 0);
        c->remove = 1;
    }

    return 0;
}

// ── Ping handler ──────────────────────────────────────────
static int gg_ping_handler(client_t *c, void *data, uint32_t len) {
    LOG_INFO("HANDLER: Ping from UIN %u", c->uin);
    c->last_ping = time(NULL);
    c->timeout   = time(NULL) + TIMEOUT_DEFAULT;
    return 0;
}

// handler table instead of big switch statement
static const gg_handler_t gg_handlers[] = {
    { GG_LOGIN5,        gg_login_handler       },
    { GG_NOTIFY_FIRST,  gg_notify_handler      },
    { GG_NOTIFY_LAST,   gg_notify_end_handler  },
    { GG_LIST_EMPTY,    gg_list_empty_handler  },
    { GG_NEW_STATUS,    gg_new_status_handler  },
    { GG_PING,          gg_ping_handler        },
    { 0, NULL }
};

// handles packets off the table and directs them
void handle_input_packet(client_t *c, void *data, uint32_t type, uint32_t len) {
    if (c->remove) return;

    LOG_INFO("HANDLER: UIN %u sent packet 0x%08X length=%u", c->uin, type, len);

    for (int i = 0; gg_handlers[i].type; i++) {
        if (gg_handlers[i].type == type) {
            gg_handlers[i].handler(c, data, len);
            return;
        }
    }

    LOG_WARN("HANDLER: Unknown packet 0x%08X from UIN %u", type, c->uin);
}