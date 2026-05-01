#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "../hash/md5.h"
#include "http_utils.h"
#include "../config/config.h"
#include "../server.h"

static void gen_tokenid(char *out, size_t out_len){
	uint8_t hash[16]; char input[64];
	
	snprintf(input, sizeof(input), "%ld%d", (long)time(NULL), rand());
}

/* Co robi: Wysyła token do klienta w celu rejestracji (GG 6.0)
 *	Wymagane: brak
	prośba: appsvc/regtoken.asp
 */

void handle_regtoken(int sock){
	char body[256]; char token[33];
	
	gen_tokenid(token, sizeof(token));
	
	ServerConf sHTTP, sCHAT;
	ReadConfig(CONFIG_FILENAME, &sCHAT, &sHTTP);
	
	// odpowiedź z namiarami
	snprintf(body, sizeof(body),
		"1 1 6\r\n"					// rozmiary "obrazka"
		"%s\r\n"					// hash (MD5)
		"http://%s/pcapt.gif\r\n",  // nieistotne - namiary na gif
		token, sHTTP.IPaddr);
	
	// I wysyłamy
	http_send_response(sock, 200, "OK", body);
}

void validate_token(int query, int sock){
	
}