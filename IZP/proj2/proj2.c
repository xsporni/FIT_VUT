/*
* Alex Sporni - xsporn01
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

/*
* Deklaracia symbolickych konstant
*/
#define IMPLICITNA_VYSKA 1.5
#define EPS 0.1e-10
#define DOLNA_HRANICA 0
#define MAX_UHOL 1.4
#define MAX_VYSKA_PRISTROJA 100

/*
* Deklaracia prototypov funkcii
*/
double taylor_tan(double x, unsigned int n);
double cfrac_tan(double x, unsigned int n);
double dlzkaD(double x, double c);
double vyskaV(double x, double d, double c);
void napoveda();
int intervalErr(double x);
int VyskaPristrojaErr(double x);

int main(int argc, char *argv[])
{
  double d;
  double v;
  char *NotANummber = 0;

        if (argc == 1 || argc > 6)
        {
          fprintf(stderr, "Zly pocet argumentov!\n");
          return 1;
        }
        if (strcmp(argv[1], "--help") == 0)
        {
          napoveda();
          return 0;
        }
        else if (strcmp(argv[1], "--tan") == 0 && argc > 4)
        {
          // N = dolna hranica intervalu
          // M = horna hranica intervalu
          int N = strtol(argv[3], &NotANummber, 10);
          if (*NotANummber != '\0')
          {
            fprintf(stderr,"Chyba vstupu!\n");
            return 1;
          }
          int M = strtol(argv[4], &NotANummber, 10);
          if (*NotANummber != '\0')
          {
            fprintf(stderr,"Chyba vstupu!\n");
            return 1;
          }
          if (N > 0 && N <= M &&  M < 14)
          {
            // x = uhol alfa v radianoch
            double x = strtod(argv[2], &NotANummber);
            if (*NotANummber != '\0')
            {
              fprintf(stderr,"Chyba vstupu!\n");
              return 1;
            }
            for (int N = strtol(argv[3], NULL, 10); N <=strtol(argv[4], NULL, 10); N++ )
            {
              printf("%d %e %e %e %e %e\n", N, tan(x), taylor_tan(x, N),  fabs(taylor_tan(x, N) - tan(x)), cfrac_tan(x, N), fabs(cfrac_tan(x,N) - tan(x)));
            }
          }
          else
          {
            fprintf(stderr,"Prekrocili ste platny rozsah!\n");
            return 1;
          }
        }
        else if (strcmp(argv[1], "-m") == 0 && argc >= 3 && argc <= 4)
        {
          //alpha == 0.3
          //beta ==0.9
          double alpha = strtod(argv[2], &NotANummber);
          if (*NotANummber != '\0')
          {
            fprintf(stderr,"Chyba vstupu!\n");
            return 1;
          }
          if (intervalErr(alpha) == 0)
          {
            d = dlzkaD(alpha, IMPLICITNA_VYSKA);
            printf("%.10e \n", d);

          }
          else
          {
            return 1;
          }

          if (argc == 4)
          {
            double beta = strtod(argv[3], &NotANummber);
            if (*NotANummber != '\0')
          {
            fprintf(stderr,"Chyba vstupu!\n");
            return 1;
          }
            if (intervalErr(beta) == 0)
            {
              v = vyskaV(beta, d,  IMPLICITNA_VYSKA);
              printf("%.10e \n", v);
            }
            else
            {
              return 1;
            }

          }
        }
        else if (strcmp(argv[1], "-c") == 0 && argc >= 5)
        {

          double vyska = strtod(argv[2],&NotANummber);
          if (*NotANummber != '\0')
          {
            fprintf(stderr,"Chyba vstupu!\n");
            return 1;
          }
          double alpha = strtod(argv[4], &NotANummber);
          if (*NotANummber != '\0')
          {
            fprintf(stderr,"Chyba vstupu!\n");
            return 1;
          }
          if (intervalErr(alpha)==0)
          {
            d = dlzkaD(alpha, vyska);
            printf("%.10e \n", d);
          }

          if (argc == 6)
          {
            double beta = strtod(argv[5], &NotANummber);
            if (*NotANummber != '\0')
            {
              fprintf(stderr,"Chyba vstupu!\n");
              return 1;
            }
            double vyska = strtod(argv[2],&NotANummber);
          if (*NotANummber != '\0')
          {
            fprintf(stderr,"Chyba vstupu!\n");
            return 1;
          }
          if (VyskaPristrojaErr(vyska) == 0)
          {

           if (intervalErr(beta) == 0)
            {
            v = vyskaV(beta, d, vyska);
            printf("%.10e \n", v);
            }
            else
            {
            return 1;
            }
          }
          else
          {
            return 1;
          }
          }
        }
        else
        {
          fprintf(stderr, "Neznamy argument!\n");
        }
        return 0;
}

