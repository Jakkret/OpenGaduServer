#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../server.h"
#include "../platform.h"
#include "../database/users.h"
#include "client.h"
#include "protocol.h"
#include "messaging.h"
#include "handlers.h"


// buduje i wysyła GG_NOTIFY_REPLY z opcjonalnym opisem statusu (zmienna długość)
static void send_status_with_descr(client_t *to, client_t *from, uint32_t uin, uint32_t fallback_status, uint32_t packet_type) {
    uint32_t status;
    char    *descr_copy = NULL;
    uint32_t descr_len  = 0;

    if (from) {
        mutex_lock(&from->lock);
        status = from->status;
        if (from->status_descr) {
            descr_len = (uint32_t)strlen(from->status_descr);
            descr_copy = malloc(descr_len + 1);
            if (descr_copy) {
                memcpy(descr_copy, from->status_descr, descr_len + 1);
            } else {
                LOG_ERR("HANDLER: malloc failed copying status_descr");
                descr_len = 0;
            }
        }
        mutex_unlock(&from->lock);
    } else {
        status = fallback_status;
    }

    uint8_t  *buf;
    uint32_t  total_len;

    if (descr_len == 0) {
        gg_notify_reply_t reply;
        reply.uin    = uin;
        reply.status = status;
        write_full_packet(to, packet_type, &reply, sizeof(reply));
        if (descr_copy) free(descr_copy);
        return;
    }

    total_len = sizeof(gg_notify_reply_descr_t) + descr_len;
    buf = malloc(total_len);
    if (!buf) {
        LOG_ERR("HANDLER: malloc failed in send_status_with_descr");
        free(descr_copy);
        return;
    }

    gg_notify_reply_descr_t *reply = (gg_notify_reply_descr_t*)buf;
    reply->uin         = uin;
    reply->status      = status;
    reply->remote_ip   = from ? from->addr.sin_addr.s_addr : 0;
    reply->remote_port = from ? from->addr.sin_port        : 0;
    reply->version     = from ? from->version               : 0;
    reply->image_size  = from ? (uint8_t)from->image_size   : 0;
    reply->unknown1    = 0;

    memcpy(buf + sizeof(gg_notify_reply_descr_t), descr_copy, descr_len);

    write_full_packet(to, packet_type, buf, total_len);

    free(buf);
    free(descr_copy);
}

// show clients the change
void changed_status(client_t *c) {
    client_t **all = client_get_all();

    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_t *other = all[i];
        if (!other || other == c) continue;
        if (other->state != STATE_LOGIN_OK) continue;
        if (!client_find_friend(other, c->uin)) continue;

        // changed_status() — LIVE update, zawsze GG_NEW_STATUS
        send_status_with_descr(other, c, c->uin, 0, GG_NEW_STATUS);

        LOG_INFO("HANDLER: Sending status packet to UIN %u: uin=%u status=0x%08X descr=\"%s\"",
            other->uin, c->uin, c->status, c->status_descr ? c->status_descr : "");
    }
}



