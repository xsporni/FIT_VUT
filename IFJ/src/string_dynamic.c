/**
 * @file string_dynamic.c
 * @brief Implementation of DYNAMIC_STRING module
 *
 * @author Igor Mjasojedov  | xmjaso00 | xmjaso00@stud.fit.vutbr.cz
 *
 * @date 23 November 2018
 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "string_dynamic.h"

#define STRING_ERROR 1
#define STRING_SUCCESS 0

int string_init(String_DYNAMIC *s) {
    if ((s->str = (char *) malloc(STRING_LENGTH_INC)) == NULL) {
        return STRING_ERROR;
    }
    s->str[0] = '\0';
    s->length = 0;
    s->allocSize = STRING_LENGTH_INC;
    return STRING_SUCCESS;
}

void string_free(String_DYNAMIC *s) {
    free(s->str);
}

void string_clear(String_DYNAMIC *s) {
    s->str[0] = '\0';
    s->length = 0;
}

int string_add_char(String_DYNAMIC *s, char c) {
    if ((s->length + 1) >= s->allocSize) {
        if ((s->str = (char *) realloc(s->str, s->length + STRING_LENGTH_INC)) == NULL) {
            return STRING_ERROR;
        }
        s->allocSize = s->length + STRING_LENGTH_INC;
    }
    s->str[s->length] = c;
    (s->length)++;
    s->str[s->length] = '\0';
    return STRING_SUCCESS;
}

int string_add_Cstr(String_DYNAMIC *s1, char *s2) {
    if ((s1->length + strlen(s2)) >= s1->allocSize) {
        if ((s1->str = (char *) realloc(s1->str, s1->length + strlen(s2) + STRING_LENGTH_INC)) == NULL) {
            return STRING_ERROR;
        }
        s1->allocSize = s1->length +  (unsigned)strlen(s2) + STRING_LENGTH_INC;
    }
    strcat(&(s1->str[s1->length]), s2);
    s1->length += strlen(s2);
    s1->str[s1->length] = '\0';
    return STRING_SUCCESS;
}

int string_copy_Cstr(String_DYNAMIC *s1, char *s2) {
    unsigned newLength = (unsigned)strlen(s2);
    if (newLength >= s1->allocSize) {
        // not enough memory allocated for s1
        if ((s1->str = (char*) realloc(s1->str, newLength + 1)) == NULL)
            return STRING_ERROR;
        s1->allocSize = newLength + 1;
    }
    strcpy(s1->str, s2);
    s1->length = newLength;
    return STRING_SUCCESS;
}

char *string_get_string(String_DYNAMIC *s) {
    return s->str;
}