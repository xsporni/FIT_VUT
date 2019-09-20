
/* c402.c: ********************************************************************}
{* Téma: Nerekurzivní implementace operací nad BVS 
**                                     Implementace: Petr Přikryl, prosinec 1994
**                                           Úpravy: Petr Přikryl, listopad 1997
**                                                     Petr Přikryl, květen 1998
**			  	                        Převod do jazyka C: Martin Tuček, srpen 2005
**                                         Úpravy: Bohuslav Křena, listopad 2009
**                                                 Karel Masařík, říjen 2013
**                                                 Radek Hranický 2014-2018
**
** S využitím dynamického přidělování paměti, implementujte NEREKURZIVNĚ
** následující operace nad binárním vyhledávacím stromem (předpona BT znamená
** Binary Tree a je u identifikátorů uvedena kvůli možné kolizi s ostatními
** příklady):
**
**     BTInit .......... inicializace stromu
**     BTInsert ........ nerekurzivní vložení nového uzlu do stromu
**     BTPreorder ...... nerekurzivní průchod typu pre-order
**     BTInorder ....... nerekurzivní průchod typu in-order
**     BTPostorder ..... nerekurzivní průchod typu post-order
**     BTDisposeTree ... zruš všechny uzly stromu
**
** U všech funkcí, které využívají některý z průchodů stromem, implementujte
** pomocnou funkci pro nalezení nejlevějšího uzlu v podstromu.
**
** Přesné definice typů naleznete v souboru c402.h. Uzel stromu je typu tBTNode,
** ukazatel na něj je typu tBTNodePtr. Jeden uzel obsahuje položku int Cont,
** která současně slouží jako užitečný obsah i jako vyhledávací klíč 
** a ukazatele na levý a pravý podstrom (LPtr a RPtr).
**
** Příklad slouží zejména k procvičení nerekurzivních zápisů algoritmů
** nad stromy. Než začnete tento příklad řešit, prostudujte si důkladně
** principy převodu rekurzivních algoritmů na nerekurzivní. Programování
** je především inženýrská disciplína, kde opětné objevování Ameriky nemá
** místo. Pokud se Vám zdá, že by něco šlo zapsat optimálněji, promyslete
** si všechny detaily Vašeho řešení. Povšimněte si typického umístění akcí
** pro různé typy průchodů. Zamyslete se nad modifikací řešených algoritmů
** například pro výpočet počtu uzlů stromu, počtu listů stromu, výšky stromu
** nebo pro vytvoření zrcadlového obrazu stromu (pouze popřehazování ukazatelů
** bez vytváření nových uzlů a rušení starých).
**
** Při průchodech stromem použijte ke zpracování uzlu funkci BTWorkOut().
** Pro zjednodušení práce máte předem připraveny zásobníky pro hodnoty typu
** bool a tBTNodePtr. Pomocnou funkci BTWorkOut ani funkce pro práci
** s pomocnými zásobníky neupravujte 
** Pozor! Je třeba správně rozlišovat, kdy použít dereferenční operátor *
** (typicky při modifikaci) a kdy budeme pracovat pouze se samotným ukazatelem 
** (např. při vyhledávání). V tomto příkladu vám napoví prototypy funkcí.
** Pokud pracujeme s ukazatelem na ukazatel, použijeme dereferenci.
**/

#include "c402.h"
int solved;

void BTWorkOut (tBTNodePtr Ptr)		{
/*   ---------
** Pomocná funkce, kterou budete volat při průchodech stromem pro zpracování
** uzlu určeného ukazatelem Ptr. Tuto funkci neupravujte.
**/
			
	if (Ptr==NULL) 
    printf("Chyba: Funkce BTWorkOut byla volána s NULL argumentem!\n");
  else 
    printf("Výpis hodnoty daného uzlu> %d\n",Ptr->Cont);
}
	
/* -------------------------------------------------------------------------- */
/*
** Funkce pro zásobník hotnot typu tBTNodePtr. Tyto funkce neupravujte.
**/

void SInitP (tStackP *S)  
/*   ------
** Inicializace zásobníku.
**/
{
	S->top = 0;  
}	

