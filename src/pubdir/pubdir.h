#ifndef PUBDIR_H
#define PUBDIR_H

// katalog publiczny
typedef struct {
	uint32_t uin;			// numer GG
	char firstname[64];		// imię
	char lastname[64];		// nazwisko
	char nickname[64];		// przezwisko
	char email[128];		// email
	char birthyear[4];		// rok
	int gender;			// Płeć
	char city[64];			// Miejscowość
	char phone[16];			// numer telefonu (wraz z nr kier.)
} PubDir;

// stałe ... tak protokół oczekuje, nie ja :shrug:
#define PUBDIR_GENDER_WOMAN 	1
#define PUBDIR_GENDER_MAN 		2

void h_pubdir_details_46(int sock, char *query);
void h_pubdir_register_46(int sock, char *query, PubDir user);

#endif