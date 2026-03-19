# OpenGaduServer
> [!WARNING]
> Ten projekt jest w stanie rozwojowym.

'Samowystarczalny' serwer kompatybilny z klientami GaduGadu z lat 2000-cznych dla własnego hostowania (self-hosting). Kod źródłowy jest napisany w C i opiera się na dokumentacji takich jak [libgadu](https://github.com/Jakkret/OpenGaduServer/blob/main/docs/SOURCES.md#libgadu-by-wojtekka-and-libgadu-team1-)) (źródła są w [`SOURCES.md`](https://github.com/Jakkret/OpenGaduServer/blob/main/docs/SOURCES.md)). Jak na razie, projekt świetnie działa na systemach Windows (linux już nie ... ale kiedyś będzie!). Serwer skupia się w tej chwili na wspieraniu Klienta 5.x (patrz Lista Wsparcia)

Jak chcesz wydać commit do tego repozytorium - poprostu wypuść commit i sprawdzę zmiany. Więcej rąk sprawi, że ten projekt będzie stabilnie chodził wcześniej

> [!IMPORTANT]
> Ten projekt nie jest powiązany, wspierany ani stowarzyszony z Gadu Gadu (lub GG). Projekt ten jest całkowicie otwarty, darmowy i niekomercyjny - nie przyjmuje z niego żadnych dochodów.

------------------------------
### Lista wsparcia wersji GG:
W chwili obecnej, OpenGaduServer wspiera większość podstawowych funkcji:
- [ ] Gadu Gadu 3.1: (opcjonalnie)
    - [ ] rejestrowanie i przekierowanie do serwera
    - [ ] resetowania hasła
    - [ ] pakiet GG_WELCOME
    - [ ] logowanie 
    - [ ] statusy
	- [ ] funkcje czatu

- [x] Gadu Gadu 5.0:
    - [x] rejestrowanie i przekierowanie do serwera
    - [ ] resetowania hasła
    - [x] pakiet GG_WELCOME
    - [x] logowanie 
    - [ ] statusy _(trwa wdrażanie...)_
	- [ ] funkcje czatu
	- [ ] odbieranie listy kontaktów z/do serwera
