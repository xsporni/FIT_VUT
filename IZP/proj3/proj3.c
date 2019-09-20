/**
 * Kostra programu pro 3. projekt IZP 2017/18
 *
 * Jednoducha shlukova analyza
 * Unweighted pair-group average
 * https://is.muni.cz/th/172767/fi_b/5739129/web/web/usrov.html
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h> // sqrtf
#include <limits.h> // INT_MAX

#define MAX_X 1000
#define MAX_Y 1000
#define MINIMAL_VALUE 0

/*****************************************************************
 * Ladici makra. Vypnout jejich efekt lze definici makra
 * NDEBUG, napr.:
 *   a) pri prekladu argumentem prekladaci -DNDEBUG
 *   b) v souboru (na radek pred #include <assert.h>
 *      #define NDEBUG
 */
#ifdef NDEBUG
#define debug(s)
#define dfmt(s, ...)
#define dint(i)
#define dfloat(f)
#else

// vypise ladici retezec
#define debug(s) printf("- %s\n", s)

// vypise formatovany ladici vystup - pouziti podobne jako printf
#define dfmt(s, ...) printf(" - "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)

// vypise ladici informaci o promenne - pouziti dint(identifikator_promenne)
#define dint(i) printf(" - " __FILE__ ":%u: " #i " = %d\n", __LINE__, i)

// vypise ladici informaci o promenne typu float - pouziti
// dfloat(identifikator_promenne)
#define dfloat(f) printf(" - " __FILE__ ":%u: " #f " = %g\n", __LINE__, f)

#endif

/*****************************************************************
 * Deklarace potrebnych datovych typu:
 *
 * TYTO DEKLARACE NEMENTE
 *
 *   struct obj_t - struktura objektu: identifikator a souradnice
 *   struct cluster_t - shluk objektu:
 *      pocet objektu ve shluku,
 *      kapacita shluku (pocet objektu, pro ktere je rezervovano
 *          misto v poli),
 *      ukazatel na pole shluku.
 */

struct obj_t {
    int id;
    float x;
    float y;
};

struct cluster_t {
    int size;
    int capacity;
    struct obj_t *obj;
};

/*****************************************************************
 * Deklarace potrebnych funkci.
 *
 * PROTOTYPY FUNKCI NEMENTE
 *
 * IMPLEMENTUJTE POUZE FUNKCE NA MISTECH OZNACENYCH 'TODO'
 *
 */

/*
 Inicializace shluku 'c'. Alokuje pamet pro cap objektu (kapacitu).
 Ukazatel NULL u pole objektu znamena kapacitu 0.
*/
void init_cluster(struct cluster_t *c, int cap)
{
    assert(c != NULL);
    assert(cap >= 0);
    c->size = 0;

    if (c->capacity == 0)
    {
      c->obj = NULL;
      c->capacity = 0;
    }
    else
    {
      // alokacia pamate pre kapacitu clusteru
      c->obj = malloc(cap*sizeof(struct obj_t));
      c->capacity = cap;
    }
}

/*
 Odstraneni vsech objektu shluku a inicializace na prazdny shluk.
 */
void clear_cluster(struct cluster_t *c)
{
    free(c->obj); //uvolnenie pamate

    //inicialiyacia na prazdny zhluk
    c->size = 0;
    c->capacity = 0;
    c->obj = NULL;

}

/// Chunk of cluster objects. Value recommended for reallocation.
const int CLUSTER_CHUNK = 10;

/*
 Zmena kapacity shluku 'c' na kapacitu 'new_cap'.
 */
struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap)
{
    // TUTO FUNKCI NEMENTE
    assert(c);
    assert(c->capacity >= 0);
    assert(new_cap >= 0);

    if (c->capacity >= new_cap)
        return c;

    size_t size = sizeof(struct obj_t) * new_cap;

    void *arr = realloc(c->obj, size);
    if (arr == NULL)
        return NULL;

    c->obj = (struct obj_t*)arr;
    c->capacity = new_cap;
    return c;
}

/*
 Prida objekt 'obj' na konec shluku 'c'. Rozsiri shluk, pokud se do nej objekt
 nevejde.
 */
