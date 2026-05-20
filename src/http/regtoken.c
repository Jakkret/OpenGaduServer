#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "../hash/md5.h"
#include "http_utils.h"
#include "../config/config.h"
#include "../server.h"

// Poprawianie generacji tokenu, zamiast śmieci
static void gen_tokenid(char *out, size_t out_len) {
    static const char hex[] = "0123456789abcdef";
    char input[64];
    snprintf(input, sizeof(input), "%ld%d", (long)time(NULL), rand());

    MD5Context ctx;
    md5Init(&ctx);
    md5Update(&ctx, (uint8_t*)input, strlen(input));
    md5Finalize(&ctx);

    for (int i = 0; i < 16; i++) {
        out[i*2]   = hex[ctx.digest[i] >> 4];
        out[i*2+1] = hex[ctx.digest[i] & 0x0f];
    }
    out[32] = '\0';
}

/* Co robi: Wysyła token do klienta w celu rejestracji (GG 6.0)
 *	Wymagane: brak
	prośba: appsvc/regtoken.asp
 */

void handle_regtoken(int sock){
	char body[256]; char token[33];
	
	gen_tokenid(token, sizeof(token));
	
	// odpowiedź z namiarami
	snprintf(body, sizeof(body),
		"1 1 6\r\n"					// rozmiary "obrazka"
		"%s\r\n"					// hash (MD5)
		"http://%s/pcapt.gif\r\n",  // nieistotne - namiary na gif
		token, sHTTP.IPaddr);
	
	// I wysyłamy
	http_send_response(sock, 200, "OK", body);
}


/* Co robi: Walidacja tokenu czy pasuje
 *	Wymagana: Definicja stałej odpowiedzi do gifu (TOKEN_ANS)
	prośba: GET /pcapt.gif
 */

// zbędne ale tak chce 
#define TOKEN_PASSED 1
#define TOKEN_NOT_PASSED 0


// omija sprawdzanie czy jest wpisany TOKEN_ANS jako odp, niech będzie
int validate_token(char *query, int sock){
	if(strncmp(query, "/pcapt.gif?tokenid=", 20) == 0){
		return TOKEN_PASSED;
		http_send_response(sock, 200, "OK", "success");
	} else {
		return TOKEN_NOT_PASSED;
		http_send_response(sock, 498, "invalid token", "cool... but how did you even get this error?");
	}
}