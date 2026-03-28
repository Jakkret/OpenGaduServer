#ifndef CONFIG_H
#define CONFIG_H

#include "../server.h"

int ReadConfig(const char *filename, ServerConf *scCHAT, ServerConf *scHTTP);

#endif