/**
 * @file symtable.c
 * @brief File contains implementation of functions regarding Hash Table
 *
 * @authors Igor Mjasojedov | xmjaso00 | xmjaso00@stud.fit.vutbr.cz
 *          Daniel Weis     | xweisd00 | xweisd00@stud.fit.vutbr.cz
 *
 * @date 23 November 2018
 */

#include "symtable.h"

int HTSIZE = MAX_HT_SIZE;

unsigned long hash_ELF_function(const unsigned char *key) {
    unsigned long   index = 0, high;
    while(*key) {
        index = (index << 4) + *key++;
        if ((high = index & 0xF0000000)) {
            index ^= high >> 24;
        }
        index &= ~high;
    }
    return ( index % HTSIZE );
}

void htInit(tHTable* ptrht) {
    if(ptrht != NULL) {
        unsigned long index;
        for(index = 0; index < MAX_HT_SIZE; index++) {
            (*ptrht)[index] = NULL;
        }
    }
}

tHTItem* htSearch(tHTable* ptrht, tKey key) {
    if((*ptrht) != NULL) {
        unsigned long index = hash_ELF_function((unsigned char *) key);
        tHTItem *tmp = (*ptrht)[index];
        while(tmp != NULL) {
            if(!strcmp(tmp->key, key)) {
                return tmp;
            }
            tmp = tmp->ptrnext;
        }
    }
    return NULL;
}

tHTItem *htInsert(tHTable* ptrht, tKey key, bool *internalError) {
    if((*ptrht) != NULL) {
        unsigned long index;
        tHTItem *newItem;
        newItem = (tHTItem*) malloc(sizeof(tHTItem));
        if(newItem != NULL) {
            index = hash_ELF_function((unsigned char *) key);
            newItem->key = malloc(strlen(key)+1);
            if(newItem->key != NULL) {
                memcpy(newItem->key,key,strlen(key)+1);
                newItem->ptrnext = (*ptrht)[index];
                (*ptrht)[index] = newItem;
                return newItem;
            }
        }
        *internalError = true;
        return NULL;
    }
    return NULL;
}

void htClearAll(tHTable* ptrht) {
    if((*ptrht) != NULL) {
        tHTItem *tmp = NULL;
        unsigned long index;
        for (index = 0; index < HTSIZE; index++) {
            while ((*ptrht)[index] != NULL) {
                tmp = (*ptrht)[index]->ptrnext;
                free((*ptrht)[index]->key);
                free((*ptrht)[index]);
                (*ptrht)[index] = tmp;
            }
        }
    }
}