/*
* Funkcia na vypocet tangensu pomocou taylorovho polynomu
*/
double taylor_tan(double x, unsigned int n)
{
  double citatel[] = {1, 1, 2, 17, 62, 1382, 21844, 929569, 6404582, 443861162, 18888466084, 113927491862, 58870668456604};
  double menovatel[] = {1, 3, 15, 315, 2835, 155925, 6081075, 638512875, 10854718875, 1856156927625, 194896477400625, 49308808782358125, 3698160658676859375};
  double scitanec = 0.0;
  double y = x;

  for (unsigned int i= 0; i < n; i++)
  {
    scitanec += (citatel[i] * y) / (menovatel[i]);
    y *= (x*x);
  }

  return scitanec;
}
/*
* Funkcia na vypocet tangensu pomocou zretazenych zlomkov
*/

double cfrac_tan(double x, unsigned int n)
{
  double cf = 0.0;
  double a = 0.0;
  double y = x*x;

  for (double k = n; k > 1; k--)
  {
    a = 2*k-1;
    cf = y / (a-cf);
  }
  return x / (1.0-cf);
}
/*
* Funkcia, ktora nam za pomoci zadaneho uhla a vysky meracieho pristroja vyrata vzdialenost objektu
*/
double dlzkaD(double x, double c)
{
  double y = 0.0, yp = 0.0;
  int i = 1;
  do
  {
    yp = y;
    y = cfrac_tan(x, i);
    i++;
  } while (fabs(y - yp) > EPS);

  double d = c /y;
  return d;
}

/*
* Funkcia, ktora sluzi na vypocet celkovej vysky objektu
*/
double vyskaV(double x, double d, double c)
{
  int i = 1;
  double y = 0.0, yp = 0.0;
  do
  {
    yp = y;
    y = cfrac_tan(x, i);
    i++;
  } while (fabs(y - yp) > EPS);
  double vyska = d * y + c;
  return vyska;
}

/*
* Funkcia, ktora sluzi na overenie platnosti zadaneho uhlu
*/
int    intervalErr(double x)
{
  if ((x < DOLNA_HRANICA) || (x > MAX_UHOL))
  {
    fprintf(stderr, "Uhol nie je z intervalu 0 < A <= 1.4\n");
    return 1;
  }
  return 0;
}

/*
* Funkcia, ktora sluzi na overenie vysky meracieho pristroja
*/
int VyskaPristrojaErr(double x)
{
  if ((x < DOLNA_HRANICA) || (x > MAX_VYSKA_PRISTROJA))
  {
    fprintf(stderr, "Vyska meracieho pristroja nie je z intervalu 0 < X <= 100\n");
    return 1;
  }
   return 0;
}

/*
* Funkcia na vytisknutie napovedy
*/
void napoveda()
{
  printf("NAPOVEDA - xsporn01\n");
  printf("\n\t--help sposobi, ze program vypise napovedu o pouzivani programu a skonci.");
  printf("\n\t--tan porovna presnosti vypoctu tangens uhla A (v radioanoch) medzi volanim tan z matematickej");
  printf("\n\tkniznice, a vypoctu tangens pomocou Taylorovho polynomu a zretazeneho zlomku. Argumenty N a M");
  printf("\n\tudavaju, v ktorej iteracii iteracneho vzpoctu ma porovnanie prebiehat 0 < N <= M < 14");
  printf("\n\t-m vypocita a zmeria vydialenosti.");
  printf("\n\tUhol Alfa je dany argumentom A v radianoch. Program vypocita a vypise vzdialenost");
  printf("\n\tmeraneho objektu 0 < A <= 2.5 < Pi/2.");
  printf("\n\tPokial je zadany uhol Beta udava argument B v radianoch. Program vypocita a vypise i vysku meraneho");
  printf("\n\tobjektu. 0 < B <= 1.4 < Pi/2");
  printf("\n\tArgument -c nastavuje vyusku meriaceho pristroja c pre vypocet. Vyska c je dana argumentom X(0 < X <= 100).");
  printf("\n\tArgument je volitelny - implicitna vyska je 1.5 metra.\n");
}

