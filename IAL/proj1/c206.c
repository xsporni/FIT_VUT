//Vypracoval Alex Sporni, xsporn01@stud.fit.vutbr.cz
/* c206.c **********************************************************}
{* Téma: Dvousměrně vázaný lineární seznam
**
**                   Návrh a referenční implementace: Bohuslav Křena, říjen 2001
**                            Přepracované do jazyka C: Martin Tuček, říjen 2004
**                                            Úpravy: Kamil Jeřábek, září 2018
**
** Implementujte abstraktní datový typ dvousměrně vázaný lineární seznam.
** Užitečným obsahem prvku seznamu je hodnota typu int.
** Seznam bude jako datová abstrakce reprezentován proměnnou
** typu tDLList (DL znamená Double-Linked a slouží pro odlišení
** jmen konstant, typů a funkcí od jmen u jednosměrně vázaného lineárního
** seznamu). Definici konstant a typů naleznete v hlavičkovém souboru c206.h.
**
** Vaším úkolem je implementovat následující operace, které spolu
** s výše uvedenou datovou částí abstrakce tvoří abstraktní datový typ
** obousměrně vázaný lineární seznam:
**
**      DLInitList ...... inicializace seznamu před prvním použitím,
**      DLDisposeList ... zrušení všech prvků seznamu,
**      DLInsertFirst ... vložení prvku na začátek seznamu,
**      DLInsertLast .... vložení prvku na konec seznamu,
**      DLFirst ......... nastavení aktivity na první prvek,
**      DLLast .......... nastavení aktivity na poslední prvek,
**      DLCopyFirst ..... vrací hodnotu prvního prvku,
**      DLCopyLast ...... vrací hodnotu posledního prvku,
**      DLDeleteFirst ... zruší první prvek seznamu,
**      DLDeleteLast .... zruší poslední prvek seznamu,
**      DLPostDelete .... ruší prvek za aktivním prvkem,
**      DLPreDelete ..... ruší prvek před aktivním prvkem,
**      DLPostInsert .... vloží nový prvek za aktivní prvek seznamu,
**      DLPreInsert ..... vloží nový prvek před aktivní prvek seznamu,
**      DLCopy .......... vrací hodnotu aktivního prvku,
**      DLActualize ..... přepíše obsah aktivního prvku novou hodnotou,
**      DLSucc .......... posune aktivitu na další prvek seznamu,
**      DLPred .......... posune aktivitu na předchozí prvek seznamu,
**      DLActive ........ zjišťuje aktivitu seznamu.
**
** Při implementaci jednotlivých funkcí nevolejte žádnou z funkcí
** implementovaných v rámci tohoto příkladu, není-li u funkce
** explicitně uvedeno něco jiného.
**
** Nemusíte ošetřovat situaci, kdy místo legálního ukazatele na seznam 
** předá někdo jako parametr hodnotu NULL.
**
** Svou implementaci vhodně komentujte!
**
** Terminologická poznámka: Jazyk C nepoužívá pojem procedura.
** Proto zde používáme pojem funkce i pro operace, které by byly
** v algoritmickém jazyce Pascalovského typu implemenovány jako
** procedury (v jazyce C procedurám odpovídají funkce vracející typ void).
**/

#include "c206.h"

int errflg;
int solved;

void DLError() {
/*
** Vytiskne upozornění na to, že došlo k chybě.
** Tato funkce bude volána z některých dále implementovaných operací.
**/	
    printf ("*ERROR* The program has performed an illegal operation.\n");
    errflg = TRUE;             /* globální proměnná -- příznak ošetření chyby */
    return;
}

void DLInitList (tDLList *L) {
/*
** Provede inicializaci seznamu L před jeho prvním použitím (tzn. žádná
** z následujících funkcí nebude volána nad neinicializovaným seznamem).
** Tato inicializace se nikdy nebude provádět nad již inicializovaným
** seznamem, a proto tuto možnost neošetřujte. Vždy předpokládejte,
** že neinicializované proměnné mají nedefinovanou hodnotu.
**/

    //inicializacia zoznamu
    L->Act = NULL;
    L->Last = NULL;
    L->First = NULL;
    
	
 //solved = FALSE;                   /* V případě řešení, smažte tento řádek! */
}

