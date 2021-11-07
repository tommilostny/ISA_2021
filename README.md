# ISA projekt programování síťové služby
## Varianta termínu - TFTP Klient

Autor: Tomáš Milostný (xmilos02)

---

Program **mytftpclient** je vytvořený v jazyce **C++**.
Implementace klienta pro protokol TFTP s podporou Option rozšíření.

Omezení oproti původnímu zadání: Projekt neimplementuje multicast.

---

### Příklady spuštění:

- Sestavení programu: ``make``, ``make mytftpclient``
- Spuštění programu: ``sudo ./mytftpclient``
    - ...
- Smazání přložených binárních sourobů: ``make clean``
- Zabalení projektu do .tar archivu: ``make tar``

---

### Odevzdané soubory:

* [mytftpclient.cpp](mytftpclient.cpp) - hlavní program.
* [ArgumentParser.hpp](ArgumentParser.hpp), [ArgumentParser.cpp](ArgumentParser.cpp) - parser argumentů příkazové řádky ze zadaného řetězce.
* [Tftp.hpp](Tftp.hpp),[Tftp.cpp](Tftp.cpp) - třída ``Tftp`` zajišťující operace s protokolem TFTP.
* [MessagePrinter.hpp](MessagePrinter.hpp), [MessagePrinter.cpp](MessagePrinter.cpp) - statická třída ``MessagePrinter`` pro výpis zpráv s časovou značkou na standardní a chybový výstup.

---

### Rozšiřující příkazy

* ``exit`` nebo ``quit``: ukončení programu.
* ``help``: zobrazení nápovědy.
