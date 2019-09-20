----------------------------------------------------------
--- IDS projekt: SQL script                            ---
--- DĂˇtum: 29.04.2019                                  ---
--- Autori:                                            ---
---   Alex Sporni (xsporn01@stud.fit.vutbr.cz)         ---
---   Igor Mjasojedov (xmjaso00@stud.fit.vutbr.cz)     ---
----------------------------------------------------------

-----------------------------------------------------
--- VYMAZANIE TABULIEK ---                          -
-----------------------------------------------------
DROP TABLE klient CASCADE CONSTRAINTS;
DROP TABLE automechanik CASCADE CONSTRAINTS;
DROP TABLE vozidlo CASCADE CONSTRAINTS;
DROP TABLE objednavka CASCADE CONSTRAINTS;
DROP TABLE vyrobca CASCADE CONSTRAINTS;
DROP TABLE vyrobca_dodavatel CASCADE CONSTRAINTS;
DROP TABLE dodavatel CASCADE CONSTRAINTS;
DROP TABLE faktura CASCADE CONSTRAINTS;
DROP TABLE oprava CASCADE CONSTRAINTS;
DROP TABLE oprava_meterial CASCADE CONSTRAINTS;
DROP TABLE material CASCADE CONSTRAINTS;

DROP SEQUENCE klient_seq;
DROP INDEX oprava_info_I;
DROP VIEW automechanik_zataz;     
DROP MATERIALIZED VIEW automechanik_zataz_materialized;  
-----------------------------------------------------
--- VYTVORENIE TABULKY ---                          -
-----------------------------------------------------
CREATE TABLE klient (
    ID_klient NUMBER,
    rodne_cislo NUMBER NOT NULL,
    meno VARCHAR2(20) NOT NULL,
    priezvisko VARCHAR2(20) NOT NULL,
    adresa VARCHAR2(30) NOT NULL,
    mesto VARCHAR2(30) NOT NULL,
    ZIP_kod NUMBER NOT NULL,
    tel_cislo NUMBER(10) NOT NULL,
    email VARCHAR2(30),
    PRIMARY KEY (ID_klient),
    CONSTRAINT check_rodne_cislo_klient CHECK(LENGTH(TRIM(rodne_cislo))=10)
);

CREATE TABLE automechanik (
    ID_automechanik NUMBER GENERATED ALWAYS AS IDENTITY,
    rodne_cislo NUMBER NOT NULL,
    meno VARCHAR2(20) NOT NULL,
    priezvisko VARCHAR2(20) NOT NULL,
    adresa VARCHAR2(30) NOT NULL,
    mesto VARCHAR2(30) NOT NULL,
    ZIP_kod NUMBER NOT NULL,
    tel_cislo NUMBER(10) NOT NULL,
    email VARCHAR2(30),
    specializacia VARCHAR2(30) NOT NULL,
    hod_mzda NUMBER NOT NULL,
    PRIMARY KEY (ID_automechanik),
    CONSTRAINT check_rodne_cislo_automechanik CHECK(LENGTH(TRIM(rodne_cislo))=10)
);

CREATE TABLE vyrobca (
    ID_vyrobca NUMBER GENERATED ALWAYS AS IDENTITY,
    nazov VARCHAR2(20) NOT NULL UNIQUE,
    PRIMARY KEY (ID_vyrobca)
);

CREATE TABLE vozidlo (
    ID_vozidlo NUMBER GENERATED ALWAYS AS IDENTITY,
    id_vyrobca NUMBER,
    id_klient NUMBER,
    vin VARCHAR2(17) NOT NULL UNIQUE,
    spz VARCHAR2(7),
    datum_vyroby DATE,
    farba VARCHAR2(15),
    PRIMARY KEY (ID_vozidlo),
    CONSTRAINT check_vin CHECK(LENGTH(TRIM(vin))=17),
    FOREIGN KEY (id_vyrobca) REFERENCES vyrobca(ID_vyrobca) ON DELETE SET NULL,
    FOREIGN KEY (id_klient) REFERENCES klient(ID_klient) ON DELETE SET NULL
);