void DLDisposeList (tDLList *L) {
/*
** Zruší všechny prvky seznamu L a uvede seznam do stavu, v jakém
** se nacházel po inicializaci. Rušené prvky seznamu budou korektně
** uvolněny voláním operace free. 
**/
     tDLElemPtr polozka;
     L->First = L->Act;
     // budem cyklit pokial sa na prvom prvku v zozname bude nachadzat prvok
     while(L->First != NULL)
     {
         polozka = L->First;
         L->First = polozka->rptr;
         free(polozka); //uvolnil som prvy prvok zoznamu
     }
     L->First = NULL;
     L->Last = NULL;
     L->Act = NULL;
	
	
 //solved = FALSE;                   /* V případě řešení, smažte tento řádek! */
}

void DLInsertFirst (tDLList *L, int val) {
/*
** Vloží nový prvek na začátek seznamu L.
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/
    tDLElemPtr polozka = malloc(sizeof(struct tDLElem));
    if (polozka == NULL)
    {
        DLError();
        return;
    }

    polozka->data = val;
    polozka->lptr = NULL;
    polozka->rptr = L->First;

    if (L->First != NULL)
    {
        L->First->lptr = polozka;
    }
    else
    {
        L->Last = polozka;
    }
    L->First = polozka;
	
 //solved = FALSE;                   /* V případě řešení, smažte tento řádek! */
}

void DLInsertLast(tDLList *L, int val) {
/*
** Vloží nový prvek na konec seznamu L (symetrická operace k DLInsertFirst).
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/ 
    tDLElemPtr polozka = malloc(sizeof(struct tDLElem));
    if (polozka == NULL)
    {
        DLError();
        return;
    }

    polozka->data = val;
    polozka->lptr = L->Last;
    polozka->rptr = NULL;

    if (L->Last != NULL)
    {
        L->Last->rptr = polozka;
    }
    else
    {
        L->First = polozka;
    }
    L->Last = polozka;
	
	
 //solved = FALSE;                   /* V případě řešení, smažte tento řádek! */
}

void DLFirst (tDLList *L) {
/*
** Nastaví aktivitu na první prvek seznamu L.
** Funkci implementujte jako jediný příkaz (nepočítáme-li return),
** aniž byste testovali, zda je seznam L prázdný.
**/
	
   L->Act = L->First;
 //solved = FALSE;                   /* V případě řešení, smažte tento řádek! */
}

void DLLast (tDLList *L) {
/*
** Nastaví aktivitu na poslední prvek seznamu L.
** Funkci implementujte jako jediný příkaz (nepočítáme-li return),
** aniž byste testovali, zda je seznam L prázdný.
**/
	
	L->Act = L->Last;
 //solved = FALSE;                   /* V případě řešení, smažte tento řádek! */
}

void DLCopyFirst (tDLList *L, int *val) {
/*
** Prostřednictvím parametru val vrátí hodnotu prvního prvku seznamu L.
** Pokud je seznam L prázdný, volá funkci DLError().
**/
    if (L->First == NULL)
    {
        DLError();
        return;
    }
    else
    {
         *val = L->First->data;
    }

 //solved = FALSE;                   /* V případě řešení, smažte tento řádek! */
}

void DLCopyLast (tDLList *L, int *val) {
/*
** Prostřednictvím parametru val vrátí hodnotu posledního prvku seznamu L.
** Pokud je seznam L prázdný, volá funkci DLError().
**/
if (L->First == NULL)
    {
        DLError();
        return;
    }
    else
    {
         *val = L->Last->data;
    }
	
	
 //solved = FALSE;                   /* V případě řešení, smažte tento řádek! */
}

void DLDeleteFirst (tDLList *L) {
/*
** Zruší první prvek seznamu L. Pokud byl první prvek aktivní, aktivita 
** se ztrácí. Pokud byl seznam L prázdný, nic se neděje.
**/

    tDLElemPtr polozka;
    if (L->First == NULL)
    {
        return;
    }

    if (L->First != NULL)
    {
        polozka = L->First;
    
        if (L->First == L->Act)
	    {
            L->Act = NULL;
        }

        if (L->First == L->Last)
        {
            L->Last = NULL;
            L->First = NULL;
        }
        else
        {
            L->First = L->First->rptr;
            L->First->lptr = NULL;
        }
        free(polozka);
    }

	
 //solved = FALSE;                   /* V případě řešení, smažte tento řádek! */
}	

