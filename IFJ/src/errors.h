/**
 * @file errors.h
 * @brief File contains macros of error codes
 *
 * @author Igor Mjasojedov | xmjaso00 | xmjaso00@stud.fit.vutbr.cz
 *
 * @date 23 November 2018
 */

#ifndef IFJ_PROJECT_ERRORS_H
#define IFJ_PROJECT_ERRORS_H

#define SYNTAX_OK 0

/// Error codes
#define LEXER_ERROR 1
#define SYNTAX_ERROR 2
#define SEMANTIC_DEF_ERROR 3
#define SEMANTIC_EXP_ERROR 4
#define SEMANTIC_PARAM_ERROR 5
#define SEMANTIC_OTHERS_ERROR 6
#define NUL_DIV_ERROR 9
#define INTERNAL_ERROR 99

#endif //IFJ_PROJECT_ERRORS_H