CREATE TABLE objednavka (
    ID_objednavka NUMBER GENERATED ALWAYS AS IDENTITY,
    id_vozidlo NUMBER,
    id_automechanik NUMBER,
    datum_prijatia DATE NOT NULL,
    datum_vratenia_vozidla DATE NOT NULL,
    PRIMARY KEY (ID_objednavka),
    FOREIGN KEY (id_vozidlo) REFERENCES vozidlo(ID_vozidlo) ON DELETE SET NULL,
    FOREIGN KEY (id_automechanik) REFERENCES automechanik(ID_automechanik) ON DELETE SET NULL
);

CREATE TABLE dodavatel (
    ID_dodavatel NUMBER GENERATED ALWAYS AS IDENTITY,
    nazov VARCHAR2(20) NOT NULL,
    ico CHAR(8),
    tel_cislo NUMBER(10) NOT NULL,
    web_url VARCHAR2(50) NOT NULL,
    PRIMARY KEY (ID_dodavatel),
    CONSTRAINT check_ico CHECK(LENGTH(TRIM(ico))=8)
);

CREATE TABLE vyrobca_dodavatel (
    id_vyrobca NUMBER,
    id_dodavatel NUMBER,
    CONSTRAINT ID_vyrobca_dodavatel PRIMARY KEY (id_vyrobca, id_dodavatel),
    FOREIGN KEY (id_vyrobca) REFERENCES vyrobca(ID_vyrobca) ON DELETE CASCADE,
    FOREIGN KEY (id_dodavatel) REFERENCES dodavatel(ID_dodavatel) ON DELETE CASCADE
);

CREATE TABLE faktura (
    ID_faktura NUMBER GENERATED ALWAYS AS IDENTITY,
    id_objednavka NUMBER,
    cislo_faktury NUMBER NOT NULL,
    PRIMARY KEY (ID_faktura),
    FOREIGN KEY (id_objednavka) REFERENCES objednavka(ID_objednavka) ON DELETE CASCADE
);

CREATE TABLE oprava (
    ID_oprava NUMBER GENERATED ALWAYS AS IDENTITY,
    id_objednavka NUMBER,
    nazov VARCHAR2(30) NOT NULL,
    popis VARCHAR2(300) NOT NULL,
    zaciatok DATE NOT NULL,
    koniec DATE NOT NULL,
    PRIMARY KEY (ID_oprava),
    FOREIGN KEY (id_objednavka) REFERENCES objednavka(ID_objednavka) ON DELETE SET NULL
);

CREATE TABLE material (
    ID_material NUMBER GENERATED ALWAYS AS IDENTITY,
    typ_dielu VARCHAR2(30) NOT NULL,
    cena_za_kus NUMBER NOT NULL,
    seriove_cislo NUMBER NOT NULL,
    PRIMARY KEY (ID_material)
);

CREATE TABLE oprava_meterial (
    id_oprava NUMBER,
    id_material NUMBER,
    CONSTRAINT ID_orpava_meterial PRIMARY KEY (id_oprava, id_material),
    FOREIGN KEY(id_oprava) REFERENCES oprava(ID_oprava) ON DELETE CASCADE,
    FOREIGN KEY(id_material) REFERENCES material(ID_material) ON DELETE CASCADE
);

---------------------------------------------------------------------------------
-----------------------------------------------------
--- TRIGGERS ---                                    - 
-----------------------------------------------------
CREATE SEQUENCE klient_seq START WITH 1 INCREMENT BY 1;

CREATE OR REPLACE TRIGGER generate_new_PK
    BEFORE INSERT ON klient
    FOR EACH ROW
BEGIN
    SELECT klient_seq.nextval 
    INTO :NEW.ID_klient
    FROM dual;
END;
/

CREATE OR REPLACE TRIGGER kontrola_RC
BEFORE INSERT OR UPDATE ON klient
FOR EACH ROW
DECLARE
    rodne_cislo klient.rodne_cislo%TYPE;
    mesiac VARCHAR2(2);
    den VARCHAR2(2);
BEGIN
    rodne_cislo := :NEW.rodne_cislo;       
    mesiac := SUBSTR(rodne_cislo, 3, 2);
    IF (mesiac > 50) THEN
        mesiac := mesiac - 50;
    END IF;    
    den := SUBSTR(rodne_cislo, 5, 2);
    
    -- kontrola delitelnosti rodneho cisla cislom 11
    IF (MOD(rodne_cislo, 11) <> 0) THEN
        Raise_Application_Error(-20420, 'Zly format rodneho cisla!');
    END IF; 
    
    IF (mesiac > 12) THEN
        Raise_Application_Error(-20421, 'Nevalidne cislo mesiaca!');
    END IF;
    
    IF (den > 31) THEN
        Raise_Application_Error(-20422, 'Nevalidne cislo dna!');
    END IF;     