void DLDeleteLast (tDLList *L) {
/*
** Zruší poslední prvek seznamu L. Pokud byl poslední prvek aktivní,
** aktivita seznamu se ztrácí. Pokud byl seznam L prázdný, nic se neděje.
**/ 
    tDLElemPtr polozka;
    if (L->First == NULL)
    {
        return; //Pokial je zoznam prazdny nic sa neudeje
    }

    if (L->First != NULL)
    {
        polozka = L->Last;
    
        if (L->Last == L->Act)
	    {
            L->Act = NULL; //Ak je moj posledny prvok v zozname aktivny, aktivita sa straca
        }

        if (L->First == L->Last)
        {
            L->Last = NULL; //Ak v pojom zozname je len jeden prvok, ktory je potom zaroven prvym aj poslednym, nastavim oba ukazatele na NULL
            L->First = NULL;
        }
        else
        {
            L->Last = L->Last->lptr; //
            L->Last->rptr = NULL; //nastavil som ukazatel na nasledujuci prvok na NULL
        }
        free(polozka);
    }
	
 //solved = FALSE;                   /* V případě řešení, smažte tento řádek! */
}

void DLPostDelete (tDLList *L) {
/*
** Zruší prvek seznamu L za aktivním prvkem.
** Pokud je seznam L neaktivní nebo pokud je aktivní prvek
** posledním prvkem seznamu, nic se neděje.
**/

    if (L->Act == NULL || L->Last == L->Act)
    {
        return;
    }
    
    // Vytvoril som si 2 premenne, do prem DELL_ITEM ulozim prvok ktory budem mazat
    // Do premennej NEXT_DELL_ITEM ulozim prvok ktory bude nasledovat za mazanym
    tDLElemPtr DELL_ITEM;
    tDLElemPtr NEXT_DELL_ITEM;

    DELL_ITEM = L->Act->rptr;
    NEXT_DELL_ITEM = L->Act->rptr->rptr;

    // osetrim stav ak je mazany prvok poslednym prvkom v zozname

    if (DELL_ITEM == L->Last)
    {
        // Pokial moj mazany prvok je poslednym prvkom v zozname,
        // Tak dalej uz nie je mozne mazat, tym padom moj posledny prvok
        // nastavim ako aktivny, a jeho ukazatel na nasledujuci prvok v zozname
        // nastavim na NULL
        L->Last = L->Act; 
        L->Last->rptr = NULL;
    }
    else //Ak sa moj mazany prvok bude nachadzat v strede zoznamu, musim zabezpecit previazanost zoznamu
    {
        // Ukazatel na nasledujuci prvok za aktivnym nastavim na nasledujuci prvok za mazanym
        L->Act->rptr = NEXT_DELL_ITEM; 
        DELL_ITEM->rptr->lptr = L->Act;
    }
    free(DELL_ITEM);
		
 //solved = FALSE;                   /* V případě řešení, smažte tento řádek! */
}

void DLPreDelete (tDLList *L) {
/*
** Zruší prvek před aktivním prvkem seznamu L .
** Pokud je seznam L neaktivní nebo pokud je aktivní prvek
** prvním prvkem seznamu, nic se neděje.
**/
if (L->Act == NULL || L->First == L->Act)
    {
        return;
    }

    //DELL_ITEM bude prvok ktory budem mazat a PRE_DELL_ITEM bude prvok pred nim
    tDLElemPtr DELL_ITEM;
    tDLElemPtr PRE_DELL_ITEM;

    DELL_ITEM = L->Act->lptr;
    PRE_DELL_ITEM = L->Act->lptr->lptr;


    if (DELL_ITEM == L->First)
    {
        L->First = L->Act; 
        L->First->lptr = NULL;
    }
    else 
    {
        L->Act->lptr = PRE_DELL_ITEM; 
        DELL_ITEM->lptr->rptr = L->Act;
    }
    free(DELL_ITEM);
	
			
//solved = FALSE;                   /* V případě řešení, smažte tento řádek! */
}

