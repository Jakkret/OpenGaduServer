#ifndef MESSAGING_H
#define MESSAGING_H

#include "client.h"

int msg_handle_send(client_t *c, void *data, uint32_t len);

#endif