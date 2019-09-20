/**
 * @file scanner.h
 * @brief Implementation of SCANNER module
 *
 * @authors Igor Mjasojedov  | xmjaso00 | xmjaso00@stud.fit.vutbr.cz
 *          Alex Sporni      | xsporn01 | xsporn01@stud.fit.vutbr.cz
 *
 * @date 23 November 2018
 */
#ifndef IFJ_PROJECT_SCANNER_H
#define IFJ_PROJECT_SCANNER_H

#include "string_dynamic.h"
#include <stdbool.h>

#define KEYWORDS_COUNT 17

/*
 * States of Finite State Automata
 */
typedef enum {
    STATE_START_F,
    STATE_ID_KEY_F,
    // ----- Numbers -----
    STATE_INTEGER_NUMBER_F,
    STATE_FLOATING_NUMBER,
    STATE_ZERO_NUMBER_F,
    STATE_E_NUMBER,
    STATE_PLUS_MINUS,
    STATE_FINAL_FLOAT_NUMBER_F_1,
    STATE_FINAL_FLOAT_NUMBER_F_2,
    // ----- Comments -----
    STATE_LINE_COMMENTARY,
    STATE_BLOCK_COMMENTARY,
    // ----- Strings -----
    STATE_STRING,
    STATE_STRING_F,
    STATE_STRING_ESCAPE,
    STATE_STRING_HEXA,
    STATE_STRING_HEXA_ADVANCED,
    // ----- Operators -----
    STATE_GREATER_EQUAL_F,
    STATE_LESS_EQUAL_F,
    STATE_NOT_EQ_F,
    STATE_ASSIGN_F
} State_FA;

/*
 * Token types
 */
typedef enum {
    TYPE_KEYWORD,
    TYPE_IDENTIFIER,

    TYPE_ASSIGMENT,     // =
    // ----- Arithmetic operators -----
    TYPE_PLUS,          // +
    TYPE_MINUS,         // -
    TYPE_MUL,           // *
    TYPE_DIV,           // /
    // ----- Relational operators -----
    TYPE_GREATER,       // >
    TYPE_LESS,          // <
    TYPE_GREATER_EQUAL, // >=
    TYPE_LESSER_EQUAL,  // <=
    TYPE_EQUAL,         // ==
    TYPE_NOT_EQUAL,     // !=
    // ----- Other symbols -----
    TYPE_LEFT_BRACKET,  // (
    TYPE_RIGHT_BRACKET, // )
    TYPE_COMMA,         // ,
    TYPE_EOL,
    TYPE_EOF,
    TYPE_TOKEN_STRING,
    TYPE_TOKEN_INTEGER,
    TYPE_TOKEN_FLOAT
} TokenType;

/*
 * Token attributes
 */
typedef union {
    int valueInt;
    double valueDouble;
    char *str;
} TokenAttribute;

/*
 * Structure of Token
 */
typedef struct token {
    TokenType type;
    TokenAttribute attribute;
} Token;

/**
 * @param string string to check with keywords table
 * @return True string is keyword
 * @return False string is not keyword
 */
bool is_keyword(char *string);

/**
 * @brief Function receives pointer to Token variable and sets its structure items
 *        to actual token based on received lexeme
 *
 * @param nextToken pointer to Token
 * @return LEX_OK lexical analysis was successful, next token is valid
 * @return LEX_ERROR lexical analysis was un-successful, next token is not valid
 */
int get_next_token(Token *nextToken);

#endif //IFJ_PROJECT_SCANNER_H