void SPushP (tStackP *S, tBTNodePtr ptr)
/*   ------
** Vloží hodnotu na vrchol zásobníku.
**/
{ 
                 /* Při implementaci v poli může dojít k přetečení zásobníku. */
  if (S->top==MAXSTACK) 
    printf("Chyba: Došlo k přetečení zásobníku s ukazateli!\n");
  else {  
		S->top++;  
		S->a[S->top]=ptr;
	}
}	

tBTNodePtr STopPopP (tStackP *S)
/*         --------
** Odstraní prvek z vrcholu zásobníku a současně vrátí jeho hodnotu.
**/
{
                            /* Operace nad prázdným zásobníkem způsobí chybu. */
	if (S->top==0)  {
		printf("Chyba: Došlo k podtečení zásobníku s ukazateli!\n");
		return(NULL);	
	}	
	else {
		return (S->a[S->top--]);
	}	
}

bool SEmptyP (tStackP *S)
/*   -------
** Je-li zásobník prázdný, vrátí hodnotu true.
**/
{
  return(S->top==0);
}	

/* -------------------------------------------------------------------------- */
/*
** Funkce pro zásobník hotnot typu bool. Tyto funkce neupravujte.
*/

void SInitB (tStackB *S) {
/*   ------
** Inicializace zásobníku.
**/

	S->top = 0;  
}	

void SPushB (tStackB *S,bool val) {
/*   ------
** Vloží hodnotu na vrchol zásobníku.
**/
                 /* Při implementaci v poli může dojít k přetečení zásobníku. */
	if (S->top==MAXSTACK) 
		printf("Chyba: Došlo k přetečení zásobníku pro boolean!\n");
	else {
		S->top++;  
		S->a[S->top]=val;
	}	
}

bool STopPopB (tStackB *S) {
/*   --------
** Odstraní prvek z vrcholu zásobníku a současně vrátí jeho hodnotu.
**/
                            /* Operace nad prázdným zásobníkem způsobí chybu. */
	if (S->top==0) {
		printf("Chyba: Došlo k podtečení zásobníku pro boolean!\n");
		return(NULL);	
	}	
	else {  
		return(S->a[S->top--]); 
	}	
}

bool SEmptyB (tStackB *S) {
/*   -------
** Je-li zásobník prázdný, vrátí hodnotu true.
**/
  return(S->top==0);
}

/* -------------------------------------------------------------------------- */
/*
** Následuje jádro domácí úlohy - funkce, které máte implementovat. 
*/

void BTInit (tBTNodePtr *RootPtr)	{
/*   ------
** Provede inicializaci binárního vyhledávacího stromu.
**
** Inicializaci smí programátor volat pouze před prvním použitím binárního
** stromu, protože neuvolňuje uzly neprázdného stromu (a ani to dělat nemůže,
** protože před inicializací jsou hodnoty nedefinované, tedy libovolné).
** Ke zrušení binárního stromu slouží procedura BTDisposeTree.
**	
** Všimněte si, že zde se poprvé v hlavičce objevuje typ ukazatel na ukazatel,	
** proto je třeba při práci s RootPtr použít dereferenční operátor *.
**/
	// Inicializacia
	(*RootPtr) = NULL;
	 //solved = FALSE;		  /* V případě řešení smažte tento řádek! */	
}

