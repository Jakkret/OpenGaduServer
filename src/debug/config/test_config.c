#include "../../config/config.h"
#include <stdio.h>

int main(){
    ServerConf *CHAT, *HTTP;

    ReadConfig("config.ini", HTTP, CHAT);
    printf("SCHTTP: IP - %s \n", CHAT->IPaddr);
    printf("SCHTTP: PORT - %s \n", CHAT->Port);
    return 0;
}