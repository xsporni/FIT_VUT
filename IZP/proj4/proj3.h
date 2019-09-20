/*!
  *@brief   Projekt: Jednoduchá shluková analýza
  *@author  Alex Sporni xsporn01@stud.fit.vutbr.cz
  *@date    11.12.2017
  *@see     https://is.muni.cz/th/172767/fi_b/5739129/web/web/usrov.html
*/
/**
 * Kostra hlavickoveho souboru 3. projekt IZP 2017/18
 * a pro dokumentaci Javadoc.
 */

/**
 * \defgroup Dat_Typy
 * @{
 */
 /**
  * @brief Struktura reprezentujuca objekt s identifikatorom (ID) a suradnicami 'x' a 'y'
  */
struct obj_t {
    /** Identifikator */
    int id;
    /** Suradnica 'x' */
    float x;
    /** Suradnica 'y' */
    float y;
};

struct cluster_t {
    /** Pocet objektov v zhluku */
    int size;
    /** Kapacita zhluku*/
    int capacity;
    /** Pole objektov patriace danemu zhluku */
    struct obj_t *obj;
};
/**
*@}
*/

/**
 * @defgroup Praca_so_zhlukmi
 * @{
 */

 /**
  *
  * @brief Vycisti a inicializuje zhluk
  * @note Inicializacia zhluku 'c', alokuje pamat pre 'cap' objektu
  *       Ukazatel NULL pri poli objektu znamena kapacitu 0.
  * @pre c != NULL || cap >= 0
  * @post Zhluk 'c' bude mat alokovanu pamat pre 'cap; objektu ak nenastane chyba pri alokacii
  * @param c je zhluk pre inicializaciu
  * @param cap je pozadovana kapacita zhluku
  */
void init_cluster(struct cluster_t *c, int cap);

/**
 * @brief Vycistenie zhluku 'c'
 * @note Odstranenie vsetkych objektov zhluku a inicializacia na prazdny zhluk.
 * @post Alokovana pamat pre vsetky objekty zhluku 'c' bude uvolnena
 * @param c zhluk pre uvolnenie
 */
void clear_cluster(struct cluster_t *c);

/**
 * @brief hodnota pre realokaciu zhluku
 */
extern const int CLUSTER_CHUNK;

/**
 * @brief Zmena kapacity zhluku 'c'
 * @pre Kapacita zhluku 'c' bude vacsia alebo rovna nule
 * @post Kapacita zhluku 'c' bude zmenena na novu kapacitu 'new_cap' pokial nenastane chyba pri alokacii pamate, ak nastane vrati NULL
 * @param c zhluk pre zmenu kapacity
 * @param new_cap nova kapacita
 * @return vrati ukazatel na zaciatok danej struktury
 */
struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap);

/**
 * @brief prida objekt 'obj' na koniec zhluku 'c', ak sa do zhluku nevojde, tak rozsiri zhluk
 * @pre pocet objektov v zhluku bude vacsi alebo rovny 0
 * @post na poslednu poziciu zhluku 'c' bude pridany objekt 'obj', pokial nenastane chyba s alokaciou pamate
 * @param c zhluk pre pridanie do objektu
 * @param obj prida objekt 'obj' na koniec zhluku 'c'... rozsiri pokia sa do neho objekt nevojde
 */
void append_cluster(struct cluster_t *c, struct obj_t obj);


/**
 * @brief Spoji dva zhluky do jedneho (c2 do c1)
 * @note Do zhluku 'c1' prida objekty 'c2'. Zhluk 'c1' bude v pripade nutnosti rozsireny.
 *         Objekty v zhluku 'c1' budu zoradene vzostupne podla identifikacneho cisla.
 *         Zhluk 'c2' bude nezmeneny.
 * @pre Pocet objektov v zhluku 'c2; bude vacsi alebo rovny 0, c1 != NULL && c2 != NULL
 * @post Zhluk 'c1' bude rozsireny o zhluk 'c2' pokial nenastane chyba pri alokacii pamate
 * @post Objekty v zhluku 'c1; budu zoradene vzostupne podla identifikatoru (ID)
 * @param 'c1' zhluk do ktoreho budu pridane objekty zhluku 'c2'
 * @param 'c2' zhluk ktory odovzda svoje objekty do zhluku 'c1'
 */
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2);

/**
  *@}
 */

 /**
  * @defgroup pole_zhlukov
  * @{
  */