END;    
/

CREATE OR REPLACE TRIGGER kontrola_datumy
BEFORE INSERT OR UPDATE ON oprava
FOR EACH ROW
DECLARE
    zaciatok oprava.zaciatok%TYPE;
    koniec oprava.koniec%TYPE;
BEGIN
    IF (:NEW.zaciatok > :NEW.koniec) THEN
        Raise_Application_Error(-20423, 'Nevalidny datum ukoncenia opravy!');
    END IF;
END;    
/

---------------------------------------------------------------------------------
-----------------------------------------------------
--- VLKADANIE ZAZNAMOV ---                          - 
-----------------------------------------------------
-- klient --
INSERT INTO klient(rodne_cislo, meno, priezvisko, adresa, mesto, ZIP_kod, tel_cislo, email) 
    VALUES('9307228414', 'Igor', 'Zelenaj', 'Janosikova 2', 'Nove Zamky', '94002', '0914293493', 'igor.zelenaj@gmail.com');
INSERT INTO klient(rodne_cislo, meno, priezvisko, adresa, mesto, ZIP_kod, tel_cislo, email) 
    VALUES('9302015822', 'Adam', 'Krust', 'Vajanskeho 12', 'Pezinok', '90201', '0914123493', 'adam.krust@gmail.com');
INSERT INTO klient(rodne_cislo, meno, priezvisko, adresa, mesto, ZIP_kod, tel_cislo, email) 
    VALUES('9803085765', 'Peter', 'Mac', 'Janka Krala 14', 'Bratislava', '84105', '09142493493', 'peter.mac@gmail.com');
INSERT INTO klient(rodne_cislo, meno, priezvisko, adresa, mesto, ZIP_kod, tel_cislo, email) 
    VALUES('9201277371', 'Dmitrij', 'Orlov', 'Jezova 1', 'Nove Zamky', '94002', '0914293243', 'dmitrij.orlov@gmail.com');
INSERT INTO klient(rodne_cislo, meno, priezvisko, adresa, mesto, ZIP_kod, tel_cislo, email) 
    VALUES('9301215583', 'Daniel', 'Mancir', 'Hlavna 21', 'Nitra', '94907', '0914296893', 'daniel.mancir@gmail.com');
INSERT INTO klient(rodne_cislo, meno, priezvisko, adresa, mesto, ZIP_kod, tel_cislo, email) 
    VALUES('9454186401', 'Alica', 'Prudka', 'Chladna 88', 'Poprad', '94909', '0914363893', 'alica.prudka@gmail.com');
INSERT INTO klient(rodne_cislo, meno, priezvisko, adresa, mesto, ZIP_kod, tel_cislo, email) 
    VALUES('9760039751', 'Erika', 'Fialova', 'Zelena 16', 'Kosice', '92505', '0915286893', 'erika.fialova@gmail.com');

-- automechanik --
INSERT INTO automechanik(rodne_cislo, meno, priezvisko, adresa, mesto, ZIP_kod, tel_cislo, email, specializacia, hod_mzda) 
    VALUES('9211079735', 'Anton', 'Trava', 'Fuzna 21', 'Nitra', '94907', '0914296811', 'adnton.trava@gmail.com', 'motory', '10');
INSERT INTO automechanik(rodne_cislo, meno, priezvisko, adresa, mesto, ZIP_kod, tel_cislo, email, specializacia, hod_mzda) 
    VALUES('8605124231', 'Daniel', 'Borbely', 'Lesna 1', 'Tvrdosovce', '94002', '0912246821', 'danko.borbi@fit.cz', 'stierace', '4');
INSERT INTO automechanik(rodne_cislo, meno, priezvisko, adresa, mesto, ZIP_kod, tel_cislo, email, specializacia, hod_mzda) 
    VALUES('9508011128', 'Gregor', 'Dostoj', 'Ruska 13', 'Nove Zamky', '94002', '0914226811', 'gregor.dostoj@vk.com', 'prevodovky', '8');
