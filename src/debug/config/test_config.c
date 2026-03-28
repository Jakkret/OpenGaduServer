#include "../../config/config.h"
#include <stdio.h>
#include "../../../server.h"
// very long directories, haha - 28.03.2026

int main(){
    ServerConf CHAT, HTTP;

    ReadConfig("config.ini", CHAT, HTTP);
    printf("SCHTTP: IP - %s \n", CHAT.IPaddr);
    printf("SCHTTP: PORT - %s \n", CHAT.Port);
    return 0;
}