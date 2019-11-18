**DNS**

**Autor:** Alex Sporni, xsporn01@stud.fit.vutbr.cz

**Dátum:** 18.11.2019

**Popis:**

Resolver je klientský program, ktorý sa dotazuje na dáta uložené v systéme DNS. Užívateľské programy, ktoré potrebujú informácie z DNS, pristupujú k týmto dátam pomocou resolveru. 
Základnou úlohou resolveru je:
___

* posielať dotazy na servery DNS
* interpretovať odpovede od serveru (prijaté záznamy, chybové hlásenia),
* predať informácie užívateľskému programu, ktorý o~dáta žiadal.
___

Program je spustiteľný na operačných systémoch Linux, bol vyvinutý a otestovaný na serveroch  eva.fit.vutbr.cz a merlin.fit.vutbr.cz
___

**Zoznam súborov:**

* Makefile - prekladový súbor
* manual.pdf - projektová dokumentáia
* Readme - daný súbor
* dns.cpp - zdrojový kód projektu
___

**Makefile:**
* $ make - preloží program
* $ make clean - odstráni preložený program
* $ make tar - vytvorí archív xsporn01.tar
* $ make rmtar - odstráni archív xsporn01.tar
___
**Preklad:**
* Preklad prebieha pomocou Makefilu alebo pomocou príkazu g++ -Wall -pedantic -Wextra dns.cpp -o dns
___
**Príklad spustenia:**

dns [-r] [-x] [-6] -s server [-p port] adresa

argumenty v hranatých zátvorkách sú voliteľné

**Konkrétny príkaz spustenia:**

./dns -r -s kazi.fit.vutbr.cz www.fit.vut.cz

___
**PZ: Projekt neobsahuje žiadne rozšírenia**