#ifndef CONFIG_H
#define CONFIG_H

typedef struct{
    int IPaddr;
    int Port;
} ServerConf;

ServerConf SCCHAT, SCHTTP;

int ReadConfig(const char *filename);

#endif CONFIG_H