void BTInsert (tBTNodePtr *RootPtr, int Content) {
/*   --------
** Vloží do stromu nový uzel s hodnotou Content.
**
** Z pohledu vkládání chápejte vytvářený strom jako binární vyhledávací strom,
** kde uzly s hodnotou menší než má otec leží v levém podstromu a uzly větší
** leží vpravo. Pokud vkládaný uzel již existuje, neprovádí se nic (daná hodnota
** se ve stromu může vyskytnout nejvýše jednou). Pokud se vytváří nový uzel,
** vzniká vždy jako list stromu. Funkci implementujte nerekurzivně.
**/

	// Ak vkladany uzol este neexistuje
	if ((*RootPtr) == NULL)
	{
		(*RootPtr) = (tBTNodePtr)malloc(sizeof(struct tBTNode));
		if ((*RootPtr) == NULL)
		{
			return;
		}
		(*RootPtr)->Cont = Content;
		(*RootPtr)->LPtr = NULL;
		(*RootPtr)->RPtr = NULL;


	}
	else
	{
		tBTNodePtr polozka = (*RootPtr);
		tBTNodePtr polozka2 = (*RootPtr);
		while (polozka != NULL)
		{
			polozka2 = polozka;
			if (Content > polozka->Cont)
			{
				polozka = polozka->RPtr;
			}
			else if (Content < polozka->Cont)
			{
				polozka = polozka->LPtr;
			}
			else
			{
				polozka = NULL;
				polozka2 = NULL;
			}
		}
		if (polozka2 != NULL)
		{
			tBTNodePtr uzol = (tBTNodePtr)malloc(sizeof(struct tBTNode));
			if (uzol == NULL)
			{
				return;
			}
			uzol->LPtr = NULL;
			uzol->RPtr = NULL;
			uzol->Cont = Content;
			if (Content > polozka2->Cont)
			{
				polozka2->RPtr = uzol;
			}
			else if (Content < polozka2->Cont)
			{
				polozka2->LPtr = uzol;
			}
		}
	}	
	 //solved = FALSE;		  /* V případě řešení smažte tento řádek! */	
}

/*                                  PREORDER                                  */

void Leftmost_Preorder (tBTNodePtr ptr, tStackP *Stack)	{
/*   -----------------
** Jde po levě větvi podstromu, dokud nenarazí na jeho nejlevější uzel.
**
** Při průchodu Preorder navštívené uzly zpracujeme voláním funkce BTWorkOut()
** a ukazatele na ně is uložíme do zásobníku.
**/

	while (ptr != NULL)
	{
		BTWorkOut(ptr); // spracovanie uzla
		SPushP(Stack, ptr); // ulozil som si uzol na neskorsi prechod do prava na zasobnik
		ptr = ptr->LPtr; // idem do lavej strany uzla
	}
	
	 //solved = FALSE;		  /* V případě řešení smažte tento řádek! */	
}

void BTPreorder (tBTNodePtr RootPtr)	{
/*   ----------
** Průchod stromem typu preorder implementovaný nerekurzivně s využitím funkce
** Leftmost_Preorder a zásobníku ukazatelů. Zpracování jednoho uzlu stromu
** realizujte jako volání funkce BTWorkOut(). 
**/
	// Ak neexistuje uzol... tak chyba
	if ((RootPtr) == NULL)
	{
		return;
	}
	else
	{
		// Vytvoril som si a inicializoval zasobnik
		tStackP zasobnik;
		SInitP(&zasobnik);
		Leftmost_Preorder(RootPtr, &zasobnik); // Presiel som strom a dal som na vrch zasobniku vsetky lave uzly
		tBTNodePtr polozka;
		while (!SEmptyP(&zasobnik))
		{
			polozka = STopPopP(&zasobnik); // odobral som z vrcholu zasobinka uzol
			Leftmost_Preorder(polozka->RPtr, &zasobnik);
		}
	}
	
	
	 //solved = FALSE;		  /* V případě řešení smažte tento řádek! */	
}


/*                                  INORDER                                   */ 

void Leftmost_Inorder(tBTNodePtr ptr, tStackP *Stack)		{
/*   ----------------
** Jde po levě větvi podstromu, dokud nenarazí na jeho nejlevější uzel.
**
** Při průchodu Inorder ukládáme ukazatele na všechny navštívené uzly do
** zásobníku. 
**/
	while (ptr != NULL)
	{
		SPushP(Stack, ptr);
		ptr = ptr->LPtr;
	}
	
	
	
	 //solved = FALSE;		  /* V případě řešení smažte tento řádek! */	
	
}

