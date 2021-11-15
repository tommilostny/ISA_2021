# ISA projekt programování síťové služby
## Varianta termínu - TFTP Klient

Autor: Tomáš Milostný (xmilos02)

---

Program **mytftpclient** je vytvořený v jazyce **C++**.
Implementace klienta pro protokol **TFTP** s podporou [Option Extension](https://datatracker.ietf.org/doc/html/rfc2347).

Omezení oproti původnímu zadání: Projekt neimplementuje multicast.

---

### Příklady spuštění:

- Sestavení programu: ``make``, ``make mytftpclient``
- Spuštění programu: ``make run``
    - Příkazová řádka programu ``mytftpclient``:
        - Stažení textového souboru na výchozí server (localhost, port 69):
            > \> -d hello.txt -R -c ascii

            > \> -d hello.txt -R -c netascii
        - Zápis textového souboru na jiný server na portu 8888:
            > \> -W -c ascii -d hello.txt -a 147.229.176.14,8888
        - Stažení binárního souboru s nastavenou velikostí bloku a časovým limitem:
            > \> -s 64000 -R -d cw2.mp4 -t 3
        - Zápis textového souboru s nastavenou velikostí bloku:
            > \> -d hello.txt -s 16 -W
        - Stažení binárního souboru s explicitně zadaným argumentem -c:
            > \> -d Victory.wav -R -c binary

            > \> -d Victory.wav -R -c octet
        - Výpis nápovědy:
            > \> help
        - Ukončení programu:
            > \> quit

            > \> exit
- Smazání přeložených binárních sourobů: ``make clean``
- Zabalení projektu do .tar archivu: ``make tar``

---

### Odevzdané soubory:

* [mytftpclient.cpp](mytftpclient.cpp) - hlavní program.
* [ArgumentParser.hpp](ArgumentParser.hpp), [ArgumentParser.cpp](ArgumentParser.cpp) - parser argumentů příkazové řádky ze zadaného řetězce.
* [Tftp.hpp](Tftp.hpp),[Tftp.cpp](Tftp.cpp) - třída ``Tftp`` zajišťující operace s protokolem TFTP.
* [StampMessagePrinter.hpp](StampMessagePrinter.hpp), [StampMessagePrinter.cpp](StampMessagePrinter.cpp) - statická třída ``StampMessagePrinter`` pro výpis zpráv s časovou značkou na standardní výstup a chybový výstup.

---

### Rozšiřující příkazy:

* ``exit`` nebo ``quit``: ukončení programu.
* ``help``: zobrazení nápovědy.