// Login handler for v5
static int gg_login31_handler(client_t *c, void *data, uint32_t len) {
    if (len < sizeof(gg_login3_t)) return -2;
    if (c->state != STATE_LOGIN) {
        LOG_WARN("HANDLER: gg_login_handler() unexpected state %d", c->state);
        return -3;
    }

    gg_login3_t *l = (gg_login3_t*) data;

    LOG_INFO("HANDLER: Login (3.1) attempt from UIN %u", l->uin);

    if (!authorize(l->uin, c->seed, l->hash)) {
        LOG_WARN("HANDLER: Login (3.1) FAILED for UIN %u", l->uin);
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
    c->status  = GG_STATUS_AVAIL;	// od razu pokazuj dostępność po zalogowaniu
    c->timeout = time(NULL) + TIMEOUT_DEFAULT;

    LOG_OK("HANDLER: Login OK for UIN %u", c->uin);
    write_full_packet(c, GG_LOGIN_OK, NULL, 0);
	changed_status(c);

    return 0;
}

// Login handler for v5
static int gg_login50_handler(client_t *c, void *data, uint32_t len) {
    if (len < sizeof(gg_login5_t)) return -2;
    if (c->state != STATE_LOGIN) {
        LOG_WARN("HANDLER: gg_login_handler() unexpected state %d", c->state);
        return -3;
    }

    gg_login5_t *l = (gg_login5_t*) data;

    LOG_INFO("HANDLER: Login50 attempt from UIN %u", l->uin);

    if (!authorize(l->uin, c->seed, l->hash)) {
        LOG_WARN("HANDLER: Login50 FAILED for UIN %u", l->uin);
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
    c->status  = GG_STATUS_AVAIL;	// od razu pokazuj dostępność po zalogowaniu
    c->version = l->version;
    c->timeout = time(NULL) + TIMEOUT_DEFAULT;

    LOG_OK("HANDLER: Login50 OK for UIN %u", c->uin);
    write_full_packet(c, GG_LOGIN_OK, NULL, 0);
	changed_status(c);

    return 0;
}

// Logowanie dla wersji 6.x
static int gg_login60_handler(client_t *c, void *data, uint32_t len) {
    if (len < sizeof(gg_login60_t)) return -2;
    if (c->state != STATE_LOGIN) {
        LOG_WARN("HANDLER: gg_login60_handler() unexpected state %d", c->state);
        return -3;
    }

    gg_login60_t *l = (gg_login60_t *)data;

    LOG_INFO("HANDLER: Login60 attempt from UIN %u (version=0x%08X)", l->uin, l->version);

    if (!authorize(l->uin, c->seed, l->hash)) {
        LOG_WARN("HANDLER: Login60 FAILED for UIN %u", l->uin);
        write_full_packet(c, GG_LOGIN_FAILED, NULL, 0);
        c->remove = 1;
        return -1;
    }

    client_t *old = client_find(l->uin);
    if (old) {
        LOG_WARN("HANDLER: Duplicate client UIN %u, removing old", l->uin);
        write_full_packet(old, GG_DISCONNECTING, NULL, 0);
        old->remove = 1;
    }

    c->uin        = l->uin;
    c->state      = STATE_LOGIN_OK;
    c->status     = GG_STATUS_AVAIL;
    c->version    = l->version;
    c->image_size = l->image_size;
    c->timeout    = time(NULL) + TIMEOUT_DEFAULT;

    LOG_OK("HANDLER: Login60 OK for UIN %u", c->uin);
    write_full_packet(c, GG_LOGIN_OK, NULL, 0);
    changed_status(c);

    return 0;
}

// Logowanie dla wersji 7.x
static int gg_login70_handler(client_t *c, void *data, uint32_t len) {
    if (len < sizeof(gg_login70_t)) return -2;
    if (c->state != STATE_LOGIN) {
        LOG_WARN("HANDLER: gg_login70_handler() unexpected state %d", c->state);
        return -3;
    }

    gg_login70_t *l = (gg_login70_t *)data;

    LOG_INFO("HANDLER: Login70 attempt from UIN %u (version=0x%08X)", l->uin, l->version);

	// wybierz typ hash
	switch(l->hash_type){
		// prestarzały hash GG32 
		case GG_LOGIN_HASH_GG32: {
			LOG_INFO("HANDLER: UIN %u requested GG32 hashing", l->uin);
			
			if(!authorize(l->uin, c->seed, (uint32_t)l->hash)){
				LOG_WARN("HANDLER: Login70(GG32) FAILED for UIN %u", l->uin);
				write_full_packet(c, GG_LOGIN_FAILED, NULL, 0);
				c->remove = 1;
				return -1;
			}
			break;
		}
		// sha1
		case GG_LOGIN_HASH_SHA1: {
			LOG_INFO("HANDLER: UIN %u requested SHA1 hashing", l->uin);
			
			if(!authorize70(l->uin, c->seed, l->hash)){
				LOG_WARN("HANDLER: Login70(SHA1) FAILED for UIN %u", l->uin);
				write_full_packet(c, GG_LOGIN_FAILED, NULL, 0);
				c->remove = 1;
				return -1;
			}
			break;
		}
		default:
			LOG_WARN("HANDLER: Unknown hash_type: 0x%02X", l->hash_type);
			return -1;
	}

	// wsparcie dla multilogowania dopiero w GG 8.0 ... daleko.
    client_t *old = client_find(l->uin);
    if (old) {
        LOG_WARN("HANDLER: Duplicate client UIN %u, removing old", l->uin);
        write_full_packet(old, GG_DISCONNECTING, NULL, 0);
        old->remove = 1;
    }

    c->uin        = l->uin;
    c->state      = STATE_LOGIN_OK;
    c->status     = GG_STATUS_AVAIL;
    c->version    = l->version;
    c->image_size = l->image_size;
    c->timeout    = time(NULL) + TIMEOUT_DEFAULT;

    LOG_OK("HANDLER: Login70 OK for UIN %u", c->uin);
    write_full_packet(c, GG_LOGIN_OK, NULL, 0);
    changed_status(c);

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

        send_status_with_descr(c, friend, uin, GG_STATUS_NOT_AVAIL, GG_NOTIFY_REPLY);

        LOG_INFO("HANDLER: Contact %u -> %s", uin, friend ? "ONLINE" : "OFFLINE");
    }


    // dalej ma opóźnienie komputer z 10/100 nic'iem... dziwne
    changed_status(c);
	
	// FIX - 22.03.2026: klienci późno dostają statusy swoich kontaktów
	// informuj zalogowanych użytkowników że jesteś online

    // NOTE - 29.08.2026: To chyba działa... ale jest zbędne bo działa implementacja na górze.
    
	for (int i = 0; i < c->friend_count; i++) {
		client_t *friend = client_find(c->friends[i].uin);
		if (!friend) continue;
		
		// wyślij temu kontaktowi twój status z opisem
		send_status_with_descr(friend, c, c->uin, 0, GG_NEW_STATUS);
	}
	changed_status(c);

    return 0;
}

// empty list
static int gg_list_empty_handler(client_t *c, void *data, uint32_t len) {
    if (c->state != STATE_LOGIN_OK) return -3;
    LOG_INFO("HANDLER: UIN %u has empty contact list", c->uin);
    return 0;
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

    mutex_lock(&c->lock);

    c->status = status;

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
    mutex_unlock(&c->lock);
    

    LOG_INFO("HANDLER: UIN %u status -> 0x%08X descr=\"%s\"",
             c->uin, c->status, c->status_descr ? c->status_descr : "");

    // spróbuj wysłać status do kontaktów
    if(c->status_descr && c->status_descr[0] != 0){
        LOG_INFO("HANDLER: UIN %u has status description, sending to contacts", c->uin);
        client_t **all = client_get_all();
        for (int i = 0; i < MAX_CLIENTS; i++) {
            client_t *other = all[i];
            if (!other || other == c) continue;
            if (other->state != STATE_LOGIN_OK) continue;

            // check if other has c in its contacts
            if (!client_find_friend(other, c->uin)) continue;

            // send status description
            gg_notify_reply_t reply;
            reply.uin    = c->uin;
            reply.status = c->status;
            write_full_packet(other, GG_NEW_STATUS, &reply, sizeof(reply));

            LOG_INFO("HANDLER: Sent status to UIN %u", other->uin);
        }
        
    }

    // powiadom kontakty
	changed_status(c);

    // rozłącz jeśli niedostępny
    if (c->status == GG_STATUS_NOT_AVAIL || c->status == GG_STATUS_NOT_AVAIL_DESCR) {
        write_full_packet(c, GG_DISCONNECTING, NULL, 0);
        c->remove = 1;
    }

    return 0;
}

// mam nadzieję, że to naprawi brak odświeżania po zalogowaniu
// - Jakkret, 22.03.2026
static int gg_notify_add_handler(client_t *c, void *data, uint32_t len){
	if(c->state != STATE_LOGIN_OK) return -3;
	if(len < sizeof(gg_add_remove_t)) return -2;
	
	gg_add_remove_t *ar = (gg_add_remove_t*) data;	
	if(ar->uin == 0) return 1;
	
	client_add_friend(c, ar->uin, ar->type);
	
	// odeślij status dodanego kontaktu
	client_t *friend = client_find(ar->uin);
	gg_notify_reply_t reply;
	reply.uin = ar->uin;											// numer dodanego kontaktu
	reply.status = friend ? friend->status : GG_STATUS_NOT_AVAIL;	// status kontaktu
	write_full_packet(c, GG_NOTIFY_REPLY, &reply, sizeof(reply));
}

static int gg_notify_remove_handler(client_t *c, void *data, uint32_t len){
	if(c->state != STATE_LOGIN_OK) return -3;
	if(len < sizeof(gg_add_remove_t)) return -2;
	
	gg_add_remove_t *ar = (gg_add_remove_t*) data;
	client_remove_friend(c, ar->uin);
	
	LOG_INFO("HANDLER: UIN %u removed %u", c->uin, ar->uin);
	return 0;
}



// ── Ping handler ──────────────────────────────────────────
static int gg_ping_handler(client_t *c, void *data, uint32_t len) {
    LOG_INFO("HANDLER: Ping from UIN %u", c->uin);
    c->last_ping = time(NULL);
    c->timeout   = time(NULL) + TIMEOUT_DEFAULT;
    return 0;
}

// gg_userlist_request dla 6.x, ignorowane
static int gg_userlist_request_handler(client_t *c, void *data, uint32_t len) {
    if (c->state != STATE_LOGIN_OK) return -3;
    if (len < sizeof(gg_userlist_request_t)) return -2;

    gg_userlist_request_t *r = (gg_userlist_request_t *)data;

    if (r->type == GG_USERLIST_GET) {
        LOG_INFO("HANDLER: UIN %u requests userlist", c->uin);
		
        // odsyłamy pustą listę
        uint8_t reply_type = GG_USERLIST_GET_REPLY;
		write_full_packet(c, GG_USERLIST_REPLY, &reply_type, sizeof(reply_type));
		
    } else if (r->type == GG_USERLIST_PUT || r->type == GG_USERLIST_PUT_MORE) {
		
		// klient wysyła listę - to się kiedyś obsłuży
		uint8_t reply_type = GG_USERLIST_PUT_REPLY;
		write_full_packet(c, GG_USERLIST_REPLY, &reply_type, sizeof(reply_type));
	} else {
        LOG_WARN("HANDLER: UIN %u unknown userlist type 0x%02X", c->uin, r->type);
    }

    return 0;
}

// handler table instead of big switch statement
static const gg_handler_t gg_handlers[] = {
    { GG_LOGIN31,       gg_login31_handler		 },
    { GG_LOGIN50,       gg_login50_handler		 },
	{ GG_LOGIN60,		gg_login60_handler		 },
	{ GG_LOGIN70,		gg_login70_handler		 },
    { GG_NOTIFY_FIRST,  gg_notify_handler		 },
    { GG_NOTIFY_LAST,   gg_notify_end_handler	 },
	{ GG_USERLIST_REQUEST, gg_userlist_request_handler },
    { GG_LIST_EMPTY,    gg_list_empty_handler	 },
    { GG_NEW_STATUS,    gg_new_status_handler	 },
	{ GG_ADD_NOTIFY,	gg_notify_add_handler	 },
	{ GG_REMOVE_NOTIFY, gg_notify_remove_handler },
    { GG_PING,          gg_ping_handler        	 },
	{ GG_SEND_MSG,		msg_handle_send		 },
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