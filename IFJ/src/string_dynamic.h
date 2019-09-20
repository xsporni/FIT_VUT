/**
 * @file string_dynamic.h
 * @brief Header file of DYNAMIC_STRING module
 *
 * @author Igor Mjasojedov  | xmjaso00 | xmjaso00@stud.fit.vutbr.cz
 *
 * @date 23 November 2018
 */

#ifndef IFJ_PROJECT_STR_DYNAMIC_H
#define IFJ_PROJECT_STR_DYNAMIC_H

#define STRING_LENGTH_INC 8

typedef struct {
    char *str;          // pointer to the string
    unsigned length;	// real length of string
    unsigned allocSize;	// size of allocated memory
} String_DYNAMIC;

/* ---------------------------------------------------------------------------------------------- *
 * ----------------------------------- FUNCTIONS DECLARATIONS ----------------------------------- */

/*
 * @brief Initializes string with primal allocSize STRING_LENGTH_INC
 *
 * @param s pointer to String_DYNAMIC
 * @return STRING_SUCCESS successful allocation of memory
 * @return STRING_ERROR un-successful allocation of memory
 */
int string_init(String_DYNAMIC *s);

/*
 * @brief Frees allocated memory of string
 *
 * @param s pointer to String_DYNAMIC
 */
void string_free(String_DYNAMIC *s);

/*
 * @brief Clears string to state after initialization
 *
 * @param s pointer to String_DYNAMIC
 */
void string_clear(String_DYNAMIC *s);

/*
 * @brief Adds char to end of string
 *
 * @param s pointer to String_DYNAMIC
 * @param c char to be added
 * @return STRING_SUCCESS by default or after successful re-allocation of memory
 * @return STRING_ERROR un-successful re-allocation of memory
 */
int string_add_char(String_DYNAMIC *s1, char c);

/*
 * @brief Adds "Cstring" to end of string
 *
 * @param s1 pointer to String_DYNAMIC
 * @param s2 pointer to "Cstring" to be added
 * @return STRING_SUCCESS by default or after successful re-allocation of memory
 * @return STRING_ERROR un-successful re-allocation of memory
 */
int string_add_Cstr(String_DYNAMIC *s1, char *s2);

/*
 * @brief Copies content of "Cstring" to dynamic string
 *
 * @param s1 pointer to String_DYNAMIC
 * @param s2 pointer to "Cstring" to be copied from
 * @return STRING_SUCCESS by default or after successful re-allocation of memory
 * @return STRING_ERROR un-successful re-allocation of memory
 */
int string_copy_Cstr(String_DYNAMIC *s1, char *s2);

/*
 * @brief Returns pointer to string
 *
 * @param s pointer to String_DYNAMIC
 * @return returns pointer to string
 */
char *string_get_string(String_DYNAMIC *s);

#endif //IFJ_PROJECT_STR_DYNAMIC_H
