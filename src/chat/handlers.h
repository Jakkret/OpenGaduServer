#ifndef HANDLERS_H
#define HANDLERS_H

#include "client.h"
#include "protocol.h"

// handler function
typedef int (*handler_fn)(client_t *c, void *data, uint32_t len);

// handler table
typedef struct {
    uint32_t   type;
    handler_fn handler;
} gg_handler_t;

// one big handler!!
void handle_input_packet(client_t *c, void *data, uint32_t type, uint32_t len);

#endif