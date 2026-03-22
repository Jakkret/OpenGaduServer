# Jak możesz kontrybutować do tego repozytorium?
> [!NOTE]
> Punkty wspomniane tutaj są w wielkim wyogólnieniu. W razie dalszych pytań skontaktuj się na serwerze discord OGS-devel.<br>
<br>
Witaj w OpenGaduServer! Jeżeli chcesz kontrybutować do tego projekty, to masz wiele działów, w których możesz wesprzeć rozwój projektu - nawet dzięki jednej linijce! Mając dużo sposobów kontrybucji, w podpunkcie [W czym mogę pomóc?](#2.-w-czym-moge-pomóc?) opisane jest Jak i co to znaczy wesprzeć w danej kategorii. 

------------
# Spis treści<br>
1. [Zanim wykonasz commit...](#1-zanim-wykonasz-commit)<br>
2. [W czym mogę pomóc?](#2-w-czym-mog%C4%99-pom%C3%B3c)<br>
3. [Jak zrobić zmiany?](#3-jak-zrobi%C4%87-zmiany)<br><br>
-----------

## 1. Zanim wykonasz commit...

Zanim wykonasz jakiekolwiek zmiany - musimy wiedzieć kto robi zmiany... W największym skrócie - Najlepiej żeby twoje konto posiadało dopisane Imię i nazwisko do konta. W Git możesz dodać te wartości przez:
```
	git config --global user.name "Jan 'Kowal' Kowalski"
	git config --global user.email "twojadres@poczty.pl"
```

Robiąc to, unikamy nieznanych autorów zmian w repozytorium. Jeżeli ustawienia te nie są ustawione - autor commitu byłby ukazany w taki sposób:
```
Author: ApplehatDoesStuff <152534245+ApplehatDot@users.noreply.github.com>
Date:   Sun Dec 22 15:52:35 2024 +0100

    Update README.md
```

więc **prosimy o dodanie tych rzeczy przed kontrybucją, by uniknąć nieporozumień!**

> [!NOTE]
> Kiedy wykonasz ustawienie po odesłaniu kontrybucji, możesz dodać dane autora z powrotem dzięki:<br>
> `git commit --amend --author="Jan Kowalski <twojadres@poczty.pl>"`<br>
> Jednak zalecamy by było to zrobione wcześniej, by uniknąć nieporozumień.

Poczas pisania kodu, prosimy o trzymanie się angielskich nazewnictw np. funkcji, nazw zmiennych, i czasami komentarzy - choć komentarze nie dotyczą tego. Commity mogą być w języku angielskim, ale nie muszą - *czy anglik używałby GaduGadu?*.

## 2. W czym mogę pomóc?
Każda dodana linijka pomaga rozwojowi projektu, nawet jeśli to ogranicza się do poprawiania słów w dokumentacji. Na przykład możesz pomóc w:
- **napisaniu dokumentacji** - choć istnieją dokumentacje jak libgadu, opisywanie działań serwera i starszych protokołów GG mogą się przydać bardzo... jeżeli deweloper będzie pisał jakieś dodatki
- **Implementacji funkcji do serwera OGS** - Przecież sam serwer się nie napisze... ktoś musi zaimplementować funkcję żeby klient poprawnie reagował :P<br>
		**Co musisz umieć?:**<br>
		możesz lekko opierać się na sztucznej inteligencji[^1]<br>
  		- język C,<br>
		- \<winsock>\[^2] dla win32 lub \<socket>\[^2] + inne biblioteki dla GNU/linux<br>
  		- rozumieć jak używać dokumentacji i używać ją do implementacji<br>
- **Sprawdzaniu działania Serwera na klientach GaduGadu** (wspierane wersje podane w README projektu) - Czyli innymi słowy alpha-/beta-tester samej kompilacji OGS. Osoby te, dostają gotową kompilacje od deweloperów, bądź dostają instrukcje do kompilacji[^3] 
- **Badaniu reakcji klienta przez programy sprawdzające traffic sieciowy** - Takie programy, np. Wireshark pomagają w inspektowaniu reakcji klienta na serwer. Pomaga to podczas sprawdzania odpowiedzi klienta na pakiet - gdyż dokumentacje nie wspominają o różnicach każdej z wersji protokołu GG, musimy to robić sami -  i jest zabawa!
- **Inspekcji Działania klienta dzięki narzędziom np. HxD** - jeżeli klient naprawdę skrywa sekrety, być może nadejdzie okazja by użyć disassemblera czy narzędzi tego typu.
- **Tłumaczeniu dokumentacji na Polski -> Angielski** (lub odwrotnie; zbędne) - W przypadku, kiedy popularność tego repozytorium nadejdzie aż do państw _nie-polskich_ (choć wątpie...), zapewnie przyjdzie okazja przepisywania dokumentacji na język angielski.
- **Wyszukiwaniu instalatorów klientów GaduGadu** (na chwile obecną wersji 5.x) - Aby sprawdzić kompatybilność z innymi klientami GG, potrzebujemy instalatorów wersji 5.x aby upewnić się, że serwer poprawnie komunikuje się z klientem! Takie rzeczy możesz doszukać i odzyskać ze snapshotów np. z Wayback Machine.[^3]

## 3. Jak zrobić zmiany?
Jeżeli zapoznałeś się z [punktem 1.](#1-zanim-wykonasz-commit) i wykonałeś to, co wymagane możesz już działać! 
- Musisz wykonać Fork (kopie) tego repozytorium na twój profil,
- wykonać zmiany, 
- przesłać pull request
- czekać aż sprawdzimy i wdrożymy go
i to tyle :P

[^1]: Tak jak wspomniano, możesz używać sztucznej inteligencji by ci lekko podpowiadała, ale musisz **wiedzieć jak, co działa i wiedzieć co kopiujesz!**
[^2]: szybka biblioteka kompatybilności w `src/platform.h` ułatwia problemy z róźnymi systemami... W chwili dopisywania (22.03.2026) jest priorytetowane wsparcie dla Win32, jednak staraj się tworzyć funkcje i definicje które są cross-compatible jak z win32 i GNU/linux
[^3]: Takie rzeczy należy załatwiać na serwerze discord **OGS-devel**
