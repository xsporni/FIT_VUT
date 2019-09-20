/**
 * @file stack.h
 * @brief Header file for stack of tokens implementation
 *
 * @author Daniel Weis | xweisd00 | xweisd00@stud.fit.vutbr.cz
 *
 * @date 25 November 2018
 */

#ifndef IFJ_PROJECT_STACK_H
#define IFJ_PROJECT_STACK_H

#include <stdio.h>
#include <stdbool.h>


typedef enum {
//  ----- Terminals ----- //
    S_TYPE_PLUS,
    S_TYPE_MINUS,
    S_TYPE_MUL,
    S_TYPE_DIV,
    S_TYPE_LESS,
    S_TYPE_LESS_EQUAL,
    S_TYPE_GREATER,
    S_TYPE_GREATER_EQUAL,
    S_TYPE_EQUAL,
    S_TYPE_NOT_EQUAL,
    S_TYPE_L_BRACKET,
    S_TYPE_R_BRACKET,
    S_TYPE_ID,
    S_TYPE_DOLLAR,
//  ----- for STACK staffs ----- //
    S_TYPE_TO_REDUCE, // <
    S_TYPE_NOT_TERMINAL, // E
    S_TYPE_ERROR
} SymbolType;

typedef enum {
    sI_TYPE_UNDEFINED,
    sI_TYPE_INTEGER,
    sI_TYPE_FLOAT,
    sI_TYPE_STRING,
    sI_TYPE_BOOL
} sItemDataType;


// ----- STRUCTURES ----- //
//items & symbols
typedef struct item_stack {
    SymbolType symbol;
    sItemDataType data_type;
    struct item_stack *nextPtr;
} Item_Stack;

//the structure
typedef struct p_stack_top {
    Item_Stack *topPtr;
} Stack;


/* ---------------------------------------------------------------------------------------------- *
 * ----------------------------------- FUNCTIONS DECLARATIONS ----------------------------------- */

/**
 * @brief Initialisation of stack with ->topPtr set to NULL
 *
 * @param stack Pointer to stack
 */
void stack_init (Stack* stack);

/**
 *  @brief Pushes new created Item_Stack with data 'symbol' 'type'
 *
 *  @param stack Pointer to stack
 *  @param symbol Data to insert into stack
 *  @param type Type of new created item
 *  @return true Success
 *  @return false Failure during allocation of memory
 */
bool stack_push(Stack* stack, SymbolType symbol, sItemDataType type);

/**
 *  @brief Pops out an item from the stack
 *
 *  @param stack Pointer to stack
 *  @return true Success
 *  @return false Failure during allocation of memory
 */
bool stack_pop (Stack* stack);

/**
 *  @brief Pops out items based on Counter value
 *
 *  @param stack Pointer to stack
 *  @param counter Count of popped elements
 *  @return true Success
 *  @return false If the stack or pointer to stack is NULL
 */
bool stack_pop_count(Stack* stack, unsigned counter);

/**
 *  @brief Pushes new created Item_Stack with 'symbol' 'data' after the first terminal
 *
 *  @param stack Pointer to stack
 *  @param symbol Data to insert into stack
 *  @param type Type of new created item
 *  @return true Success
 *  @return false Failure during allocation of memory
 */
bool stack_push_after_top_terminal(Stack* stack, SymbolType symbol, sItemDataType type);

/**
 *  @brief Searching through the stack until finds the terminal and returns its type
 *
 *  @param stack Pointer to stack
 *  @return SymbolType first found terminal from the top of the stack
 *  @return S_TYPE_ERROR If there is no terminal in the stack
 */
SymbolType stack_get_last_terminal(Stack* stack);

/**
 *  @brief Search for how many elements are in the stack before the reduce symbol 'S_TYPE_TO_REDUCE'
 *
 *  @param stack Pointer to stack
 *  @param found Indicator of found symbol
 *  @return number of elements before S_TYPE_TO_REDUCE
 */
unsigned stack_get_count_to_reduce(Stack* stack, bool *found);

/**
 *  @brief Empties out the whole stack and frees out the memory
 *
 *  @param stack Pointer to stack
 */
void stack_free (Stack* stack);

#endif //IFJ_PROJECT_STACK_H
