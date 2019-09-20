#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX 101
#define AlphabetLength 27
#define MaxNumberOfStrings 42

/*
* Deklaracia funkcii
*/
void ArrayToZero();
void DupliciRemove(char *DuplicatedItem);
void BubbleSort(char *VirtualnaKlavesnica);
int NullArg(char *Mesto, char *VirtualnaKlavesnica);
void StringToUpper(char *StringToUpper);
int  ArgcMoreThanTwo(int argument);
int  OutOfSpace(int counter);
int  NotAllowedChar(char *Buffer);
//int  MaxStrings(int StringCounter);

int main(int argc, char* argv[])
{
    if (ArgcMoreThanTwo(argc))
    {
       return 0;
    }

    char Mesto[MAX] = {0};
    char VirtualnaKlavesnica[AlphabetLength] = {0};
    char ZhodaMesta[AlphabetLength] = {0};
    char znak;
    int counter = 0;
    int StringCounter = 0;
    int i=0;
    int k=0;

    if (argc == 1)
    {
      if(NullArg(Mesto, VirtualnaKlavesnica)==1)
      {
        return 1;
      }
      DupliciRemove(VirtualnaKlavesnica);
      BubbleSort(VirtualnaKlavesnica);
      printf("Enable: %s\n", VirtualnaKlavesnica);

      return 0;
    }
    else
    {
      StringToUpper(argv[1]);

      while ((znak = toupper(getchar()))!=EOF)      //do znaku sa ulozi cely buffer
      {
        counter++;
        if (OutOfSpace(counter))
        {
          return 0;
        }

        if (znak == '\n')        //ak narazi kompilator na koniec slova tak vykona podmienku
        {

          /*
          * Zavola sa funkcia MaxStrings() na kontrolu poctu vstupov
          */
          StringCounter++;
          if (StringCounter > MaxNumberOfStrings)
          {
            fprintf(stderr, "Nepovolene mnozstvo vstupov!\n");
            return 1;
          }

          /*
          * Zavola sa funckia NotAllowedChar na kontrolu nepovolenych znakov
          */
          if(NotAllowedChar(Mesto))
          {
            return 1;
          }
          counter = 0;
          if (strncmp(Mesto, argv[1],strlen(argv[1])) == 0)   // porovaju sa prve  2 znaky (Brno a Br)
          {
            VirtualnaKlavesnica[k] = Mesto[strlen(argv[1])];  //ak to je tak tak sa do virtualnej klavesnice na 0. znak zapise 2. prvok z pola v tomto pripade to bude n
            k++;
            strcpy(ZhodaMesta,Mesto);

          }
          i = 0;

          ArrayToZero(Mesto);

          continue;
        }

        Mesto[i] = znak;      //do pola sa zapise Brno
        i++;
      }
    }

        if (k == 0)
        {
          printf("Not found\n");
        }

        else if (k == 1)
        {
          printf("Found: %s\n", ZhodaMesta);
        }
        else
        {
          DupliciRemove(VirtualnaKlavesnica);
          BubbleSort(VirtualnaKlavesnica);
          printf("Enable: %s\n",VirtualnaKlavesnica);
        }

    return 0;
}
/*
* Funkcia, ktora sluzi no zoradenie znakov podla abecedy
*/
void BubbleSort(char *VirtualnaKlavesnica)
{
  int dlzkaVypisu = strlen(VirtualnaKlavesnica);
  int j, temp = 0, i = 0;

    for(i=1;i<dlzkaVypisu;i++)
    {
      for(j=0;j<(dlzkaVypisu-i);j++)
      {
        if(VirtualnaKlavesnica[j]>VirtualnaKlavesnica[j+1])
            {
                temp=VirtualnaKlavesnica[j];
                VirtualnaKlavesnica[j]=VirtualnaKlavesnica[j+1];
                VirtualnaKlavesnica[j+1]=temp;
            }
      }

    }
}

/*
* Funkcia, ktora sluzi na vymazanie duplicitnych znakov z vypisu pola
*/
void DupliciRemove(char *DuplicatedItem)
{
  int i = 0, j = 0, k = 0, size = 0;
  for(i=0; i<(int)strlen(DuplicatedItem); i++)
    {
        for(j=i+1; j<(int)strlen(DuplicatedItem); j++)
        {
            /* Ak najde nejaky duplikat*/
            if(DuplicatedItem[i] == DuplicatedItem[j])
            {
                /* Vymaz aktualny duplicitny element */
                for(k=j; k<(int)strlen(DuplicatedItem); k++)
                {
                    DuplicatedItem[k] = DuplicatedItem[k + 1];
                }

                /* Decrementuj velkost po odstraneni duplicitneho elementu */
                size--;

                /* If shifting of elements occur then don't increment j */
                j--;
            }
        }
    }
}
/*
* Funkcia, ktora sluzi na prevedenie maleho pismena na velke
*/
void StringToUpper(char *StringToUpper)
{
  for(int i = 0; i <= (int)strlen(StringToUpper); i++)
    {
      StringToUpper[i] = toupper(StringToUpper[i]);
    }
}

/*
* Funkcia, ktora sluzi na vynulovanie pola
*/
void ArrayToZero(char *ArrayToZero)
{
  for (int i = 0; ArrayToZero[i] != '\0'; i++)
        {
          ArrayToZero[i] = '\0';
        }
}

/*
* Funkcia, ktora sluzi na vypisanie prveho znaku zo zadaneho slova
*/
 int NullArg(char *Mesto, char *VirtualnaKlavesnica)
 {
    int g = 0, znak, i=0;
    while((znak = getchar())!=EOF)
    {
      if(znak == '\n')
      {
        /*
        * Zavola sa funckia NotAllowedChar na kontrolu nepovolenych znakov
        */
        if(NotAllowedChar(Mesto))
        {
          return 1;
        }
        VirtualnaKlavesnica[g] = Mesto[0];
        g++;
        i = 0;
        ArrayToZero(Mesto);
      }

      else
      {
        Mesto[i] = (char)toupper(znak);
        i++;
      }

    }
    return 0;
 }
 /*
 * Funkcia zavola stderr ak uzivatel zada viac argumentov
 */
 int ArgcMoreThanTwo(int argument)
 {
   if(argument > 2)
   {
     fprintf(stderr,"Zadali ste %d argumenty to je nepovolene mnozstvo\n!",argument-1);
     return 1;
   }
   return 0;
 }
/*
* Funkcia zavola stderr ak je nazov mesta priliz dlhy
*/
 int OutOfSpace(int counter)
 {
   if (counter > (MAX-1))
   {
     fprintf(stderr,"Nazov mesta je priliz dlhy!\n");
     return 1;
   }
   return 0;
 }

int NotAllowedChar(char *Buffer)
{
      for (int i = 0; i < (int)strlen(Buffer);i++)
      {
        if (isalpha(Buffer[i])==0)
        {
          fprintf(stderr,"Zadali ste nepovoleny znak!\n");
          return 1;
        }
      }
      return 0;
}

