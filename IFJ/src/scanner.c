/**
 * @file scanner.c
 * @brief Implementation of SCANNER module
 *
 * @authors Igor Mjasojedov  | xmjaso00 | xmjaso00@stud.fit.vutbr.cz
 *          Alex Sporni      | xsporn01 | xsporn01@stud.fit.vutbr.cz
 *
 * @date 23 November 2018
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "scanner.h"

const char *keywords[KEYWORDS_COUNT] = { "def", "do", "else", "end", "if", "not", "nil", "then", "while",
                                         "inputs", "inputf", "inputi", "print", "length", "substr", "ord", "chr"};

// ----- STDIN stream -----
extern FILE *inputFile;

/// Dynamic string to work with in get_next_token()
extern String_DYNAMIC lexeme;

#define LEX_ERROR 1;
#define LEX_OK 0;
#define EOL '\n'

/*
 * @return True param string is one of keywords
 * @return False param string is not one of keywords
 */
bool is_keyword(char *string) {
    int i;
    for(i = 0; i < KEYWORDS_COUNT; i++) {
        if(strcmp(string, keywords[i]) == 0) {
            return true;
        }
    }
    return false;
}

/*
 * @param c char to check, if it is HEXA char
 *
 * @return True received char is HEXA char
 * @return False received char is not HEXA char
 */