void append_cluster(struct cluster_t *c, struct obj_t obj)
{
   if (c->capacity <= c->size)
   {
     resize_cluster(c,c->capacity + CLUSTER_CHUNK);
   }
   //prida objekt na koniec pola zhlukov
   c->obj[c->size++] = obj;
}

/*
 Seradi objekty ve shluku 'c' vzestupne podle jejich identifikacniho cisla.
 */
void sort_cluster(struct cluster_t *c);

/*
 Do shluku 'c1' prida objekty 'c2'. Shluk 'c1' bude v pripade nutnosti rozsiren.
 Objekty ve shluku 'c1' budou serazeny vzestupne podle identifikacniho cisla.
 Shluk 'c2' bude nezmenen.
 */
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c2 != NULL);
/*
* Pridanie objektov zhluku c2 do zhluku c1
*/
    int predosla_velkost_c1 = c1->size;
    for (int j = 0; j < c2->size; j++)
    {
      //prida objekty c2, v pripade potreby kapacitu zvacsi vo funkcii appent_cluster
      append_cluster(c1, c2->obj[j]);
    }
    if (c2->size > 0 && c1->size == predosla_velkost_c1 + c2->size)
    {
      sort_cluster(c1);
    }
}

/**********************************************************************/
/* Prace s polem shluku */

/*
 Odstrani shluk z pole shluku 'carr'. Pole shluku obsahuje 'narr' polozek
 (shluku). Shluk pro odstraneni se nachazi na indexu 'idx'. Funkce vraci novy
 pocet shluku v poli.
*/
int remove_cluster(struct cluster_t *carr, int narr, int idx)
{
    assert(idx < narr);
    assert(narr > 0);

    //vymaze cluster z pola zhlukov
    clear_cluster(&carr[idx]);
    int novy_narr = narr - 1;
    for (int k = idx; k < novy_narr; k++)
    {
      carr[k] = carr[k+1];
    }
    return novy_narr;
}

/*
 Pocita Euklidovskou vzdalenost mezi dvema objekty.
 */
float obj_distance(struct obj_t *o1, struct obj_t *o2)
{
    assert(o1 != NULL);
    assert(o2 != NULL);

    //Euklidovska vzdialenost --> sqrt( (a1 - b1)^2 + (a2 - b2)^2 )
    return sqrtf(powf(o1->x - o2->x, 2.0) + powf(o1->y - o2->y, 2.0));
}

/*
 Pocita vzdalenost dvou shluku.
*/
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c1->size > 0);
    assert(c2 != NULL);
    assert(c2->size > 0);

    // TODO
    float vzdialenost = 0.0;
    int cesta = 0;
    for (int j = 0; j < c1->size; j++)
    {
      for (int k = 0; k < c2->size; k++)
      {
        vzdialenost = vzdialenost + obj_distance(&c1->obj[j],&c2->obj[k]);
        cesta++;
      }
    }
    float aritVzdial = (vzdialenost / (float)cesta);
    return aritVzdial;
}

/*
 Funkce najde dva nejblizsi shluky. V poli shluku 'carr' o velikosti 'narr'
 hleda dva nejblizsi shluky. Nalezene shluky identifikuje jejich indexy v poli
 'carr'. Funkce nalezene shluky (indexy do pole 'carr') uklada do pameti na
 adresu 'c1' resp. 'c2'.
*/
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2)
{
    assert(narr > 0);


    float minVzdial = INFINITY;

    for(int i = 0; i < narr; i++)
    {
        for(int k = i+1; k < narr; k++)
        {
            if(cluster_distance(&carr[i], &carr[k]) < minVzdial)
            {
                minVzdial = cluster_distance(&carr[i], &carr[k]);
                *c1 = i;
                *c2 = k;
            }
        }
    }
}

// pomocna funkce pro razeni shluku
static int obj_sort_compar(const void *a, const void *b)
{
    // TUTO FUNKCI NEMENTE
    const struct obj_t *o1 = (const struct obj_t *)a;
    const struct obj_t *o2 = (const struct obj_t *)b;
    if (o1->id < o2->id) return -1;
    if (o1->id > o2->id) return 1;
    return 0;
}

/*
 Razeni objektu ve shluku vzestupne podle jejich identifikatoru.
*/
void sort_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    qsort(c->obj, c->size, sizeof(struct obj_t), &obj_sort_compar);
}

