#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    const char *IPaddr;
    const char *Port;
} ServerConf;

int ReadConfig(const char *filename, ServerConf *scHTTP, ServerConf *scCHAT);

#endif