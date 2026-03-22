#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include "../platform.h"

// Packet definitions
#define GG_WELCOME          		 0x0001
#define GG_LOGIN3           		 0x0001  // GG 3.x
#define GG_LOGIN5           		 0x000c  // GG 5.x
#define GG_LOGIN_OK         		 0x0003
#define GG_LOGIN_FAILED     		 0x0009
#define GG_DISCONNECTING    		 0x000d


// contact list
#define GG_NOTIFY_FIRST     		 0x000f
#define GG_NOTIFY_LAST      		 0x0010
#define GG_LIST_EMPTY				 0x0012
#define GG_NOTIFY_REPLY				 0x000c
#define GG_ADD_NOTIFY       		 0x000d
#define GG_REMOVE_NOTIFY    		 0x000e

// types (for gg_notify_t -> int type)
#define GG_USER_BUDDY			0x0001
#define GG_USER_FRIEND			0x0002

	// user state (gg_notify_t -> int type)
#define GG_USER_NORMAL			0x0003
#define GG_USER_BLOCKED			0x0004

// status changes
#define GG_NEW_STATUS       		 0x0002
#define GG_STATUS77         		 0x0006
#define GG_STATUS50					 0x0002	

#define GG_SEND_MSG         		 0x000b
#define GG_RECV_MSG         		 0x000a
#define GG_SEND_MSG_ACK     		 0x0005

#define GG_PING             		 0x0008

// Client status
#define GG_STATUS_NOT_AVAIL      	 0x0001	// "niedostępny"
#define GG_STATUS_AVAIL          	 0x0002	// "dostępny"
#define GG_STATUS_BUSY          	 0x0003	// "Zaraz Wracam" / "nie przeszkadzaj"
#define GG_STATUS_INVISIBLE     	 0x0014 // "niewidoczny"
#define GG_STATUS_NOT_AVAIL_DESCR 	 0x0015
#define GG_STATUS_AVAIL_DESCR    	 0x0004
#define GG_STATUS_BUSY_DESCR     	 0x0005
#define GG_STATUS_FRIENDS_MASK   	 0x8000
#define GG_STATUS_VOICE_MASK     	 0x0200

// Message classes
#define GG_CLASS_MSG        		 0x0004
#define GG_CLASS_CHAT       		 0x0008
#define GG_CLASS_ACK        		 0x0020
#define GG_CLASS_QUEUED    	 		 0x0001

// ACK TYPES
#define GG_ACK_DELIVERED    		 0x0002
#define GG_ACK_QUEUED       		 0x0003

// Packets
typedef struct {
    uint32_t type;
    uint32_t length;
} GG_PACKED gg_header_t;

typedef struct {
    uint32_t seed;
} GG_PACKED gg_welcome_t;

typedef struct {
    uint32_t uin;
    uint32_t hash;
    uint32_t status;
    uint32_t version;
    uint32_t local_ip;
    uint16_t local_port;
} GG_PACKED gg_login5_t;

typedef struct {
    uint32_t uin;
    uint8_t  type;
} GG_PACKED gg_notify_t;

typedef struct {
    uint32_t uin;
    uint32_t status;
} GG_PACKED gg_notify_reply_t;

typedef struct {
    uint32_t status;
} GG_PACKED gg_new_status_t;

typedef struct {
    uint32_t uin;
    uint32_t status;
} GG_PACKED gg_status_t;

typedef struct {
    uint32_t recipient;
    uint32_t seq;
    uint32_t msgclass;
} GG_PACKED gg_send_msg_t;

typedef struct {
    uint32_t sender;
    uint32_t seq;
    uint32_t time;
    uint32_t msgclass;
} GG_PACKED gg_recv_msg_t;

typedef struct {
    uint32_t status;
    uint32_t recipient;
    uint32_t seq;
} GG_PACKED gg_send_msg_ack_t;

typedef struct {
    uint32_t uin;
    uint8_t  type;
} GG_PACKED gg_add_remove_t;

uint32_t generate_seed();
unsigned int gg_login_hash(const unsigned char *password, uint32_t seed);
int authorize(uint32_t uin, uint32_t seed, uint32_t hash);

#endif