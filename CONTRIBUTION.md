# Jak możesz kontrybutować do tego repozytorium?

Witaj w OpenGaduServer! Jeżeli chcesz kontrybutować do tego projekty, to masz wiele działów, w których możesz wesprzeć rozwój projektu - nawet dzięki jednej linijce! Mając dużo sposobów kontrybucji, w podpunkcie [W czym mogę pomóc?](#2.-w-czym-moge-pomóc?) opisane jest Jak i co to znaczy wesprzeć w danej kategorii. 

------------
# Spis treści
1. Zanim wykonasz commit...
2. W czym mogę pomóc?
3. Jak zrobić zmiany?
-----------

## 1. Zanim wykonasz commit...

Zanim wykonasz jakiekolwiek zmiany - musimy wiedzieć kto robi zmiany... W największym skrócie - Najlepiej żeby twoje konto posiadało dopisane Imię i nazwisko do konta. W Git możesz dodać te wartości przez:
```
	git config --global user.name "Jan 'Kowal' Kowalski"
	git config --global user.email "twojadres@poczty.pl"
```

Robiąc to, unikamy nieznanych zmian w repozytorium, gdyby tego nie było - autor commitu byłby ukazanay w taki sposób:
```
Author: ApplehatDoesStuff <152534245+ApplehatDot@users.noreply.github.com>
Date:   Sun Dec 22 15:52:35 2024 +0100

    Update README.md
```

więc **prosimy o dodanie tych rzeczy przed kontrybucją, by uniknąć nieporozumień!**

> ![NOTE]
> Kiedy wykonasz ustawienie po odesłaniu kontrybucji, możesz dodać dane autora z powrotem dzięki:
> `git commit --amend --author="Jan Kowalski <twojadres@poczty.pl>"`
> Jednak zalecamy by było to zrobione wcześniej, by uniknąć nieporozumień.

## 2. W czym mogę pomóc?
Każda dodana linijka pomaga rozwojowi projektu, nawet jeśli to ogranicza się do poprawiania słów w dokumentacji. Na przykład możesz pomóc w:
- **napisaniu dokumentacji**
- **Implementacji funkcji do serwera OGS**,
- **Sprawdzaniu działania Serwera na klientach GaduGadu** (wspierane wersje podane w README projektu),
- **Badanie reakcji klienta przez programy sprawdzające traffic sieciowy**
- **Tłumaczeniu dokumentacji na Polski -> Angielski** (lub odwrotnie)
- **Wyszukiwaniu instalatorów klientów GaduGadu** (na chwile obecną wersjii 5.x)

## 3. Jak zrobić zmiany?

Kiedy ustawisz konto, możesz już działać! 
- Musisz wykonać Fork (kopie) tego repozytorium na twój profil,
- wykonać zmiany, 
- przesłać pull request
- czekać aż sprawdzimy i wdrożymy go
i to tyle :P