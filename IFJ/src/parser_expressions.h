/**
 * @file parser_expressions.h
 * @brief  Header file of PARSER_EXPRESSIONS module
 *
 * @author Igor Mjasojedov  | xmjaso00 | xmjaso00@stud.fit.vutbr.cz
 *
 * @date 27 November 2018
 */

#ifndef IFJ_PROJECT_PARSER_EXPRESSIONS_H
#define IFJ_PROJECT_PARSER_EXPRESSIONS_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "scanner.h"
#include "errors.h"
#include "stack.h"
#include "symtable.h"
#include "string_dynamic.h"

#define PRECEDENCE_TABLE_SIZE 8

/*
 *
 */
typedef enum {
    INDEX_PLUS_MINUS,   // index 0
    INDEX_MUL_DIV,      // index 1
    INDEX_REL_OP,       // index 2
    INDEX_REL_EQ_OP,    // index 3
    INDEX_L_BRACKET,    // index 4
    INDEX_R_BRACKET,    // index 5
    INDEX_ID,           // index 6
    INDEX_DOLLAR        // index 7
} TableIndex;

/*
 * Type of Rules for precedence syntax analysis of expressions
 */
typedef enum Rule_Type{
    E_PLUS_E,       // E -> E + E
    E_CONCAT_E,     // E -> E + E -- strings
    E_MINUS_E,      // E -> E - E
    E_MUL_E,        // E -> E * E
    E_DIV_E,        // E -> E / E
    E_IDIV_E,
    E_LT_E,         // E -> E < E
    E_LE_E,         // E -> E <= E
    E_GT_E,         // E -> E > E
    E_GE_E,         // E -> E >= E
    E_EQ_E,         // E -> E == E
    E_NEQ_E,        // E -> E != E
 // ------------------------------ //
    LB_E_RB,        // E -> (E)
    OPERAND_to_E    // E -> i
} RuleType;

extern RuleType Rule_Type;

int expression(Token tokenBuffer[2], unsigned BufferTCounter, tHTItem *function);

#endif //IFJ_PROJECT_PARSER_EXPRESSIONS_H