INSERT INTO automechanik(rodne_cislo, meno, priezvisko, adresa, mesto, ZIP_kod, tel_cislo, email, specializacia, hod_mzda) 
    VALUES('9011250336', 'Pavol', 'Luka', 'Janska 3', 'Nitra', '94907', '0914299911', 'pavol.luka@gmail.com', 'full-stack', '13');
INSERT INTO automechanik(rodne_cislo, meno, priezvisko, adresa, mesto, ZIP_kod, tel_cislo, email, specializacia, hod_mzda) 
    VALUES('8403144563', 'Janko', 'Steblo', 'Hladova 9', 'Nove Zamky', '99646', '0914299420', 'janko.steblo@post.com', 'lak', '8');    

-- vyrobca --
INSERT INTO vyrobca(nazov) VALUES('Audi');
INSERT INTO vyrobca(nazov) VALUES('BMW');
INSERT INTO vyrobca(nazov) VALUES('Mercedes-Benz');
INSERT INTO vyrobca(nazov) VALUES('Volvo');
INSERT INTO vyrobca(nazov) VALUES('Opel');

-- vozidlo --
INSERT INTO vozidlo(id_vyrobca, id_klient, vin, spz, datum_vyroby, farba) VALUES('1', '3', '1HGCM82633A004352', 'BA123CI', '19.12.2008', 'cosmic-blue');
INSERT INTO vozidlo(id_vyrobca, id_klient, vin, spz, datum_vyroby, farba) VALUES('3', '4', '42GCM82666A014352', 'NZ123JE', '01.01.2013', 'white');
INSERT INTO vozidlo(id_vyrobca, id_klient, vin, spz, datum_vyroby, farba) VALUES('2', '5', '3HFCM82543B004872', 'NR423DA', '20.08.2017', 'black');
INSERT INTO vozidlo(id_vyrobca, id_klient, vin, spz, datum_vyroby, farba) VALUES('1', '1', '1EDCM12644A404251', 'NZ261CP', '01.02.2002', 'red');
INSERT INTO vozidlo(id_vyrobca, id_klient, vin, spz, datum_vyroby, farba) VALUES('4', '2', '2HGAB81614A014102', 'PK903PP', '13.05.2008', 'dark-green');

-- dodavatel --
INSERT INTO dodavatel(nazov, ico, tel_cislo, web_url) VALUES('Autoconf', '92401234', '0914296893', 'autoconf.sk');
INSERT INTO dodavatel(nazov, ico, tel_cislo, web_url) VALUES('DraftParts', '12723234', '0923223813', 'draftparts.sk');
INSERT INTO dodavatel(nazov, ico, tel_cislo, web_url) VALUES('MobileAuto', '92248834', '0942962343', 'mobileauto.sk');
INSERT INTO dodavatel(nazov, ico, tel_cislo, web_url) VALUES('ComponentCar', '01358493', '0913212644', 'componentcar.sk');

--- objednavka ---
INSERT INTO objednavka (id_vozidlo, id_automechanik, datum_prijatia, datum_vratenia_vozidla) VALUES('1','1', TO_DATE('03-05-2018','dd-mm-yyyy'),TO_DATE('07-05-2018', 'dd-mm-yyyy'));
INSERT INTO objednavka (id_vozidlo, id_automechanik, datum_prijatia, datum_vratenia_vozidla) VALUES('2','2', TO_DATE('01-06-2018','dd-mm-yyyy'),TO_DATE('12-06-2018', 'dd-mm-yyyy'));
INSERT INTO objednavka (id_vozidlo, id_automechanik, datum_prijatia, datum_vratenia_vozidla) VALUES('3','3', TO_DATE('13-06-2018','dd-mm-yyyy'),TO_DATE('22-06-2018', 'dd-mm-yyyy'));
INSERT INTO objednavka (id_vozidlo, id_automechanik, datum_prijatia, datum_vratenia_vozidla) VALUES('4','2', TO_DATE('25-03-2018','dd-mm-yyyy'),TO_DATE('29-06-2018', 'dd-mm-yyyy'));
INSERT INTO objednavka (id_vozidlo, id_automechanik, datum_prijatia, datum_vratenia_vozidla) VALUES('5','1', TO_DATE('06-07-2018','dd-mm-yyyy'),TO_DATE('14-07-2018', 'dd-mm-yyyy'));
--- objednavky s prebiehajucou opravou ---
INSERT INTO objednavka (id_vozidlo, id_automechanik, datum_prijatia, datum_vratenia_vozidla) VALUES('2','4', TO_DATE('17-03-2019','dd-mm-yyyy'),TO_DATE('01-06-2019', 'dd-mm-yyyy'));
INSERT INTO objednavka (id_vozidlo, id_automechanik, datum_prijatia, datum_vratenia_vozidla) VALUES('4','5', TO_DATE('20-04-2019','dd-mm-yyyy'),TO_DATE('08-05-2019', 'dd-mm-yyyy'));
INSERT INTO objednavka (id_vozidlo, id_automechanik, datum_prijatia, datum_vratenia_vozidla) VALUES('1','1', TO_DATE('29-04-2019','dd-mm-yyyy'),TO_DATE('20-05-2019', 'dd-mm-yyyy'));

