# Implementowanie protokołu GaduGadu
Projekt **OpenGaduServer** jest oparty na kilku projektach społeczności np. bibliotekach, alternatywnych klientach itd.
Źródła podane niżej są tymi, które są najczęściej używane podczas doszukiwania się błędów czy implementacji funkcji.

## Projekty społeczności
Tutaj podane są projekty społeczności, na którym oparty jest ten projekt.

### LibGadu _- Wojtekka and LibGadu Team[^1]_: <br>
Tłumaczy większość głównych funkcji i działania klienta GaduGadu. Wersja na stronie WWW tłumaczy najlepiej wersje 6.0 i nowsze, natomiast repozytorium na Githubie pokazuje implementacje kodu w języku C.
- WWW: https://libgadu.github.io/protocol<br> 
- Github: https://github.com/wojtekka/libgadu<br>
### Uniksowy Serwer Gadu _- Wojtekka:_ <br>
Jest prostą implementacją logiki serwera, Projekt ten pomaga w oczyszczaniu kodu i poprawnym zrozumieniu, w jaki sposób serwer powinien działać i komunikować się z klientem. Projekt ten jest już dawno nie wspierany.
- Github: https://github.com/wojtekka/usg<br>
### Eksperymentalny Klient Gadu (EKG) _- EKG team[^1]_
Oryginalne źródło skąd protokół GaduGadu był inżynierowany wstecznie (eng. _reserve engineered_). Ale później stał się osobnym projektem.
- WWW: http://ekg.chmurka.net/download.php
- Github: https://github.com/ekg2/ekg2/

## Progamy
### Wireshark<br>
Najlepszy i dosyć zaawansowany program do sprawdzania przepustu sieciowego, według mojej małej wiedzy. Program ten m.in wykrywa protokół GaduGadu, i przede wszystkim pakiety HTTP wysyłane do/z serwera
- WWW: https://www.wireshark.org/download.html

[^1]: Zsumowano osoby, które pracowały we wspomnianych repozytoriach. Możesz znaleźć wszystkich współautorów w pliku `AUTHORS` projektu.