/*
 Tisk shluku 'c' na stdout.
*/
void print_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    for (int i = 0; i < c->size; i++)
    {
        if (i) putchar(' ');
        printf("%d[%g,%g]", c->obj[i].id, c->obj[i].x, c->obj[i].y);
    }
    putchar('\n');
}

/*
 Ze souboru 'filename' nacte objekty. Pro kazdy objekt vytvori shluk a ulozi
 jej do pole shluku. Alokuje prostor pro pole vsech shluku a ukazatel na prvni
 polozku pole (ukalazatel na prvni shluk v alokovanem poli) ulozi do pameti,
 kam se odkazuje parametr 'arr'. Funkce vraci pocet nactenych objektu (shluku).
 V pripade nejake chyby uklada do pameti, kam se odkazuje 'arr', hodnotu NULL.
*/
int load_clusters(char *filename, struct cluster_t **arr)
{
  assert(arr != NULL);

    int pocObjektov;
    FILE *objekty;
    objekty = fopen(filename,"r");
    if (!objekty)
    {
      perror("Chyba s otvorenim suboru!\n");
      *arr = NULL;
      return EXIT_FAILURE;
    }
    fscanf(objekty, "count=%d", &pocObjektov);
    *arr= malloc((sizeof(struct cluster_t)) * pocObjektov);
    if (*arr == NULL)
    {
      fprintf(stderr, "Chyba s alokaciou pamate!\n");
     // *arr = NULL;
      return EXIT_FAILURE;
    }
    struct obj_t help;
    for (int i = 0; i <= pocObjektov; i++)
    {
        if (fscanf(objekty,"%d %f %f", &help.id, &help.x, &help.y)==3)
        {
          if ((help.x <= MINIMAL_VALUE) || (help.x > MAX_X) || (help.y <= MINIMAL_VALUE) || (help.y > MAX_Y))
          {
            fprintf(stderr,"Chyba! zadana hodnota pre x alebo y nie je z intervalu 0 <= X <= 1000 alebo 0 <= Y <= 1000!\n");
            *arr = NULL;
            return EXIT_FAILURE;
          }
          init_cluster(&(*arr)[i],1);
          append_cluster(&(*arr)[i],help);
        }

    }

    //fclose(objekty);
    if (fclose(objekty) == EOF)
    {
      fprintf(stderr,"Chyba s uzavretim suboru!\n");
      *arr = NULL;
      return EXIT_FAILURE;
    }
    return pocObjektov;

}
/*
 Tisk pole shluku. Parametr 'carr' je ukazatel na prvni polozku (shluk).
 Tiskne se prvnich 'narr' shluku.
*/
void print_clusters(struct cluster_t *carr, int narr)
{
    printf("Clusters:\n");
    for (int i = 0; i < narr; i++)
    {
        printf("cluster %d: ", i);
        print_cluster(&carr[i]);
    }
}

int main(int argc, char *argv[])
{
    struct cluster_t *clusters;
    if((argc > 3) || (argc < 2))
    {
        fprintf(stderr,"Chyba! nepovoleny pocet argumentov!\n");
        return EXIT_FAILURE;
    }
    int N = 0; //atoi(argv[2]);
    if(argc == 2)
    {
      N = 1;
    }
    else if (argc == 3)
    {
      N = atoi(argv[2]);
      if (atoi(argv[2]) <= 0)
      {
        fprintf(stderr, "Chyba!, zaporny argument!\n");
        return EXIT_FAILURE;
      }
    }

    int nacti = load_clusters(argv[1], &clusters);
    if (nacti < N)
    {
      fprintf(stderr, "Chyba s alokaciou pamate!\n");
      return EXIT_FAILURE;
    }
    int help = nacti;
    int index1, index2;

    for (int k = 0; k < (nacti - N); k++)
    {
      find_neighbours(clusters, help, &index1, &index2);
      merge_clusters(&clusters[index1], &clusters[index2]);
      help = remove_cluster(clusters, help, index2);
    }
    print_clusters(clusters, help);

    for (int j = 0; j < help; j++)
        {

            clear_cluster(&clusters[j]);
        }

        free(clusters);
  return 0;
}

