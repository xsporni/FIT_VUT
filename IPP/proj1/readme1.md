## Implementačná dokumentácia k 1. úlohe do IPP 2019/2020 ##
Meno a priezvisko: **Alex Sporni**  
Login: **xsporn01**  
Projekt: **Analyzátor kódu v IPPcode20**  
___
#### Návrh a filozofia projektu ####
Cieľom tohto projektu bolo vytvoriť php skript, ktorý má za úlohu previesť syntaktickú a lexikálnu analýzu vstupného kódu IPPcode20. V prípade správnej funkcionality vypíše na výstup XML reprezentáciu vstupného kódu. Jediným povinným parametrom tohto skriptu je parameter '-\-help', ktorý zobrazí nápovedu a voliteľný parameter '-\-stats' pri rozšírení.
#### Lexikálna analýza ####
Na lexikálnu kontrolu inštrukčných argumentov som využil regulárne výrazy (regexy), pomocou ktorých som vytvoril príslušné regexové vzory k jednotlivým premenným, návestiam, typom a literálom ako: bool, int, nil a string. Taktiež som vytvoril regexový vzor pre komentár, ktorý mi slúžil na overenie prítomnosti komentáru vo vstupnom programe. V prípade lexikálnej alebo syntaktickej chyby sa skript ukončí chybovým návratovým kódom 23 (error 23). Všetky regexy som vytvoril na stránke https://regex101.com/.
#### Syntaktická analýza ####
Na syntaktickú kontrolu som využil `switch()`, ktorý sa nachádza v nekonečnom  `while()` cykle, ktorý číta zdrojový kód IPPcode20 po riadkoch. Vo switchi sa kontroluje správnosť jednotlivých inštrukcii. V prípade chybnej alebo neexistujúcej inštrukcie, ktorá sa nenachádza v inštrukčnej sade sa skript ukončí chybovým návratovým kódom 22 (error 22).
#### Detaily implementácie ####
Vstupný program je čítaný zo štandardného vstupu, spracovanie vstupného programu začína načítaním prvého riadku, na ktorom by sa štandardne mala nachádzať hlavička vstupného jazyka IPPcode20. Ak sa názov hlavičky nezhoduje s predpísanou hlavičkou **.IPPcode20**, tak sa skript ukončí s chybovým návratovým kódom 21 (error 21). Po kontrole hlavičky nasleduje volanie funkcie `xml_code_init()`, ktorá inicializuje rozšírenie `XMLWriter`, ktoré som využil na pohodlné vypisovanie a formátovanie XML kódu.
Po volaní funkcie som ďalej kontroloval vstupný program na prítomnosť komentárov, ktoré som pomocou funkcie `preg_replace()` nahradil prázdnym reťazcom. Ako som v odseku *Syntaktická analýza* spomínal, tak nasleduje `switch()`, ktorý sa nachádza v nekonečnom  `while()` cykle. Vo vnútri switchu som si inštrukcie rozdelil do štyroch hlavných kategórii podľa počtu operandov. V prípade zhodnej inštrukcie sa volajú príslušné funkcie v závislosti od počtu operandov. Napríklad pri inštrukcii **RETURN** to bude funkcia `xmlwrite_zero_param()` s predanými parametrami `$ins_order, $instruction, $xw`, ktoré ďalej využívam pri výpise korektného XML kódu. Ako som vyššie spomínal na výpis XML kódu, po lexikálnej a syntaktickej analýze som použíl rozšírenie `XMLWriter` z dôvodu jeho praktickosti. 
___

#### Rozšírenie STATP #####
Rozhodol som sa implementovať aj bonusové rozšírenie, ktoré mi umožnilo zbierať štatistiky o spracovanom zdrojovom kóde IPPcode20. Rozšírenie podporuje viacero parametrov, ktoré vyplývajú zo zadania, ako -\-loc (vypíše do štatistík počet riadkov s inštrukciami), -\-comments (vypíše do štatistík počet riadkov s komentárom) atď... Toto rozšírenie si vyžaduje podporu ďalšieho parametra skriptu '-\-stats', kde treba špecifikovať súbor, do ktorého sa budú agregované štatistiky vypisovať. Zapís do súboru vykonávam pomocou funkcie `write_to_file()`. V prípade zlyhania otvorenia výstupného súboru pre zápis sa skript ukončí chybovým návratovým kódom 12 (error 12) a v prípade zlyhania zapísania do výstupného súboru sa skript ukončí chybovým návratovým kódom 99 (error 99).