void BTInorder (tBTNodePtr RootPtr)	{
/*   ---------
** Průchod stromem typu inorder implementovaný nerekurzivně s využitím funkce
** Leftmost_Inorder a zásobníku ukazatelů. Zpracování jednoho uzlu stromu
** realizujte jako volání funkce BTWorkOut(). 
**/
	if ((RootPtr) == NULL)
	{
		return;
	}
	else
	{
		// Vytvoril som si a inicializoval zasobnik
		tStackP zasobnik;
		SInitP(&zasobnik);
		Leftmost_Inorder(RootPtr, &zasobnik); // Presiel som strom a dal som na vrch zasobniku vsetky lave uzly
		tBTNodePtr polozka;
		while (!SEmptyP(&zasobnik))
		{
			polozka = STopPopP(&zasobnik); // odobral som z vrcholu zasobinka uzol
			BTWorkOut(polozka);
			Leftmost_Inorder(polozka->RPtr, &zasobnik);
		}
	}

	
	
	 //solved = FALSE;		  /* V případě řešení smažte tento řádek! */	
}

/*                                 POSTORDER                                  */ 

void Leftmost_Postorder (tBTNodePtr ptr, tStackP *StackP, tStackB *StackB) {
/*           --------
** Jde po levě větvi podstromu, dokud nenarazí na jeho nejlevější uzel.
**
** Při průchodu Postorder ukládáme ukazatele na navštívené uzly do zásobníku
** a současně do zásobníku bool hodnot ukládáme informaci, zda byl uzel
** navštíven poprvé a že se tedy ještě nemá zpracovávat. 
**/
	while (ptr != NULL)
	{
		SPushP(StackP, ptr);
		SPushB(StackB, TRUE);
		ptr = ptr->LPtr;
	}

	 //solved = FALSE;		  /* V případě řešení smažte tento řádek! */	
}

void BTPostorder (tBTNodePtr RootPtr)	{
/*           -----------
** Průchod stromem typu postorder implementovaný nerekurzivně s využitím funkce
** Leftmost_Postorder, zásobníku ukazatelů a zásobníku hotdnot typu bool.
** Zpracování jednoho uzlu stromu realizujte jako volání funkce BTWorkOut(). 
**/
	if ((RootPtr) == NULL)
	{
		return;
	}
	else
	{
		// Vytvaranie a inicializacia
		tStackP zasobik;
		tStackB zasobik2;
		tBTNodePtr polozka = RootPtr;
		SInitP(&zasobik);
		SInitB(&zasobik2);
		bool lava_strana;
		Leftmost_Postorder(polozka, &zasobik, &zasobik2);

		while(!SEmptyP(&zasobik))
		{
			polozka = STopPopP(&zasobik);
			SPushP(&zasobik, polozka);
			lava_strana = STopPopB(&zasobik2);

			if (lava_strana == TRUE)
			{
				SPushB(&zasobik2, FALSE);
				Leftmost_Postorder(polozka->RPtr, &zasobik, &zasobik2);
			}
			else
			{
				STopPopP(&zasobik);
				BTWorkOut(polozka);
			}
		}
	}
	
		
	 //solved = FALSE;		  /* V případě řešení smažte tento řádek! */	
}


void BTDisposeTree (tBTNodePtr *RootPtr)	{
/*   -------------
** Zruší všechny uzly stromu a korektně uvolní jimi zabranou paměť.
**
** Funkci implementujte nerekurzivně s využitím zásobníku ukazatelů.
**/

	if ((*RootPtr) != NULL)
	{
		// Vytvorenie a inicializacia
		tStackP zasobnik;
		SInitP(&zasobnik);
		tBTNodePtr polozka;
		do {
			// Ak neexistuje koren tak...
			if ((*RootPtr) == NULL)
			{
				if(!SEmptyP(&zasobnik))
				{
					(*RootPtr) = STopPopP(&zasobnik);
				}
			}
			else
			{
				// Ak sa mi v pravej vetve nic nenachadza...
				if((*RootPtr)->RPtr != NULL)
				{	
					SPushP(&zasobnik,(*RootPtr)->RPtr);
				}

				polozka = (*RootPtr);
				(*RootPtr) = (*RootPtr)->LPtr;
				free(polozka);
			}

		} while (((*RootPtr) != NULL) || (!SEmptyP(&zasobnik)));
	}

	
	 //solved = FALSE;		  /* V případě řešení smažte tento řádek! */	
}

/* konec c402.c */

