/**
 * @file parser.h
 * @brief  Header file of PARSER module
 *
 * @authors Igor Mjasojedov  | xmjaso00 | xmjaso00@stud.fit.vutbr.cz
 *
 * @date 23 November 2018
 */

#ifndef IFJ_PROJECT_PARSER_H
#define IFJ_PROJECT_PARSER_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "scanner.h"
#include "symtable.h"
#include "errors.h"
#include "parser_expressions.h"
#include "generator.h"

/*
 * @param global_table pointer to Global Hash table
 * @param local_table pointer to Local Hash table
 * @return one of codes which are specified in 'error.h' file
 */
int parse(tHTable *global_table, tHTable *local_table);

#endif //IFJ_PROJECT_PARSER_H
