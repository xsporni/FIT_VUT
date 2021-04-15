## Implementačná dokumentácia k 2. úlohe do IPP 2019/2020 ##
Meno a priezvisko: **Alex Sporni**  
Login: **xsporn01**  
Projekt: **Interpret XML reprezentácie kódu**  
File:  **`interpreter.py`**
___
### **Interpret XML reprezentácie kódu IPPcode20** ###
#### Návrh a filozofia projektu ####
Cieľom tohto projektu bolo vytvoriť skript v jazyku `python3.8`, ktorý má za úlohu načítať XML reprezentáciu programu a interpretovať ho. Skript zabezpečuje syntaktickú, lexikálnu a sémantickú analýzu zdrojového kódu.

#### Štruktúra programu ####
Skript `interpret.py` je navrhnutý s využitím objektovo-orientovaných princípov a je tvorený nasledovnými triedami:  
* Trieda *`Variable()`*   
    * Trieda má za úlohu inicializovať a ukladať údaje o jednotlivých premenných
    * Údaje, ktoré trieda ukladá: meno, typ a hodnota 
* Trieda *`Interpret()`*
    * Trieda interpret je hlavnou triedou celého skriptu
    * Obsahuje metódy pre interpretáciu
    * Prebieha v nej samotná interpretácia zdrojového kódu 

#### Detaily implementácie ####
Implementácia skriptu začína spracovaním vstupných argumentov. Na túto úlohu bol importovaný modul `argparse`. Na spracovanie vstupného XML som využil modul `xml.etree.ElementTree`.   
Zdrojový XML kód a vstup programu je možné načítač ako zo súboru, tak aj zo štandardného vstupu. Pri načítaní XML súboru pomocou spomenutej knižnice `xml.etree.ElementTree` je nutné zoradiť inštrukcie podľa poradia **order** a to stúpajúcou postupnosťou (vzostupne). Pred samotnou interpretáciou je potrebné skontrolovať vstupný súbor, či sa v ňom nenachádzajú nepovolené znaky, nadbytočný text, či sedí hlavička a či neobsahuje voliteľné atribúty ako: **description** a **name**. Ak áno, rozhodol som sa ich z XML štruktúry odstrániť, pretože sú pre samotnú interpretáciu nepodstatné. Ďalej je súbor predaný pre vytvorenie všetkých návestí pomocou metódy `create_label_for_xml_appearance()`, ktorá sa nachádza v triede *`Interpret()`*. Hlavná funkcia `main_parser()` je volaná na každú inštrukciu samostatne.  
Pri jednotlivých inštrukciách sa postupne kontroluje najprv syntaktická správnosť pomocou funkcie `parse_ins_args()`, ktorá kontroluje správnosť názvu inštrukcii a vracia počet inštrukčných argumentov. Po tejto kontrole prichádza na rad samotná interpretácia jednotlivých inštrukcii, ktoré sú rozdelené do kategórii podľa počtu inštrukčných argumentov. Sémantickú a lexikálnu správnosť zabezpečujú príslušné metódy, ktoré sa nachádzajú v hlavnej triede *`Interpret()`* a sú volané k jednotlivých inštrukciám podľa potreby. Premenné sú vytvárané pomocou triedy *`Variable()`* a sú ukladané do rámcov. V prípade, že sa jedná o globálny rámec som použil dátový typ `dictionary` (slovník) a v prípade, že sa jedná o lokálny a dočastný rámec som zvolil dátový typ `list` (zoznam).  
Pokiaľ interpretácia zdrojového kódu prebehne úspešne tak je interpretovaný celý vstupný XML kód s návratovým kódom úspechu 0.
___
#### Rozšírenia #####
Rozhodol som sa implementovať aj bonusové rozšírenie **STATI**, ktoré mi umožnilo zbierať štatistiky o spracovanom zdrojovom kóde. Rozšírenie podporuje viacero parametrov, ktoré vyplývajú zo zadania ako **-\-insts** (výpis počtu vykonaných inštrukcii behom interpretácie) a **-\-vars** (výpis maximálneho počtu inicializovaných premenných).  
Rozšírenie STATI vyžadovalo pridanie ďalších parametrov do `argparse`. Ďalej bolo potrebné vytvoriť počítadlo vykonaných inštrukcii **(stati_counter)** a premenných **(var_counter)**. Vytvoril som funkciu `stati_vars_counter()`, ktorá počítadlo premenných vždy aktualizuje.
