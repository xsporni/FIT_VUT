/**
 * @file symtable.c
 * @brief Header file of Hash Table module
 *
 * @authors Igor Mjasojedov | xmjaso00 | xmjaso00@stud.fit.vutbr.cz
 *          Daniel Weis     | xweisd00 | xweisd00@stud.fit.vutbr.cz
 *
 * @date 23 November 2018
 */

#ifndef IFJ_PROJECT_HASH_TABLE_H
#define IFJ_PROJECT_HASH_TABLE_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "string_dynamic.h"
#include "errors.h"


#define MAX_HT_SIZE 12289

/*
 * @enum Identifier type
 */
typedef enum{
  TYPE_INTEGER,
  TYPE_FLOAT,
  TYPE_STRING,
  TYPE_UNDEFINED
} IdType;

/*
 * Hash table key
 */
typedef char *tKey;

/*
 * Hash table item data structure
 */
typedef struct table_data{
    String_DYNAMIC parameters;
    IdType type;
    bool isDefined;
    bool isFunction;
} tData;

/*
 * Type of ITEM in HT with explicit concatenated synonyms
 */
typedef struct tHTItem{
    tKey key;
    tData data;
    struct tHTItem *ptrnext;
} tHTItem;

typedef tHTItem *tHTable[MAX_HT_SIZE];

extern int HTSIZE;

/* ---------------------------------------------------------------------------------------------- *
 * ----------------------------------- FUNCTIONS DECLARATIONS ----------------------------------- */

/**
 * @brief Function returns index of hash table based on 'key' parameter in range 0...HTSIZE-1
 * @param key Key -- identifier of function or variable
 * @return indexOfTable
 */
unsigned long hash_ELF_function(const unsigned char *key);

/**
 * @brief Initializes table with NULL ptr to all indexes
 * @param ptrht pointer to Hash Table
 */
void htInit(tHTable* ptrht);

/**
 * @brief Searches for HT Item with 'key'
 * @param ptrht pointer to Hash Table
 * @param key Key -- identifier of function or variable
 * @return pointer to founded tHTItem
 * @return NULL if tHTItem with certain key not found
 */
tHTItem *htSearch(tHTable* ptrht, tKey key);

/**
 * @brief Inserts new tHTItem with certain key
 * @param ptrht pointer to Hash Table
 * @param key Key -- identifier of function or variable
 * @param internalError indicator of internal error (malloc failure)
 * @return pointer to newly created tHTItem
 * @return NULL
 */
tHTItem *htInsert (tHTable* ptrht, tKey key, bool *internalError);

/**
 * @brief Clears entire Hash Table
 * @param ptrht pointer to Hash Table
 */
void htClearAll(tHTable* ptrht);

#endif //IFJ_PROJECT_HASH_TABLE_H