--- faktura ---
INSERT INTO faktura (id_objednavka, cislo_faktury) VALUES('1', '00001');
INSERT INTO faktura (id_objednavka, cislo_faktury) VALUES('2', '00002');
INSERT INTO faktura (id_objednavka, cislo_faktury) VALUES('3', '00003');
INSERT INTO faktura (id_objednavka, cislo_faktury) VALUES('4', '00004');
INSERT INTO faktura (id_objednavka, cislo_faktury) VALUES('5', '00005');

--- oprava ---
INSERT INTO oprava (id_objednavka, nazov, popis, zaciatok, koniec) VALUES('1', 'deravy vyfuk', 'deravy vyfuk z dosledku korozie materialu', TO_DATE('03-05-2018','dd-mm-yyyy'),TO_DATE('07-05-2018', 'dd-mm-yyyy'));
INSERT INTO oprava (id_objednavka, nazov, popis, zaciatok, koniec) VALUES('2', 'defekt na zadnej naprave', 'vznik defektu sa viaze na opotrebovany dezen pneumatik', TO_DATE('01-06-2018','dd-mm-yyyy'),TO_DATE('12-06-2018', 'dd-mm-yyyy'));
INSERT INTO oprava (id_objednavka, nazov, popis, zaciatok, koniec) VALUES('3', 'prasknute celne sklo', 'rozsiahla prasklina na celnom skle, pravdepedobna pricina odrazeny kamen', TO_DATE('13-06-2018','dd-mm-yyyy'),TO_DATE('22-06-2018', 'dd-mm-yyyy'));
INSERT INTO oprava (id_objednavka, nazov, popis, zaciatok, koniec) VALUES('4', 'zapecenie motora', 'neprimerana jazda k stavu vozidla', TO_DATE('25-03-2018','dd-mm-yyyy'),TO_DATE('29-06-2018', 'dd-mm-yyyy'));
INSERT INTO oprava (id_objednavka, nazov, popis, zaciatok, koniec) VALUES('5', 'odtrhnuty klinovy remen', 'pravdepodobna pricina opotrebovanie materialu', TO_DATE('06-07-2018','dd-mm-yyyy'),TO_DATE('14-07-2018', 'dd-mm-yyyy'));
--- prebiehajuca oprava ---
INSERT INTO oprava (id_objednavka, nazov, popis, zaciatok, koniec) VALUES('6', 'Stahovanie okien', 'poskodenie motorceka z dovodu zapadnutia prachom', TO_DATE('17-03-2019','dd-mm-yyyy'),TO_DATE('01-06-2019', 'dd-mm-yyyy'));
INSERT INTO oprava (id_objednavka, nazov, popis, zaciatok, koniec) VALUES('7', 'Brzdovy system', 'poskodene brzdy na zadnej naprave = vydraty kotuc', TO_DATE('20-04-2019','dd-mm-yyyy'),TO_DATE('08-05-2019', 'dd-mm-yyyy'));
INSERT INTO oprava (id_objednavka, nazov, popis, zaciatok, koniec) VALUES('8', 'Elektroinstalacia', 'nefunkcna palubna doska', TO_DATE('29-04-2019','dd-mm-yyyy'),TO_DATE('20-05-2019', 'dd-mm-yyyy'));
INSERT INTO oprava (id_objednavka, nazov, popis, zaciatok, koniec) VALUES('1', 'Vymena oleja', 'Servisna prehliadka-vymena oleja po 15000km', TO_DATE('02-05-2019','dd-mm-yyyy'),TO_DATE('11-05-2019', 'dd-mm-yyyy'));

