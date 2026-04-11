/*
	Co to jest? - Definicje funkcji PUBDIR (HTTP) dla wersji GG 4.6
	Czy działa? - Nie wiem. w trakcie implementacji
	Istotne? - Niezbyt, taki side-quest
	Badania wysyłanych podań są na podstawie Wireshark :P
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../server.h"
#include "../platform.h"
#include "../../http/http_utils.h"
#include "pubdir.h"

/*
 * Handler Szczegółów Katalogu Publicznego (fmpubdetails2.asp) dla wersji GG 4.6
 * wysłane:
	FmNum - UIN zalogowanego użytkownika
	Pass - hasło użytkownika
 * Odsyłane:
	Response: HTTP 200 [dane wcześniej napisane]
	
 * NOTE: czy to niby to? sprawdzę jak wykonam implementacje.
 */
void h_pubdir_details_46(int sock, char *query){
	char c_uin, clientpass;
	
	get_param(query, "FmNum", c_uin, sizeof(c_uin));
	get_param(query, "Pass", clientpass, sizeof(clientpass));
	
	uint32_t uin = atoi(c_uin);
	
	// jeżeli dane nie poprawne, odeślij błąd i nie przyzwalaj do kat. pub.
	if(!db_verify_password(uin, clientpass)){
		LOG_WARN("PUBDIR: incorrect password - access denied");
		http_send_response(sock, 401, "Unauthorised", "incorrect UIN or password");
	} else {
		// zamiast '200 - ok' wysyłamy dane spowrotem 
		// (patrz na dane rejestracji do pubdir)
		http_send_response(sock, 200, "OK", "200 - OK");
	}
}

/*
 * Handler Rejestrowania do Katalogu Publicznego (fmpubreg2.asp) dla wersji GG 4.6
 * wysłane:
	FmNum - UIN zalogowanego użytkownika
	Pass - hasło użytkownika
	FirstName - Imię
	LastName - nazwisko
	NickName - przezwisko
	Email - poczta elektroniczna
	BirthYear - rok urodzenia
	Gender - Płeć (Kobieta - 1 ; Mężczyzna - 2)
	City - Miejscowość
	Phone - numer telefonu
	
 * Odsyłane:
	Nieeudane HTTP 500
	udane: HTTP 200 OK 
	
 * NOTE: nie jestem pewien czy to to
 */
void h_pubdir_register_46(int sock, char *query, PubDir user){
	char c_uin, clientpass;
	
	get_param(query, "FmNum", c_uin, sizeof(c_uin));
	get_param(query, "Pass", clientpass, sizeof(clientpass));
	
	if(!db_verify_password((uint32_t)atoi(c_uin), clientpass)){
		LOG_WARN("PUBDIR: incorrect password - access denied");
		http_send_response(sock, 401, "Unauthorised", "incorrect UIN or password");
	}
	
	// Piękność kodowa - we Love C Syntax
	
	user.uin = c_uin;
	get_param(query, "FirstName", user.firstname, sizeof(user.firstname));
	get_param(query, "LastName", user.lastname, sizeof(user.firstname));
	get_param(query, "NickName", user.nickname, sizeof(user.nickname));
	get_param(query, "Email", user.email, sizeof(user.email));
	get_param(query, "BirthYear", user.birthyear, sizeof(user.birthyear));
	get_param(query, "Gender", (int)atoi(user.gender), sizeof((int)atoi(user.gender)));
	get_param(query, "City", user.city, sizeof(user.city));
	get_param(query, "Phone", user.phone, sizeof(user.phone));
	
	// TODO: dodaj do osobnej bazy danych (pubdir.dat)
	// TODO: wydaj dane z podania HTTP do struktury, a potem do bazy
	
}