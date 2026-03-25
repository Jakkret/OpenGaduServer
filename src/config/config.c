#include <stdio.h>

int ReadConfig(const char *filename){
    FILE *f = fopen(filename, "r");
    if(!f){
        LOG_ERR("CONFIG: Server could not load config: %s", filename);
        return -1;  // failed
    } 

    char arg[256];
    while(fgets(arg, sizeof(arg), filename)){
        if(strncmp(arg, "CHAT_IP=", 9) == 0){
            return 0;
            // todo: napisz czytanie linii z configu
        } else if(strncmp(arg, "CHAT_PORT=", 11) == 0){
            return 0; // POWTÓRKA

        } else if(strncmp(arg, "HTTP_IP=", 9) == 0){
            return 0; // POWTÓRKA

        } else if(strncmp(arg, "HTTP_PORT=", 11) == 0){
            return 0; // POWTÓRKA

        }
    }

    fclose(filename);
}