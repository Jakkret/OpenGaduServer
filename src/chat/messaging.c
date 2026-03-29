#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "messaging.h"
#include "client.h"
#include "../platform.h"
#include "protocol.h"
#include "../server.h"

static void send_ack(client_t *sender, uint32_t seq, uint32_t recipient, uint32_t status){
	gg_send_msg_ack_t ack;
	ack.status = status;
	ack.recipient = recipient;
	ack.seq = seq;
	write_full_packet(sender, GG_SEND_MSG_ACK, &ack, sizeof(ack));
}

// handling gg_send_msg
int msg_handle_send(client_t *c, void *data, uint32_t len) {
	
	const uint8_t *body = (const uint8_t *)data;
	
	if(len < sizeof(gg_send_msg_t) + 1){
		LOG_ERR("MSG: packet too short from UIN %u", c->uin);
		return -1;
	}
	
	const gg_send_msg_t *hdr = (const gg_send_msg_t *)body;
	const char *text = (const char *)(body + sizeof(gg_send_msg_t));
	uint32_t text_len = len - sizeof(gg_send_msg_t);
	
	// sprawdź czy jest null-terminated
	if(memchr(text, '\0', text_len) == NULL){
		LOG_ERR("MSG: Message from UIN %u has no null-terminator", c->uin);
		return -1;
	}
	
	LOG_INFO("MSG: UIN %u -> UIN %u \"%s\"", c->uin, hdr->recipient, text);
	
	client_t *recipient = client_find(hdr->recipient);
	
	if(recipient == NULL || recipient->state != STATE_LOGIN_OK){
		LOG_WARN("MSG: UIN %u is offline. Message is not queued yet", hdr->recipient);
		send_ack(c, hdr->seq, hdr->recipient, GG_ACK_QUEUED);
		return -1;	
	}
	
	// stwórz reszte GG_RECV_MSG: nagłówek + tekst
	uint32_t out_len = sizeof(gg_recv_msg_t) + text_len;
	uint8_t *out = malloc(out_len);
	if(!out){
		LOG_ERR("MSG: malloc failed");
		return -1;
	}
	
	gg_recv_msg_t *recv = (gg_recv_msg_t *)out;
	recv->sender = c->uin;
	recv->seq = (uint32_t)time(NULL);
	recv->time = (uint32_t)time(NULL);
	recv->msgclass = hdr->msgclass;
	memcpy(out + sizeof(gg_recv_msg_t), text, text_len);
	
	write_full_packet(recipient, GG_RECV_MSG, out, out_len);
	free(out);
	
	send_ack(c, hdr->seq, hdr->recipient, GG_ACK_DELIVERED);
	LOG_OK("MSG: Delivered UIN %u -> UIN %u", c->uin, hdr->recipient);
	return 0;
}