bool is_hexa(int c) {
    if((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || isdigit(c)) {
        return true;
    }
    return false;
}

int get_next_token(Token *nextToken) {
    int symbol;
    static bool lastTokenEOL = true;
    char begin[7] = "begin ";
    char end[6] = "=end ";
    char hexaCode[5] = "0x";
    State_FA actualState = STATE_START_F;
    string_clear(&lexeme);

    while(true) {
        symbol = getc(inputFile);
        switch(actualState) {
            case STATE_START_F:
                if(symbol == EOL){
                    nextToken->type = TYPE_EOL;
                    lastTokenEOL = true;
                    return LEX_OK;
                }
                else if(symbol == '=') {
                    actualState = STATE_ASSIGN_F;
                    break;
                }
                lastTokenEOL = false;
                if(isspace(symbol)) {
                    /// staying in STATE_START_F
                    break;
                }
                else if(islower(symbol) || symbol == '_') {
                    actualState = STATE_ID_KEY_F;
                    string_add_char(&lexeme, (char)symbol);
                    break;
                }
                else if((isdigit(symbol) && symbol != '0')) {
                    actualState = STATE_INTEGER_NUMBER_F;
                    string_add_char(&lexeme, (char)symbol);
                    break;
                }
                else if(symbol == '0') {
                    string_add_char(&lexeme, (char)symbol);
                    actualState = STATE_ZERO_NUMBER_F;
                    break;
                }
                /// ---- Arithmetic operators ----
                else if(symbol == '+') {
                    nextToken->type = TYPE_PLUS;
                    return LEX_OK;
                }
                else if(symbol == '-') {
                    nextToken->type = TYPE_MINUS;
                    return LEX_OK;
                }
                else if(symbol == '*') {
                    nextToken->type = TYPE_MUL;
                    return LEX_OK;
                }
                else if(symbol == '/') {
                    nextToken->type = TYPE_DIV;
                    return LEX_OK;
                }
                /// ---- Relational operators ----
                else if(symbol == '>') {
                    actualState = STATE_GREATER_EQUAL_F;
                    break;
                }
                else if(symbol == '<') {
                    actualState = STATE_LESS_EQUAL_F;
                    break;
                }
                else if(symbol == '!') {
                    actualState = STATE_NOT_EQ_F;
                    break;
                }
                /// ---- Other Symbols ----
                else if(symbol == ',') {
                    nextToken->type = TYPE_COMMA;
                    return LEX_OK;
                }
                else if(symbol == '(') {
                    nextToken->type = TYPE_LEFT_BRACKET;
                    return LEX_OK;
                }
                else if(symbol == ')') {
                    nextToken->type = TYPE_RIGHT_BRACKET;
                    return LEX_OK;
                }
                else if(symbol == '#') {
                    actualState = STATE_LINE_COMMENTARY;
                    break;
                }
                else if(symbol == '"') {
                    actualState = STATE_STRING;
                    break;
                }
                else if(symbol == EOF) {
                    nextToken->type = TYPE_EOF;
                    return LEX_OK;
                }
                return LEX_ERROR;
                ///---------------------------------------------------------------///
            case STATE_ID_KEY_F:
                if(isalnum(symbol) || symbol == '_') {
                    string_add_char(&lexeme, (char)symbol);
                    break;
                }
                if(symbol == '?' || symbol == '!') {
                    string_add_char(&lexeme, (char)symbol);
                    symbol = getc(inputFile);
                    if(symbol != '(') {
                        return LEX_ERROR;
                    }
                }

                (is_keyword(string_get_string(&lexeme)))
                ? (nextToken->type = TYPE_KEYWORD)
                : (nextToken->type = TYPE_IDENTIFIER);

                nextToken->attribute.str = string_get_string(&lexeme);
                ungetc(symbol, inputFile);
                return LEX_OK;
                ///---------------------------------------------------------------///
            case STATE_INTEGER_NUMBER_F:
                if(isdigit(symbol)) {
                    string_add_char(&lexeme, (char)symbol);
                    break;
                }
                if(symbol == '.') {
                    string_add_char(&lexeme, (char)symbol);
                    actualState = STATE_FLOATING_NUMBER;
                    break;
                }
                else if(symbol == 'e' || symbol == 'E') {
                    string_add_char(&lexeme, (char)symbol);
                    actualState = STATE_E_NUMBER;
                    break;
                }
                else {
                    ungetc(symbol,inputFile);
                    nextToken->type = TYPE_TOKEN_INTEGER;
                    nextToken->attribute.valueInt = atoi(string_get_string(&lexeme));
                    return LEX_OK;
                }
                ///---------------------------------------------------------------///
            case STATE_ZERO_NUMBER_F:
                if(symbol == '.') {
                    string_add_char(&lexeme, (char)symbol);
                    actualState = STATE_FLOATING_NUMBER;
                    break;
                }
                else if (symbol == 'e' || symbol == 'E') {
                    string_add_char(&lexeme, (char)symbol);
                    actualState = STATE_E_NUMBER;
                    break;
                }
                ungetc(symbol,inputFile);
                nextToken->type = TYPE_TOKEN_INTEGER;
                nextToken->attribute.valueInt = atoi(string_get_string(&lexeme));
                return LEX_OK;
                ///---------------------------------------------------------------///
            case STATE_FLOATING_NUMBER:
                if (isdigit(symbol)) {
                    string_add_char(&lexeme, (char)symbol);
                    actualState = STATE_FINAL_FLOAT_NUMBER_F_1;
                    break;
                }
                return LEX_ERROR;
                ///---------------------------------------------------------------///
            case STATE_E_NUMBER:
                if(symbol == '+' || symbol == '-') {
                    string_add_char(&lexeme, (char)symbol);
                    actualState = STATE_PLUS_MINUS;
                    break;
                }
                if(isdigit(symbol)) {
                    string_add_char(&lexeme, (char)symbol);
                    actualState = STATE_FINAL_FLOAT_NUMBER_F_2;
                    break;
                }
                return LEX_ERROR;
                ///---------------------------------------------------------------///
            case STATE_FINAL_FLOAT_NUMBER_F_1:
                if(isdigit(symbol)) {
                    string_add_char(&lexeme, (char)symbol);
                    break;
                }
                if(symbol == 'e' || symbol == 'E') {
                    string_add_char(&lexeme, (char)symbol);
                    actualState = STATE_E_NUMBER;
                    break;
                }
                ungetc(symbol, inputFile);
                nextToken->type = TYPE_TOKEN_FLOAT;
                nextToken->attribute.valueDouble = atof(string_get_string(&lexeme));
                return LEX_OK;
            case STATE_FINAL_FLOAT_NUMBER_F_2:
                if(isdigit(symbol)) {
                    string_add_char(&lexeme, (char)symbol);
                    break;
                }
                ungetc(symbol, inputFile);
                nextToken->type = TYPE_TOKEN_FLOAT;
                nextToken->attribute.valueDouble = atof(string_get_string(&lexeme));
                return LEX_OK;
                ///---------------------------------------------------------------///
            case STATE_PLUS_MINUS:
                if (isdigit(symbol)) {
                    string_add_char(&lexeme, (char)symbol);
                    actualState = STATE_FINAL_FLOAT_NUMBER_F_2;
                    break;
                }
                return LEX_ERROR;
                ///---------------------------------------------------------------///
            case STATE_STRING:
                if(symbol < 32) {
                    return LEX_ERROR;
                }
                if(symbol == '"') {
                    actualState = STATE_STRING_F;
                    break;
                }
                else if(symbol == '\\') {
                    actualState = STATE_STRING_ESCAPE;
                    break;
                }
                string_add_char(&lexeme,(char)symbol);
                break;
            case STATE_STRING_F:
                ungetc(symbol, inputFile);
                nextToken->type = TYPE_TOKEN_STRING;
                nextToken->attribute.str = string_get_string(&lexeme);
                return LEX_OK;
            case STATE_STRING_ESCAPE:
                if(symbol < 32) {
                    return LEX_ERROR;
                }
                else if(symbol == 't') {
                    string_add_char(&lexeme, '\t');
                    actualState = STATE_STRING;
                    break;
                }
                else if(symbol == 's') {
                    string_add_char(&lexeme, ' ');
                    actualState = STATE_STRING;
                    break;
                }
                else if(symbol == '"') {
                    string_add_char(&lexeme, '"');
                    actualState = STATE_STRING;
                    break;
                }
                else if(symbol == 'n') {
                    string_add_char(&lexeme, '\n');
                    actualState = STATE_STRING;
                    break;
                }
                else if(symbol == '\\') {
                    string_add_char(&lexeme, '\\');
                    actualState = STATE_STRING;
                    break;
                }
                else if(symbol == 'x') {
                    actualState = STATE_STRING_HEXA;
                    break;
                }
                else {
                    string_add_char(&lexeme, (char)symbol);
                    actualState = STATE_STRING;
                    break;
                }
                ///---------------------------------------------------------------///
            case STATE_STRING_HEXA:
                if(symbol < 32) {
                    return LEX_ERROR;
                }
                else if(is_hexa(symbol)) {
                    hexaCode[2] = (char)symbol;
                    actualState = STATE_STRING_HEXA_ADVANCED;
                    break;
                }
                return LEX_ERROR;
                ///---------------------------------------------------------------///
            case STATE_STRING_HEXA_ADVANCED:
                if(symbol < 32) {
                    return LEX_ERROR;
                }
                if(is_hexa(symbol)) {
                    hexaCode[3] = (char)symbol;
                    string_add_char(&lexeme, (char)strtol(hexaCode, NULL, 16));
                    actualState = STATE_STRING;
                    break;
                }
                else if(symbol == '"') {
                    //
                    string_add_char(&lexeme, (char)strtol(hexaCode, NULL, 16));
                    actualState = STATE_STRING_F;
                    break;
                }
                else if(symbol == '\\') {
                    actualState = STATE_STRING_ESCAPE;
                    break;
                }
                string_add_char(&lexeme, (char)strtol(hexaCode, NULL, 16));
                actualState = STATE_STRING;
                break;
            ///---------------------------------------------------------------///
            case STATE_ASSIGN_F:
                if(symbol == '=') {
                    nextToken->type = TYPE_EQUAL;
                    return LEX_OK;
                } else if(symbol == 'b' && lastTokenEOL) {
                    actualState = STATE_BLOCK_COMMENTARY;
                    break;
                }
                ungetc(symbol, inputFile);
                nextToken->type = TYPE_ASSIGMENT;
                return LEX_OK;
            case STATE_GREATER_EQUAL_F:
                if(symbol == '=') {
                    nextToken->type = TYPE_GREATER_EQUAL;
                    return LEX_OK;
                }
                ungetc(symbol, inputFile);
                nextToken->type = TYPE_GREATER;
                return LEX_OK;
                ///---------------------------------------------------------------///
            case STATE_LESS_EQUAL_F:
                if(symbol == '=') {
                    nextToken->type = TYPE_LESSER_EQUAL;
                    return LEX_OK;
                }
                ungetc(symbol, inputFile);
                nextToken->type = TYPE_LESS;
                return LEX_OK;
                ///---------------------------------------------------------------///
            case STATE_NOT_EQ_F:
                if(symbol == '=') {
                    nextToken->type = TYPE_NOT_EQUAL;
                    return LEX_OK;
                }
                return LEX_ERROR;
                ///---------------------------------------------------------------///
            case STATE_LINE_COMMENTARY:
                if(symbol == EOL) {
                    ungetc(symbol, inputFile);
                    actualState = STATE_START_F;
                    break;
                }
                break;
            case STATE_BLOCK_COMMENTARY:
                for(int i=2; i<6; i++) {
                    symbol = getc(inputFile);
                    if(symbol == EOF){return LEX_ERROR;}
                    if(symbol != begin[i]) {
                        if(i == 5 && symbol == EOL) {
                            continue;
                        }
                        nextToken->type = TYPE_ASSIGMENT;
                        return LEX_OK;
                    }
                }
                while(true) {
                    int i;
                    while(symbol != EOL) {
                        symbol = getc(inputFile);
                        if(symbol == EOF){return LEX_ERROR;}
                    }
                    for (i = 0; i < 5; i++) {
                        symbol = getc(inputFile);
                        if(symbol == EOF){return LEX_ERROR;}
                        if (symbol != end[i]) {
                            if(i == 4 && symbol == EOL) {
                                continue;
                            }
                            break;
                        }
                    }
                    if (i == 5) {
                        while(symbol != EOL) {
                            symbol = getc(inputFile);
                            if(symbol == EOF){return LEX_ERROR;}
                        }
                        actualState = STATE_START_F;
                        break;
                    }
                }
                break;
        }
    }
}