-- material --
INSERT INTO material (typ_dielu, cena_za_kus, seriove_cislo) VALUES('DPF filter', '20', '032321');
INSERT INTO material (typ_dielu, cena_za_kus, seriove_cislo) VALUES('pneumatika', '50', '232341');
INSERT INTO material (typ_dielu, cena_za_kus, seriove_cislo) VALUES('celne-sklo', '158', '012421');
INSERT INTO material (typ_dielu, cena_za_kus, seriove_cislo) VALUES('motor', '3999', '059821');
INSERT INTO material (typ_dielu, cena_za_kus, seriove_cislo) VALUES('klinovy remen', '666', '029521');
INSERT INTO material (typ_dielu, cena_za_kus, seriove_cislo) VALUES('W30-D motorovy olej', '50', '024521');
INSERT INTO material (typ_dielu, cena_za_kus, seriove_cislo) VALUES('elektricky motorcek', '33', '034351');
INSERT INTO material (typ_dielu, cena_za_kus, seriove_cislo) VALUES('brzdova kvapalina', '15', '011151');
INSERT INTO material (typ_dielu, cena_za_kus, seriove_cislo) VALUES('palubny pocitac', '300', '264357');

-- vyrobca_dodavatel --
INSERT INTO vyrobca_dodavatel(id_vyrobca, id_dodavatel) VALUES('1', '3');
INSERT INTO vyrobca_dodavatel(id_vyrobca, id_dodavatel) VALUES('2', '2');
INSERT INTO vyrobca_dodavatel(id_vyrobca, id_dodavatel) VALUES('3', '4');
INSERT INTO vyrobca_dodavatel(id_vyrobca, id_dodavatel) VALUES('4', '1');
INSERT INTO vyrobca_dodavatel(id_vyrobca, id_dodavatel) VALUES('5', '4');

-- oprava_meterial --
INSERT INTO oprava_meterial(id_oprava, id_material) VALUES('2', '2');
INSERT INTO oprava_meterial(id_oprava, id_material) VALUES('1', '1');
INSERT INTO oprava_meterial(id_oprava, id_material) VALUES('5', '5');
INSERT INTO oprava_meterial(id_oprava, id_material) VALUES('3', '3');
INSERT INTO oprava_meterial(id_oprava, id_material) VALUES('4', '6');
INSERT INTO oprava_meterial(id_oprava, id_material) VALUES('4', '4');

---------------------------------------------------------------------------------
-----------------------------------------------------
--- SELECTS ---                                     - 
-----------------------------------------------------

--? Informacie o klientovy a jeho vozidle. --
SELECT meno, priezvisko, email, vin, spz FROM klient NATURAL JOIN vozidlo;

--? Podrobne informacie o vozidle. --
SELECT nazov, vin, spz, datum_vyroby, farba 
FROM vozidlo NATURAL JOIN vyrobca;

--? Informacie o zavade vozidla, informacie o danej objednavke a o pridelenom automechanikovi --
--? Vyuzitie pri evidovani zodpovednosti pre danu opravu --
SELECT A.meno, A.priezvisko, A.email, datum_prijatia, datum_vratenia_vozidla, O.nazov, O.popis 
FROM objednavka NATURAL JOIN oprava O NATURAL JOIN automechanik A;

--? Informacie o vytazenosti jednotlivych mechanikov. --
--? Vyuzitie administrativneho pracovnika pri rozdelovani odmien na zaklade vytazenosti automechanika --
SELECT meno, priezvisko, COUNT(id_objednavka) AS vytazenost
FROM automechanik NATURAL LEFT JOIN objednavka
GROUP BY meno, priezvisko
ORDER BY vytazenost DESC;

--? Informacie o oprave konkretneho vozidla a nakladoch na nahradne diely. --
--? Vyuzitie pri vystavovani faktury pre danu opravu. --
SELECT O.nazov, O.popis, V.vin, SUM(M.cena_za_kus) AS naklady
FROM oprava_meterial NATURAL JOIN material M NATURAL JOIN oprava O NATURAL JOIN objednavka NATURAL JOIN vozidlo V
GROUP BY O.nazov, O.popis, V.vin
ORDER BY naklady;