void DLPostInsert (tDLList *L, int val) {
/*
** Vloží prvek za aktivní prvek seznamu L.
** Pokud nebyl seznam L aktivní, nic se neděje.
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/
    //Ak nie je zoznam aktivny, nic sa nedeje
	if (L->Act == NULL)
    {
        return;
    }

    tDLElemPtr polozka = malloc(sizeof(struct tDLElem));
    //Osetril som chyby pri alokacii
    if (polozka == NULL)
    {
        DLError();
        return;
    }

    //Inicializujem
    polozka->data = val;
    polozka->lptr = L->Act;
    polozka->rptr = L->Act->rptr;
    L->Act->rptr = polozka;

    //Ak bude aktivny prvok na konci zoznamu
    if (L->Act == L->Last)
    {
        L->Last = polozka; //z novo pridaneho prvku sa stava posledny
    }
    else   //ak bude aktivny prvok v strede zoznamu
    {
        polozka->rptr->lptr = polozka;
    }

 //solved = FALSE;                   /* V případě řešení, smažte tento řádek! */
}

void DLPreInsert (tDLList *L, int val) {
/*
** Vloží prvek před aktivní prvek seznamu L.
** Pokud nebyl seznam L aktivní, nic se neděje.
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/
    if (L->Act == NULL)
    {
        return;
    }
    tDLElemPtr polozka = malloc(sizeof(struct tDLElem));
    
    if (polozka == NULL)
    {
        DLError();
        return;
    }

    //Inicializujem
    polozka->data = val;
    polozka->rptr = L->Act;
    polozka->lptr = L->Act->lptr;
    L->Act->lptr = polozka;

    //Ak bude aktivny prvok na zaciatku zoznamu
    if (L->Act == L->First)
    {
        L->First = polozka;  //z novo pridaneho prvku sa stava prvy
    }
    else   //ak bude aktivny prvok v strede zoznamu
    {
         polozka->lptr->rptr = polozka;
    }
	
	
 //solved = FALSE;                   /* V případě řešení, smažte tento řádek! */
}

void DLCopy (tDLList *L, int *val) {
/*
** Prostřednictvím parametru val vrátí hodnotu aktivního prvku seznamu L.
** Pokud seznam L není aktivní, volá funkci DLError ().
**/

    if (L->Act == NULL) //v pripade ze zoznam je neaktivny vraciam chybovu funkciu
    {
        DLError();
        return;
    }
    else
    {
        *val = L->Act->data; // vratil som hodnotu aktivneho prvku
    }
	
 //solved = FALSE;                   /* V případě řešení, smažte tento řádek! */
}

void DLActualize (tDLList *L, int val) {
/*
** Přepíše obsah aktivního prvku seznamu L.
** Pokud seznam L není aktivní, nedělá nic.
**/
    if (L->Act == NULL) 
    {
        return; //pokial nie je aktivny nic neurobi
    }
    else
    {
        L->Act->data = val; // prepisanie obsah
    }
	
	
 //solved = FALSE;                   /* V případě řešení, smažte tento řádek! */
}

void DLSucc (tDLList *L) {
/*
** Posune aktivitu na následující prvek seznamu L.
** Není-li seznam aktivní, nedělá nic.
** Všimněte si, že při aktivitě na posledním prvku se seznam stane neaktivním.
**/

     if (L->Act == NULL)
    {
        return;
    }
    else
    {
        L->Act = L->Act->rptr; // posunul som aktivitu na dalsi prvok v zozname
    }
	
	
 //solved = FALSE;                   /* V případě řešení, smažte tento řádek! */
}


void DLPred (tDLList *L) {
/*
** Posune aktivitu na předchozí prvek seznamu L.
** Není-li seznam aktivní, nedělá nic.
** Všimněte si, že při aktivitě na prvním prvku se seznam stane neaktivním.
**/
    
    if (L->Act == NULL)
    {
        return;
    }
    else
    {
        L->Act = L->Act->lptr; // posuvam aktivitu na predosli prvok
    }
	
	
 //solved = FALSE;                   /* V případě řešení, smažte tento řádek! */
}

int DLActive (tDLList *L) {
/*
** Je-li seznam L aktivní, vrací nenulovou hodnotu, jinak vrací 0.
** Funkci je vhodné implementovat jedním příkazem return.
**/
	
	return (L->Act != NULL);
 //solved = FALSE;                   /* V případě řešení, smažte tento řádek! */
}

/* Konec c206.c*/
