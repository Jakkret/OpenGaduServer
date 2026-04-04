# Jak uruchomić OpenGaduServer?
Przed uruchomieniem OpenGaduServer potrzebujesz kilku rzeczy. jednym z nich jest plik konfiguracji `ogs-conf.ini`.<br>
Plik ten mówi programowi, na jakich adresach i na jakich portach ma czuwać. Taki plik powinien mieć takie wartości:
```ini
CHAT_IP=127.0.0.1
CHAT_PORT=8074
HTTP_IP=127.0.0.1
HTTP_PORT=80
```

Jeżeli uruchamiasz klient Gadu-Gadu na jednym komputerze, możesz pozostawić domyślne adresy.<br>
Aby klient Gadu-Gadu mógł połączyć się z OpenGaduServer, musisz przekierować klient do serwera - przez edycje pliku `hosts`.
Pliki te, możesz znaleźć w podanych poniżej lokalizacjach:<br>
-	**Windows**: `C:/Windows/System32/drivers/etc/hosts` (wymagane są prawa administratora)
-	**Linux**: `/etc/hosts` (wymagany jest przywilej `sudo`)
W samym pliku `hosts` dodajesz te linijki:
```
0.0.0.0		appmsg.gadu-gadu.pl
127.0.0.1	appmsg2.gadu-gadu.pl
127.0.0.1	register.gadu-gadu.pl
```
gdzie zamiast `127.0.0.1` wpisujesz adres który podałeś w pliku konfiguracyjnym:
- 	**CHAT_IP** dotyczy `appmsg2.gadu-gadu.pl`
-	**HTTP_IP** dotyczy `register.gadu-gadu.pl`

**Zalecany klient do użycia jest klient Gadu-Gadu wersji 5.0.5 lub 6.0** Linki do pobrania klientów będą na stronie internetowej projektu