--? Informacie o automechanikoch, ktori aktualne nepracuju na ziadnej objednavke. --
--? Vyuzitie administrativneho pracovnika pri rozdelovani prace. --
SELECT DISTINCT A.meno, A.priezvisko, A.tel_cislo, A.email 
FROM automechanik A, objednavka O
WHERE NOT EXISTS (
        SELECT * FROM objednavka O
        WHERE A.id_automechanik=O.id_automechanik);

--? Informacie o klientoch, ktori priniesli vozidlo do opravy v mesiaci JUN. --
--? Vyuzitie pri zasielani voucherov na letnu kontrolu vozidla. --
SELECT meno, priezvisko, adresa, mesto, zip_kod, email
FROM klient
WHERE id_klient IN
    (SELECT id_klient FROM vozidlo
     WHERE id_vozidlo IN
        (SELECT id_vozidlo FROM objednavka
         WHERE datum_prijatia BETWEEN '01-06-2018' AND '30-06-2018'));


-----------------------------------------------------
--- PROCEDURES ---                                  - 
-----------------------------------------------------
SET serveroutput ON;

-- Vypise sa percentualna zataz autoopravne v zadanom meste, na zaklade vytazenosti automechanikov
CREATE OR REPLACE PROCEDURE prevadzka_zataz(mesto_t IN VARCHAR2)
IS
    CURSOR automechanik_c IS SELECT * FROM automechanik natural left join objednavka natural left join oprava;  
    tmp automechanik_c%ROWTYPE;
    dnesny_datum VARCHAR2(20);
    pocet_automechanikov NUMBER;
    pocet_automechanikov_np NUMBER;
    percenta_vytazenosti NUMBER;
BEGIN
    SELECT TO_CHAR(Sysdate, 'DD-MM-YYYY') into dnesny_datum FROM Dual; 
    SELECT COUNT(*) into pocet_automechanikov FROM automechanik WHERE mesto = mesto_t;
    pocet_automechanikov_np := 0;
    OPEN automechanik_c;
    LOOP
        FETCH automechanik_c INTO tmp;
        EXIT WHEN automechanik_c%NOTFOUND;
        IF (tmp.mesto = mesto_t) THEN
            IF (tmp.koniec > TO_DATE(dnesny_datum,'dd-mm-yyyy')) THEN
                pocet_automechanikov_np := pocet_automechanikov_np + 1;
            END IF;
        END IF;
    END LOOP;
    percenta_vytazenosti := 100 * (pocet_automechanikov_np / pocet_automechanikov);
    DBMS_OUTPUT.put_line('Prevadzka: ' || mesto_t || ' | Vytazenost: ' || percenta_vytazenosti || '% ' || '| Pocet automechanikov: ' || pocet_automechanikov);
    CLOSE automechanik_c;
EXCEPTION
WHEN ZERO_DIVIDE THEN
    DBMS_OUTPUT.put_line('Erorr: ZERO_DIVIDE | Nieje mozne vypocitat percentualnu vytazenost pobocky!');
    Raise_Application_Error(-20410, 'Error!');
END prevadzka_zataz;
/
-- test procedury --
EXECUTE prevadzka_zataz('Nove Zamky'); 


-- Vypisu sa opravy s popisom, na ktorych sa aktualne pracuje
CREATE OR REPLACE PROCEDURE aktivne_opravy
IS
    CURSOR oprava_c IS SELECT * FROM oprava;
    tmp oprava_c%ROWTYPE;
    dnesny_datum VARCHAR2(20);
BEGIN
    SELECT TO_CHAR(Sysdate, 'DD-MM-YYYY') into dnesny_datum FROM Dual; 
    OPEN oprava_c;
    LOOP
        FETCH oprava_c INTO tmp;
        EXIT WHEN oprava_c%NOTFOUND;
        IF (tmp.koniec > TO_DATE(dnesny_datum,'dd-mm-yyyy')) THEN
            DBMS_OUTPUT.put_line('OPRAVA: ' || tmp.nazov || '| POPIS: ' || tmp.popis);
        END IF;
    END LOOP;
    CLOSE oprava_c;
EXCEPTION
WHEN OTHERS THEN
    Raise_Application_Error(-20410, 'Error!');
END aktivne_opravy;
/
-- test procedury --
EXECUTE aktivne_opravy();        

