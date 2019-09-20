/**
 * @file stack.c
 * @brief File contains implementation of functions regarding stack
 *
 * @author Daniel Weis | xweisd00 | xweisd00@stud.fit.vutbr.cz
 *
 * @date 25 November 2018
 */

#include <stdbool.h>
#include <malloc.h>
#include "stack.h"

#define IS_NOT_TERMINAL(stackItem) \
    (stackItem->symbol == S_TYPE_TO_REDUCE || stackItem->symbol == S_TYPE_NOT_TERMINAL || \
     stackItem->symbol == S_TYPE_ERROR) \
    ? true : false


void stack_init(Stack *stack) {
    stack->topPtr = NULL;
}

bool stack_push(Stack *stack, SymbolType symbol, sItemDataType type) {
    Item_Stack *new_item = (Item_Stack *) malloc(sizeof(Item_Stack));
    if (new_item == NULL) {
        return false;
    }

    if((symbol == S_TYPE_NOT_TERMINAL) || (symbol == S_TYPE_ID)) {
        new_item->data_type = type;
    } else {
        new_item->data_type = sI_TYPE_UNDEFINED;
    }
    new_item->symbol = symbol;
    new_item->nextPtr = stack->topPtr;
    stack->topPtr = new_item;

    return true;
}

bool stack_pop(Stack *stack) {
    Item_Stack *new_item;
    if (stack->topPtr != NULL) {
        new_item = stack->topPtr;
        stack->topPtr = new_item->nextPtr;
        free(new_item);
        return true;
    }
    return false;

}

bool stack_pop_count(Stack *stack, unsigned counter) {
    if (stack != NULL) {
        for (int i = 0; i < counter; i++) {
            stack_pop(stack);
        }
        return true;
    }
    return false;

}

bool stack_push_after_top_terminal(Stack *stack, SymbolType symbol, sItemDataType type) {
    Item_Stack* tmp = stack->topPtr, *previousItem = NULL;

    while(tmp != NULL) {
        if(IS_NOT_TERMINAL(tmp)) {
            previousItem = tmp;
            tmp = tmp->nextPtr;
            continue;
        }
        Item_Stack *new_item = (Item_Stack *) malloc(sizeof(Item_Stack));
        if (new_item == NULL) {
            return false;
        }

        if((symbol == S_TYPE_NOT_TERMINAL) || (symbol == S_TYPE_ID)) {
            new_item->data_type = type;
        } else {
            new_item->data_type = sI_TYPE_UNDEFINED;
        }
        new_item->symbol = symbol;

        if(previousItem == NULL) {
            stack->topPtr = new_item;
        } else {
            previousItem->nextPtr = new_item;
        }
        new_item->nextPtr = tmp;

        return true;
    }
    return false;
}

SymbolType stack_get_last_terminal(Stack* stack) {
    Item_Stack* tmp = stack->topPtr;

    while(tmp != NULL) {
        if(tmp->symbol == S_TYPE_TO_REDUCE || tmp->symbol == S_TYPE_NOT_TERMINAL || tmp->symbol == S_TYPE_ERROR) {
            tmp = tmp->nextPtr;
        }
        return tmp->symbol;
    }
    return S_TYPE_ERROR;
}

unsigned stack_get_count_to_reduce(Stack* stack, bool *found) {
    Item_Stack* tmp = stack->topPtr;
    unsigned counter = 0;

    while(tmp->nextPtr != NULL) {
        counter++;
        if(tmp->symbol == S_TYPE_TO_REDUCE) {
            *found = true;
            return counter;
        }
        tmp = tmp->nextPtr;
    }

    *found = false;
    return 0;
}


void stack_free(Stack *stack) {
    while (stack_pop(stack))
        ;
    stack = NULL;
}
