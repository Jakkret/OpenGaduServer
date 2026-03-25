#include <stdio.h>
#include <string.h>
#include "config.h"

// Kod nie jest testowany, nie implementuj funkcji bez testowania.
// Napisano 25.03.2026 - Jakkret

ServerConf SCCHAT, SCHTTP;

/*
 * Cuts the string from a specific part.
 *
 * @param: string which is cut
 * @param: the character from where it is cut
 * @returns: string after the specific character
 * 
 * Note: code based off StackOverflow.
 */
static char cutstr(char str, const char Cut){
    char *p = str, token[256];

    if((p = strchr(str, Cut))){
        size_t len = strlen(++p);
        if(len > 256 - 1){
            fputs("error: string exceeds allowable length.\n", stderr);
        }
        memcpy(token, p, len + 1);
    }
    
    return token;
}


int ReadConfig(const char *filename){
    FILE *f = fopen(filename, "r");
    if(!f){
        LOG_ERR("CONFIG: Server could not load config: %s", filename);
        return -1;  // failed
    } 

    char arg[256],
        *p = arg;
    while(fgets(arg, sizeof(arg), filename)){
        if(strncmp(arg, "CHAT_IP=", 9) == 0){
            
            SCCHAT.IPaddr = cutstr(arg, "=");
            // return 0;
            
        } else if(strncmp(arg, "CHAT_PORT=", 11) == 0){
            SCCHAT.Port = cutstr(arg, "=");

        } else if(strncmp(arg, "HTTP_IP=", 9) == 0){
            SCHTTP.IPaddr = cutstr(arg, "=");

        } else if(strncmp(arg, "HTTP_PORT=", 11) == 0){
            SCHTTP.Port = cutstr(arg, "=");

        }
    }

    fclose(filename);
}