-----------------------------------------------------
--- EXPLAIN PLAN & INDEX ---                        - 
-----------------------------------------------------
EXPLAIN PLAN FOR
    SELECT O.nazov, O.popis, V.vin, SUM(M.cena_za_kus) AS naklady
    FROM oprava_meterial NATURAL JOIN material M NATURAL JOIN oprava O NATURAL JOIN objednavka NATURAL JOIN vozidlo V
    WHERE zaciatok BETWEEN '01-06-2018' AND '30-06-2018' 
    GROUP BY O.nazov, O.popis, V.vin
    HAVING SUM(M.cena_za_kus) > 150
    ORDER BY naklady;

SELECT * FROM TABLE(DBMS_XPLAN.display);

-- index --
CREATE INDEX oprava_info_I ON oprava(zaciatok);

EXPLAIN PLAN FOR
    SELECT O.nazov, O.popis, V.vin, SUM(M.cena_za_kus) AS naklady
    FROM oprava_meterial NATURAL JOIN material M NATURAL JOIN oprava O NATURAL JOIN objednavka NATURAL JOIN vozidlo V
    WHERE zaciatok BETWEEN '01-06-2018' AND '30-06-2018' 
    GROUP BY O.nazov, O.popis, V.vin
    HAVING SUM(M.cena_za_kus) > 150
    ORDER BY naklady;

SELECT * FROM TABLE(DBMS_XPLAN.display);

-----------------------------------------------------
--- PRIRADENIE PRAV ---                             - 
-----------------------------------------------------
GRANT SELECT ON klient TO xsporn01;
GRANT SELECT ON automechanik TO xsporn01;
GRANT SELECT ON vozidlo TO xsporn01;
GRANT SELECT ON objednavka TO xsporn01;
GRANT SELECT ON vyrobca TO xsporn01;
GRANT SELECT ON vyrobca_dodavatel TO xsporn01;
GRANT SELECT ON dodavatel TO xsporn01;
GRANT SELECT ON faktura TO xsporn01;
GRANT ALL ON oprava TO xsporn01;
GRANT ALL ON oprava_meterial TO xsporn01;
GRANT ALL ON material TO xsporn01;

GRANT EXECUTE ON aktivne_opravy TO xsporn01;

-----------------------------------------------------
--- MATERIALIZED VIEW ---                           - 
-----------------------------------------------------
CREATE MATERIALIZED VIEW LOG ON objednavka WITH PRIMARY KEY, ROWID(id_automechanik) INCLUDING NEW VALUES;              

CREATE VIEW automechanik_zataz AS
    SELECT id_automechanik, COUNT(id_automechanik) AS Priradene_Objednavky
    FROM objednavka NATURAL JOIN automechanik
    GROUP BY id_automechanik;
        
CREATE MATERIALIZED VIEW automechanik_zataz_materialized
    NOLOGGING
    CACHE
    BUILD IMMEDIATE
    REFRESH FAST ON COMMIT
    ENABLE QUERY REWRITE
    AS
        SELECT id_automechanik, COUNT(id_automechanik) AS Priradene_Objednavky
        FROM objednavka NATURAL JOIN automechanik
        GROUP BY id_automechanik;
         
GRANT ALL ON automechanik_zataz_materialized TO xsporn01;

SELECT * FROM automechanik_zataz;  
SELECT * FROM automechanik_zataz_materialized;  

INSERT INTO objednavka (id_vozidlo, id_automechanik, datum_prijatia, datum_vratenia_vozidla) VALUES('1','2', TO_DATE('11-03-2017','dd-mm-yyyy'),TO_DATE('20-04-2017', 'dd-mm-yyyy'));        
      
SELECT * FROM automechanik_zataz;  
SELECT * FROM automechanik_zataz_materialized;               
      
-----------------------------------------------------
--- TRIGGERS DEMONSTRATION ---                      - 
-----------------------------------------------------      
INSERT INTO klient(rodne_cislo, meno, priezvisko, adresa, mesto, ZIP_kod, tel_cislo, email) 
    VALUES('0901961111', 'Peter', 'Petro', 'Petrovska 42', 'Bratislava', '83101', '0924292842', 'peter.petro@gmail.com');      

SELECT * FROM klient;      
      
INSERT INTO oprava(id_objednavka, nazov, popis, zaciatok, koniec) VALUES('5', 'Vymena oleja', 'Servisna prehliadka-vymena oleja po 15000km', TO_DATE('02-05-2019','dd-mm-yyyy'),TO_DATE('14-04-2019', 'dd-mm-yyyy'));
      
SELECT * FROM oprava;      
    
        