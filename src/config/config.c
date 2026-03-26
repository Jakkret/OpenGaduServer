#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "../server.h"

/*
 * Cuts the string from a specific part.
 *
 * @param str string which is cut
 * @param Cut the character from where it is cut
 * @returns string after the specific character
 * 
 */
const char *cutstr(const char *str, char cut) {
    const char *p = strchr(str, cut);
    if (!p) return NULL;
    return p + 1;
}


int ReadConfig(const char *filename, ServerConf *scHTTP, ServerConf *scCHAT){
    FILE *f = fopen(filename, "r");
    if(!f){
        LOG_ERR("CONFIG: Server could not load config: %s", filename);
        return -1;  // failed
    } 

    char arg[256];
    char symbol = '=';
    while(fgets(arg, sizeof(arg), f)){

        arg[strcspn(arg, "\n")] = '\0';

        if(strncmp(arg, "CHAT_IP=", 8) == 0){
            
            scCHAT->IPaddr = strdup(cutstr(arg, symbol));
            
        } else if(strncmp(arg, "CHAT_PORT=", 10) == 0){
            scCHAT->Port = strdup(cutstr(arg, symbol));

        } else if(strncmp(arg, "HTTP_IP=", 8) == 0){
            scHTTP->IPaddr = strdup(cutstr(arg, symbol));

        } else if(strncmp(arg, "HTTP_PORT=", 10) == 0){
            scHTTP->Port = strdup(cutstr(arg, symbol));

        }
    }

    fclose(f);
    return 0;
}