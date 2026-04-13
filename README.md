# OpenGaduServer
'Samowystarczalny' serwer kompatybilny z klientami Gadu-Gadu dla własnego hostowania (self-hosting). Kod źródłowy jest napisany w C i opiera się na dokumentacji takich jak [libgadu](https://github.com/Jakkret/OpenGaduServer/blob/main/docs/SOURCES.md#libgadu-by-wojtekka-and-libgadu-team1-)) (źródła są w [`SOURCES.md`](https://github.com/Jakkret/OpenGaduServer/blob/main/docs/SOURCES.md)). Jak na razie, projekt świetnie działa na systemach Windows i Linux (działaj z przywilejem `sudo`). Serwer skupia się w tej chwili na wspieraniu Klienta 5.x (patrz Lista Wsparcia)

Jak chcesz wydać commit do tego repozytorium - **obczaj CONTRIBUTING.MD** - Więcej rąk sprawi, że ten projekt będzie stabilnie chodził wcześniej.. a czasami lepiej!

> [!IMPORTANT]
> Ten projekt nie jest powiązany ani wspierany przez Gadu Gadu (lub GG). Projekt ten jest całkowicie otwarty, darmowy — i nie powinien być 

------------------------------
### Lista wsparcia wersji GG:

> [!WARNING]
> Ten projekt jest w stanie rozwojowym. Nie oczekuj, że wszystko działa od tak - W tej chwili mimo, że funkcje poniżej są podane, nie znaczy że działają w 100%. Więcej funkcji nadejdzie w późniejszych poprawkach.

W chwili obecnej, OpenGaduServer wspiera większość podstawowych funkcji:
- [ ] Gadu Gadu 3.1: (wymaga serwera SSL na porcie 443, omijane jak narazie)

- [x] Gadu Gadu 5.0:
    - [x] rejestrowanie i przekierowanie do serwera
    - [ ] resetowania hasła *(opcjonalne)*
    - [x] pakiet GG_WELCOME
    - [x] logowanie 
    - [x] statusy
	- [x] funkcje czatu
	- [X] odbieranie listy kontaktów z/do serwera
	
- [x] Gadu Gadu 6.0: _(dla JaduGadu... bo używa logowania GG_LOGIN60)_
    - [ ] rejestrowanie i przekierowanie do serwera *(opcjonalne)*
    - [ ] resetowania hasła *(opcjonalne)*
    - [x] pakiet GG_WELCOME (wraz z 5.0)
    - [x] logowanie
    - [x] statusy
	- [x] funkcje czatu
	- [x] odbieranie listy kontaktów z/do serwera *(implementacja według GG - GG_USERLIST_REQUEST)*