/**
 * @brief Odstrani zhluk z pola zhlukov 'carr'
 * @note Odstrani zhluk z pola shlukov 'carr'. Pole zhluku obsahuje 'narr' poloziek
 *         (zhluku). Zhluk pre odstranenie sa nachadza na indexe 'idx'. Funkcia vracia novy
 *         pocet zhluku v poli.
 * @pre idx < narr && narr > 0
 * @post Z pola zhlukov 'carr' bude odstraneny prvok na indexe 'idx' a pole bude o 1 mensie
 * @param carr pole zhlukov
 * @param narr pocet zhlukov v poli
 * @param idx index zhluku pre odstranenie
 * @return vrati novy pocet zhlukov v poli
 */
int remove_cluster(struct cluster_t *carr, int narr, int idx);

/**
 * @brief Pocita Euklidovsku vzdialenost medzi dvoma objektmi podla vztahu sqrt( (a1 - b1)^2 + (a2 - b2)^2 )
 * @see https://en.wikipedia.org/wiki/Euclidean_distance
 * @pre o1 != NULL && o2 != NULL
 * @param o1 = objekt 1
 * @param o2 = objekt 2
 * @return vracia Euklidovsku vzdialenost medzi objektmi 'o1' a 'o2' (float)
*/
float obj_distance(struct obj_t *o1, struct obj_t *o2);

/**
 * @brief Pocita vzdialenost dvoch zhlukov
 * @pre pocet objektov v zhlukoch 'c1' a 'c2' bude vacsi nez 0, c1->size > 0 && c2->size > 0
 * @param c1 zhluk 1
 * @param c2 zhluk 2
 * @return vracia aritmeticku vzdialenost medzi zhlukmi 'c1' a 'c2'
*/
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2);

/**
  * @brief Najde dva najblizsie zhluky
  * @note Funkcia najde dva nejblizsie zhluky. V poli shlukov 'carr' o velkosti 'narr'
  *        hlada dva nejblizsie zhluky. Najde zhluky identifikuje ich indexy v poli
  *        'carr'. Funkcia najde zhluky (indexy do pola 'carr') uklada do pamati na
  *        adresu 'c1' resp. 'c2'.
  * @pre narr > 0
  * @param carr pole zhlukov
  * @param narr pocet zhlukov v poli
  * @param c1 index jedneho z najdenzych zhlukov
  * @param c2 index druheho z najdenych zhlukov
  * @return (void)
*/
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2);
/**
  * @}
*/
/**
 *\addtogroup Praca_so_zhlukmi
 * @{
*/
/**
 * @brief Zoradenie objektov
 * @note Zoradenie objektov v zhluku vzostupne podle ich identifikatorov (ID).
 * @post Objekty v zhluku 'c' budu zoradene vzostupne podla ID
 * @param c zhluk pre zoradenie
*/

void sort_cluster(struct cluster_t *c);

/**
 * @brief Vypis zhluku na stdout.
 * @post Objekty zhluku 'c' budu vypisane na stdout
 * @param c zhluk pre vypis
*/
/**
 *@}
*/

/**
 *\addtogroup pole_zhlukov
 * @{
*/
void print_cluster(struct cluster_t *c);

/**
 * @brief Nacita zhluky zo suboru
 * @note Zo suboru 'filename' nacita objekty. Pre kazdy objekt vytvori zhluk a ulozi
 *       ho do pola zhlukov. Alokuje priestor pre pole vsetkych zhlukov a ukazatel na prvu
 *       polozku pola (ukalazatel na prvy shluk v alokovanom poli) ulozi do pamati,
 *       kam se odkazuje parameter 'arr'. Funkcia vracia pocet nacitanych objektov (zhluku).
 *       V pripade nejakej chyby uklada do pamati, kam se odkazuje 'arr', hodnotu NULL.
 * @pre Bude existovat subor 'filename' a program bude mat prava pre jeho precitanie
 * @pre Data v subore musia byt v spravnom formate ID -> decimalne cele cislo, X a Y realne cisla z intervalu (0,1000>
 * @post Pre kazdy ojekt uvedeny vo vstupnom subore bude vytvoreny zhluk, vsetky zhluky budu ulozene v
 *       poli zhlukov 'arr', ktore budu mat alokovanu pamat pre pocet zhlukov uvedenych vo vstupnom subore pokial nenastane chyba
 * @param filename nazov suboru pre nacitanie objektov
 * @param arr ukazatel na pole zhlukov nacitanych zo suboru
 * @return vrati pocet nacitanych zhlukov v pripade chyby EXIT_FAILURE
*/
int load_clusters(char *filename, struct cluster_t **arr);

/**
* @brief Vypis pola zhlukov na stdout
* @param carr pole zhlukov pre vypis
* @param narr pocet zhlukov v poli
* return (void)
*/
void print_clusters(struct cluster_t *carr, int narr);
/**
 *@}
*/

