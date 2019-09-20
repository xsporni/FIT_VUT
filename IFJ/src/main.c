/**
 *  Project: IFJ2018 Compiler
 *  School: VUTBR FIT BIT2
 *
 *  @file main.c
 *  @brief Compiler for programming language IFJ2018
 *
 *  @authors Igor Mjasojedov    | xmjaso00 | xmjaso00@stud.fit.vutbr.cz
 *           Alex Sporni        | xsporn01 | xsporn01@stud.fit.vutbr.cz
 *           Richard Borbely    | xborbe00 | xborbe00@stud.fit.vutbr.cz
 *           Daniel Weis        | xweisd00 | xweisd00@stud.fit.vutbr.cz
 *
 *  @date 5 December 2018
 */

#include <stdio.h>
#include <stdbool.h>
#include "scanner.h"
#include "string_dynamic.h"
#include "parser.h"
#include "symtable.h"

FILE *inputFile;
String_DYNAMIC lexeme;

int main(int argc, char *argv[]) {
    inputFile = stdin;
    string_init(&lexeme);
    int result;
    tHTable Gtable, Ltable;
    htInit(&Gtable);
    htInit(&Ltable);

    result = parse(&Gtable, &Ltable);
    switch(result) {
        case SYNTAX_ERROR:
            fprintf(stderr, "%s\n", "SYNTAX_ERROR");
            break;
        case SEMANTIC_DEF_ERROR:
            fprintf(stderr, "%s\n", "SEMANTIC_DEF_ERROR");
            break;
        case SEMANTIC_PARAM_ERROR:
            fprintf(stderr, "%s\n", "SEMANTIC_PARAM_ERROR");
            break;
        case SEMANTIC_EXP_ERROR:
            fprintf(stderr, "%s\n", "SEMANTIC_EXP_ERROR");
            break;
        case SEMANTIC_OTHERS_ERROR:
            fprintf(stderr, "%s\n", "SEMANTIC_OTHERS_ERROR");
            break;
        case NUL_DIV_ERROR:
            fprintf(stderr, "%s\n", "NUL_DIV_ERROR");
            break;
        case INTERNAL_ERROR:
            fprintf(stderr, "%s\n", "INTERNAL_ERROR");
            break;
        case LEXER_ERROR:
            fprintf(stderr, "%s\n", "LEXER_ERROR");
            break;
        default:
            fprintf(stderr, "%s\n", "Compilation OK");
    }

    htClearAll(&Gtable);
    htClearAll(&Ltable);
    string_free(&lexeme);
    return result;
}