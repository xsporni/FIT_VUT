/**
 * @file parser_expressions.c
 * @brief Implementation of PARSER_EXPRESSIONS module
 *
 * @author Igor Mjasojedov  | xmjaso00 | xmjaso00@stud.fit.vutbr.cz
 *
 * @date 27 November 2018
 */

#include "parser_expressions.h"
#include "generator.h"

// ----------------- Extern Global variables ---------------- //
extern bool inFunction;
extern bool inIF_OR_inWHILE_condition;
extern tHTItem *l_value_id;
extern tHTable *Gtable;
extern tHTable *Ltable;

// -------------------- Global variables -------------------- //
String_DYNAMIC tmp;
Token token;
Stack stack;
sItemDataType finalDataType;

/*
 *  Precendece table
 *  @brief  < SHIFT
 *          > REDUCE
 *          = EQUAL
 *          # ERROR
 */
const char precendence_table[PRECEDENCE_TABLE_SIZE][PRECEDENCE_TABLE_SIZE] = {
//    +-     */    r     rE    (     )     i     $
    { '>' , '<' , '>' , '>' , '<' , '>' , '<' , '>' }, //   + -
    { '>' , '>' , '>' , '>' , '<' , '>' , '<' , '>' }, //   * /
    { '<' , '<' , '#' , '>' , '<' , '>' , '<' , '>' }, //   r     -->  < <= > >=
    { '<' , '<' , '<' , '#' , '<' , '>' , '<' , '>' }, //   rE    -->  == !=
    { '<' , '<' , '<' , '<' , '<' , '=' , '<' , '#' }, //   (
    { '>' , '>' , '>' , '>' , '#' , '>' , '#' , '>' }, //   )
    { '>' , '>' , '>' , '>' , '#' , '>' , '#' , '>' }, //   i     -->  id, integer, float, string
    { '<' , '<' , '<' , '<' , '<' , '#' , '<' , '#' }, //   $     -->  EOL, then, do
};

#define OPERAND_1 1
#define OPERAND_3 0

/* ----------------------------------------------------------------- *\
 * ----------------------- MACROS DEFINITIONS ---------------------- *
\* ----------------------------------------------------------------- */
#define GET_NEXT_TOKEN() \
        if((get_next_token(&token)) == LEXER_ERROR) { \
            stack_free(&stack); \
            return LEXER_ERROR; \
        }

#define IS_TOKEN(token_type) \
        (token.type == token_type)

#define IS_TOKEN_KEYWORD(keyword) \
        ((token.type == TYPE_KEYWORD) && (strcmp(token.attribute.str, keyword) == 0))

#define CHECK_TOKEN_VALID_SYMBOL(token_type) \
    if(IS_TOKEN(TYPE_COMMA) || IS_TOKEN(TYPE_EOF) || IS_TOKEN(TYPE_ASSIGMENT)) { \
        stack_free(&stack);  \
        return SYNTAX_ERROR; \
    } else if(IS_TOKEN(TYPE_KEYWORD)) { \
        if(!(IS_TOKEN_KEYWORD("do") || IS_TOKEN_KEYWORD("then"))) { \
            stack_free(&stack); \
            return SYNTAX_ERROR; \
        } \
    }

IdType get_idType(sItemDataType finalExpType) {
        switch(finalExpType) {
            case sI_TYPE_INTEGER:
                return TYPE_INTEGER;
            case sI_TYPE_FLOAT:
                return TYPE_FLOAT;
            case sI_TYPE_STRING:
                return TYPE_STRING;
            case sI_TYPE_UNDEFINED:
                return TYPE_UNDEFINED;
        }
}

int get_table_position(SymbolType symbol) {
    switch(symbol) {
        case S_TYPE_PLUS:
        case S_TYPE_MINUS:
            return INDEX_PLUS_MINUS;
        case S_TYPE_MUL:
        case S_TYPE_DIV:
            return INDEX_MUL_DIV;
        case S_TYPE_LESS:
        case S_TYPE_LESS_EQUAL:
        case S_TYPE_GREATER:
        case S_TYPE_GREATER_EQUAL:
            return INDEX_REL_OP;
        case S_TYPE_EQUAL:
        case S_TYPE_NOT_EQUAL:
            return INDEX_REL_EQ_OP;
        case S_TYPE_L_BRACKET:
            return INDEX_L_BRACKET;
        case S_TYPE_R_BRACKET:
            return INDEX_R_BRACKET;
        case S_TYPE_ID:
            return INDEX_ID;
        case S_TYPE_DOLLAR:
            return INDEX_DOLLAR;
    }
}

/*
 * @pre have to be called after CHECK_TOKEN_VALID_SYMBOL() check
 */
SymbolType get_symbol_of_token(TokenType tokenType){
    switch(tokenType) {
    // ------------ TERMS -------------
        case TYPE_IDENTIFIER:
        case TYPE_TOKEN_INTEGER:
        case TYPE_TOKEN_STRING:
        case TYPE_TOKEN_FLOAT:
            return S_TYPE_ID;
    // ----- ARITHMETIC OPERATORS -----
        case TYPE_PLUS:
            return S_TYPE_PLUS;
        case TYPE_MINUS:
            return S_TYPE_MINUS;
        case TYPE_MUL:
            return S_TYPE_MUL;
        case TYPE_DIV:
            return S_TYPE_DIV;
    // ----- RELATIONAL OPERATORS -----
        case TYPE_GREATER:
            return S_TYPE_GREATER;
        case TYPE_GREATER_EQUAL:
            return S_TYPE_GREATER_EQUAL;
        case TYPE_LESS:
            return S_TYPE_LESS;
        case TYPE_LESSER_EQUAL:
            return S_TYPE_LESS_EQUAL;
        case TYPE_EQUAL:
            return S_TYPE_EQUAL;
        case TYPE_NOT_EQUAL:
            return S_TYPE_NOT_EQUAL;
    // ----------- BRACKETS -----------
        case TYPE_LEFT_BRACKET:
            return S_TYPE_L_BRACKET;
        case TYPE_RIGHT_BRACKET:
            return S_TYPE_R_BRACKET;
    // --------------------------------
        case TYPE_EOL:
        case TYPE_KEYWORD:
            return S_TYPE_DOLLAR; // here we can get only with keyword 'then' or 'do' which are both valid
    // --------------------------------
        default:
            return S_TYPE_ERROR;
    }
}

int get_type_of_token(Token token, sItemDataType *type) {
    tHTItem *symbol;

    switch(token.type) {
        case TYPE_TOKEN_STRING:
             *type = sI_TYPE_STRING;
             break;
        case TYPE_TOKEN_INTEGER:
            *type = sI_TYPE_INTEGER;
            break;
        case TYPE_TOKEN_FLOAT:
            *type = sI_TYPE_FLOAT;
            break;
        case TYPE_IDENTIFIER:
            if(inFunction) {
                if((symbol=htSearch(Ltable, token.attribute.str)) != NULL) {
                    switch(symbol->data.type) {
                        case TYPE_STRING:
                            *type = sI_TYPE_STRING;
                            break;
                        case TYPE_INTEGER:
                            *type = sI_TYPE_INTEGER;
                            break;
                        case TYPE_FLOAT:
                            *type = sI_TYPE_FLOAT;
                            break;
                        case TYPE_UNDEFINED:
                            *type = sI_TYPE_UNDEFINED;
                            break;
                    }
                    return SYNTAX_OK;
                }
                return SEMANTIC_DEF_ERROR;
            }
            if((symbol=htSearch(Gtable, token.attribute.str)) != NULL) {
                if(!(symbol->data.isFunction)) {
                    switch(symbol->data.type) {
                        case TYPE_STRING:
                            *type = sI_TYPE_STRING;
                            break;
                        case TYPE_INTEGER:
                            *type = sI_TYPE_INTEGER;
                            break;
                        case TYPE_FLOAT:
                            *type = sI_TYPE_FLOAT;
                            break;
                        case TYPE_UNDEFINED:
                            *type = sI_TYPE_UNDEFINED;
                            break;
                    }
                    return SYNTAX_OK;
                }
            }
            return SEMANTIC_DEF_ERROR;
        default:
            *type = sI_TYPE_UNDEFINED;
    }
    return SYNTAX_OK;
}

int rule_check(RuleType *rule, Item_Stack *op1, Item_Stack *op2, Item_Stack *op3, unsigned symbolCount) {
    switch(symbolCount) {
        case 1:
            if(op1->symbol == S_TYPE_ID) {
                *rule = OPERAND_to_E;       // E -> i
                break;
            }
            return SYNTAX_ERROR;

        case 3:
            if((op1->symbol == S_TYPE_L_BRACKET) && (op2->symbol == S_TYPE_NOT_TERMINAL) &&
               (op3->symbol == S_TYPE_R_BRACKET)) {
                *rule = LB_E_RB;            // E -> (E)
                return SYNTAX_OK;
            }
            switch(op2->symbol) {
                case S_TYPE_PLUS:           // E -> E + E
                    *rule = E_PLUS_E;
                    break;
                case S_TYPE_MINUS:          // E -> E - E
                    *rule = E_MINUS_E;
                    break;
                case S_TYPE_MUL:            // E -> E * E
                    *rule = E_MUL_E;
                    break;
                case S_TYPE_DIV:            // E -> E / E
                    *rule = E_DIV_E;
                    break;
                case S_TYPE_LESS:           // E -> E < E
                    *rule = E_LT_E;
                    break;
                case S_TYPE_LESS_EQUAL:     // E -> E <= E
                    *rule = E_LE_E;
                    break;
                case S_TYPE_GREATER:        // E -> E > E
                    *rule = E_GT_E;
                    break;
                case S_TYPE_GREATER_EQUAL:  // E -> E >= E
                    *rule = E_GE_E;
                    break;
                case S_TYPE_EQUAL:          // E -> E == E
                    *rule = E_EQ_E;
                    break;
                case S_TYPE_NOT_EQUAL:      // E -> E != E
                    *rule = E_NEQ_E;
                    break;
                default:
                    return SYNTAX_ERROR;
            }
            break;
        default:
            return SYNTAX_ERROR;
    }
    return SYNTAX_OK;
}

int semantics_control(RuleType *rule, Item_Stack *op1, Item_Stack *op2, Item_Stack *op3,
                      sItemDataType *finalDataType, tHTItem *function) {
    switch(*rule) {
        case OPERAND_to_E:
            *finalDataType = op1->data_type;
            break;
        case LB_E_RB:
            *finalDataType = op2->data_type;
            break;
            // -------------------------------------------------------------------------------------------- //
        case E_PLUS_E:
        case E_MINUS_E:
        case E_MUL_E:
            if (op1->data_type != sI_TYPE_UNDEFINED && op3->data_type != sI_TYPE_UNDEFINED) {
                if (op1->data_type == sI_TYPE_STRING && op3->data_type == sI_TYPE_STRING &&
                    (op2->symbol == S_TYPE_PLUS)) {
                    *finalDataType = sI_TYPE_STRING;
                    *rule = E_CONCAT_E;
                    break;
                }
                if (op1->data_type == sI_TYPE_STRING || op3->data_type == sI_TYPE_STRING) {
                    return SEMANTIC_EXP_ERROR;
                }
                if (op1->data_type == sI_TYPE_BOOL || op2->data_type == sI_TYPE_BOOL) {
                    return SEMANTIC_EXP_ERROR;
                }
                if (op1->data_type == sI_TYPE_INTEGER && op3->data_type == sI_TYPE_INTEGER) {
                    *finalDataType = sI_TYPE_INTEGER;
                    break;
                }
                if (op1->data_type == sI_TYPE_INTEGER) {
                    generate_INT2FLOAT(OPERAND_1);
                    *finalDataType = sI_TYPE_FLOAT;
                    break;
                } else if (op3->data_type == sI_TYPE_INTEGER) {
                    generate_INT2FLOAT(OPERAND_3);
                    *finalDataType = sI_TYPE_FLOAT;
                    break;
                } else {
                    *finalDataType = sI_TYPE_FLOAT;
                    break;
                }
            }
            if (inFunction) {
                if (op1->data_type == sI_TYPE_UNDEFINED && op3->data_type == sI_TYPE_UNDEFINED) {
                    generate_TYPE_S_control(function->key);
                } else if (op1->data_type == sI_TYPE_UNDEFINED) {
                    switch (op3->data_type) {
                        case sI_TYPE_INTEGER:
                            generate_TYPE_control(OPERAND_1, 1, function->key, finalDataType);
                            break;
                        case sI_TYPE_FLOAT:
                            generate_TYPE_control(OPERAND_1, 2, function->key, finalDataType);
                            break;
                        case sI_TYPE_STRING:
                            generate_TYPE_control(OPERAND_1, 0, function->key, finalDataType);
                            break;
                    }
                } else {
                    switch (op1->data_type) {
                        case sI_TYPE_INTEGER:
                            generate_TYPE_control(OPERAND_3, 1, function->key, finalDataType);
                            break;
                        case sI_TYPE_FLOAT:
                            generate_TYPE_control(OPERAND_3, 2, function->key, finalDataType);
                            break;
                        case sI_TYPE_STRING:
                            generate_TYPE_control(OPERAND_3, 0, function->key, finalDataType);
                            break;
                    }
                }
                break;
            }
            return SEMANTIC_DEF_ERROR;
            // -------------------------------------------------------------------------------------------- //
        case E_DIV_E:
            if (op1->data_type != sI_TYPE_UNDEFINED && op3->data_type != sI_TYPE_UNDEFINED) {
                if (op1->data_type == sI_TYPE_STRING || op3->data_type == sI_TYPE_STRING) {
                    return SEMANTIC_EXP_ERROR;
                }
                if (op1->data_type == sI_TYPE_BOOL || op2->data_type == sI_TYPE_BOOL) {
                    return SEMANTIC_EXP_ERROR;
                }

                if (op1->data_type == sI_TYPE_INTEGER && op3->data_type == sI_TYPE_INTEGER) {
                    generate_DIV_by_0();
                    *rule = E_IDIV_E;
                    *finalDataType = sI_TYPE_FLOAT;
                    break;
                }
                *finalDataType = sI_TYPE_FLOAT;
                if (op1->data_type == sI_TYPE_INTEGER) {
                    generate_INT2FLOAT(OPERAND_1);
                    generate_DIV_by_0f();
                    break;
                } else if (op3->data_type == sI_TYPE_INTEGER) {
                    generate_INT2FLOAT(OPERAND_3);
                    generate_DIV_by_0f();
                    break;
                }
                generate_DIV_by_0f();
                break;
            }
            if (inFunction) {
                if (op1->data_type == sI_TYPE_UNDEFINED && op3->data_type == sI_TYPE_UNDEFINED) {
                    generate_TYPE_S_control(function->key);
                } else if (op1->data_type == sI_TYPE_UNDEFINED) {
                    switch (op3->data_type) {
                        case sI_TYPE_INTEGER:
                            generate_TYPE_control(OPERAND_1, 1, function->key, finalDataType);
                            break;
                        case sI_TYPE_FLOAT:
                            generate_TYPE_control(OPERAND_1, 2, function->key, finalDataType);
                            break;
                        case sI_TYPE_STRING:
                            generate_TYPE_control(OPERAND_1, 0, function->key, finalDataType);
                            break;
                    }
                } else {
                    switch (op1->data_type) {
                        case sI_TYPE_INTEGER:
                            generate_TYPE_control(OPERAND_3, 1, function->key, finalDataType);
                            break;
                        case sI_TYPE_FLOAT:
                            generate_TYPE_control(OPERAND_3, 2, function->key, finalDataType);
                            break;
                        case sI_TYPE_STRING:
                            generate_TYPE_control(OPERAND_3, 0, function->key, finalDataType);
                            break;
                    }
                }
                break;
            }
            return SEMANTIC_DEF_ERROR;
        case E_LT_E:
        case E_LE_E:
        case E_GT_E:
        case E_GE_E:
            if (op1->data_type != sI_TYPE_UNDEFINED && op3->data_type != sI_TYPE_UNDEFINED) {
                if (op1->data_type == sI_TYPE_STRING && op3->data_type == sI_TYPE_STRING) {
                    *finalDataType = sI_TYPE_BOOL;
                    break;
                }
                if (op1->data_type == sI_TYPE_STRING || op3->data_type == sI_TYPE_STRING) {
                    return SEMANTIC_EXP_ERROR;
                }
                if (op1->data_type == sI_TYPE_INTEGER && op3->data_type == sI_TYPE_INTEGER) {
                    *finalDataType = sI_TYPE_BOOL;
                    break;
                }
                if (op1->data_type == sI_TYPE_BOOL && op2->data_type == sI_TYPE_BOOL) {
                    *finalDataType = sI_TYPE_BOOL;
                    break;
                }
                if ((op1->data_type == sI_TYPE_BOOL && op2->data_type != sI_TYPE_BOOL) ||
                    (op1->data_type != sI_TYPE_BOOL && op2->data_type == sI_TYPE_BOOL)) {
                    return SEMANTIC_EXP_ERROR;
                }
                *finalDataType = sI_TYPE_BOOL;
                if (op1->data_type == sI_TYPE_INTEGER) {
                    generate_INT2FLOAT(OPERAND_1);
                    break;
                } else if (op3->data_type == sI_TYPE_INTEGER) {
                    generate_INT2FLOAT(OPERAND_3);
                    break;
                }
                break;
            }
            if (inFunction) {
                if (op1->data_type == sI_TYPE_UNDEFINED && op3->data_type == sI_TYPE_UNDEFINED) {

                } else if (op1->data_type == sI_TYPE_UNDEFINED) {
                    switch (op3->data_type) {
                        case sI_TYPE_INTEGER:
                            generate_TYPE_control(OPERAND_1, 1, function->key, finalDataType);
                            break;
                        case sI_TYPE_FLOAT:
                            generate_TYPE_control(OPERAND_1, 2, function->key, finalDataType);
                            break;
                        case sI_TYPE_STRING:
                            generate_TYPE_control(OPERAND_1, 0, function->key, finalDataType);
                            break;
                    }
                } else {
                    switch (op1->data_type) {
                        case sI_TYPE_INTEGER:
                            generate_TYPE_control(OPERAND_3, 1, function->key, finalDataType);
                            break;
                        case sI_TYPE_FLOAT:
                            generate_TYPE_control(OPERAND_3, 2, function->key, finalDataType);
                            break;
                        case sI_TYPE_STRING:
                            generate_TYPE_control(OPERAND_3, 0, function->key, finalDataType);
                            break;
                    }
                }
                break;
            }
            return SEMANTIC_DEF_ERROR;
        case E_EQ_E:
        case E_NEQ_E:
            if (op1->data_type != sI_TYPE_UNDEFINED && op3->data_type != sI_TYPE_UNDEFINED) {
                *finalDataType = sI_TYPE_BOOL;
                if (op1->data_type == sI_TYPE_INTEGER && op3->data_type == sI_TYPE_FLOAT) {
                    generate_INT2FLOAT(OPERAND_1);
                    break;
                } else if (op3->data_type == sI_TYPE_INTEGER && op1->data_type == sI_TYPE_FLOAT) {
                    generate_INT2FLOAT(OPERAND_3);
                    break;
                } else if (op1->data_type == op3->data_type) {
                    *finalDataType = sI_TYPE_BOOL;
                    break;
                }
                generate_truly_false(); // gen pop 2x, push false, push true -> false after EQ
                break;
            }
            break;
    }
    return SYNTAX_OK;
}

int reduce(tHTItem *function) {
    int result;
    bool found = false;
    unsigned count = stack_get_count_to_reduce(&stack, &found);
    Item_Stack operand1, operand2, operand3;
    RuleType rule;

    if(count == 2 && found) {
        operand1 = *(stack.topPtr);
        rule_check(&rule, &operand1, NULL, NULL, count-1);
    } else if(count == 4 && found) {
        operand1 = *(stack.topPtr->nextPtr->nextPtr);
        operand2 = *(stack.topPtr->nextPtr);
        operand3 = *(stack.topPtr);
        rule_check(&rule, &operand1, &operand2, &operand3, count-1);
    } else {
        return SYNTAX_ERROR;
    }
    if((result=semantics_control(&rule, &operand1, &operand2, &operand3, &finalDataType, function))) {
        return result;
    }

    generate_stack_OPERATION(rule);

    stack_pop_count(&stack, count);
    stack_push(&stack, S_TYPE_NOT_TERMINAL, finalDataType);    // will push also with its dataType
    return SYNTAX_OK;
}

int expression(Token tokenBuffer[2], unsigned BufferTCounter, tHTItem *function) {
    string_init(&tmp);
    int result;
    if(BufferTCounter != 0) {
        CHECK_TOKEN_VALID_SYMBOL(tokenBuffer[0]->type);
    } else {
        GET_NEXT_TOKEN();
        CHECK_TOKEN_VALID_SYMBOL(token.type);
    }
    // init_stack with DOLLAR
    stack_init(&stack);
    stack_push(&stack, S_TYPE_DOLLAR, sI_TYPE_UNDEFINED);
    char table_value;
    SymbolType actual_symbol, last_terminal;
    sItemDataType actual_dataType;

    while(true) {
        if(BufferTCounter != 0) {
            actual_symbol = get_symbol_of_token(tokenBuffer[2-BufferTCounter].type);
            if((result=get_type_of_token(tokenBuffer[2-BufferTCounter], &actual_dataType))) {
                return result;
            }

        } else {
            actual_symbol = get_symbol_of_token(token.type);
            if((result=get_type_of_token(token, &actual_dataType))) {
                return result;
            }
        }
        last_terminal = stack_get_last_terminal(&stack);
        table_value = precendence_table[get_table_position(last_terminal)][get_table_position(actual_symbol)];

        switch(table_value) {
            case '<':
                //generate push to stack str--if ID    valueInt/Double/str--if VALUE
                if(BufferTCounter != 0) {
                    switch((tokenBuffer[2-BufferTCounter].type)) {
                        case TYPE_IDENTIFIER:
                            generate_PUSHS_id(tokenBuffer[2-BufferTCounter].attribute.str);
                            break;
                        case TYPE_TOKEN_STRING:
                            string_clear(&tmp);
                            convert_string2codeString(tokenBuffer[2-BufferTCounter].attribute.str, &tmp);
                            generate_PUSHS_value("string", tmp.str);
                            break;
                        case TYPE_TOKEN_FLOAT:
                            sprintf(tmp.str, "%a",  tokenBuffer[2-BufferTCounter].attribute.valueDouble);
                            generate_PUSHS_value("float", tmp.str);
                            break;
                        case TYPE_TOKEN_INTEGER:
                            sprintf(tmp.str, "%d",  tokenBuffer[2-BufferTCounter].attribute.valueInt);
                            generate_PUSHS_value("int", tmp.str);
                            break;
                        default:
                            break;
                    }
                    BufferTCounter--;
                }
                else {
                    switch (token.type) {
                        case TYPE_IDENTIFIER:
                            generate_PUSHS_id(token.attribute.str);
                            break;
                        case TYPE_TOKEN_STRING:
                            string_clear(&tmp);
                            convert_string2codeString(token.attribute.str, &tmp);
                            generate_PUSHS_value("string", tmp.str);
                            break;
                        case TYPE_TOKEN_FLOAT:
                            sprintf(tmp.str, "%a", token.attribute.valueDouble);
                            generate_PUSHS_value("float", tmp.str);
                            break;
                        case TYPE_TOKEN_INTEGER:
                            sprintf(tmp.str, "%d", token.attribute.valueInt);
                            generate_PUSHS_value("int", tmp.str);
                            break;
                        default:
                            break;
                    }
                }
                stack_push_after_top_terminal(&stack, S_TYPE_TO_REDUCE, sI_TYPE_UNDEFINED);
                stack_push(&stack, actual_symbol, actual_dataType);
                if(BufferTCounter != 0) {
                    CHECK_TOKEN_VALID_SYMBOL(tokenBuffer[2-BufferTCounter].type);
                } else {
                    GET_NEXT_TOKEN();
                    CHECK_TOKEN_VALID_SYMBOL(token.type);
                }
                break;
            case '>':
                if(!(result = reduce(function))) {
                    break;
                }
                stack_free(&stack);
                return result;
            case '=':
                stack_push(&stack, actual_symbol, actual_dataType);
                GET_NEXT_TOKEN();
                CHECK_TOKEN_VALID_SYMBOL(token.type);
                break;
            case '#':
                if(actual_symbol == S_TYPE_DOLLAR && last_terminal == S_TYPE_DOLLAR) {
                    if(inIF_OR_inWHILE_condition) {
                        generate_POPS_toEXPresult();
                    }
                    else {
                        if(l_value_id != NULL) {
                            generate_POPS_toEXPresult();
                            generate_MOVE_exp_to_Lvalue(l_value_id->key);
                            if(get_idType(finalDataType) == l_value_id->data.type ||
                               l_value_id->data.type == TYPE_UNDEFINED) {
                                l_value_id->data.type = get_idType(finalDataType);
                                l_value_id = NULL;
                            }
                            else {
                                return SEMANTIC_DEF_ERROR;
                            }
                        }
                        else {
                            generate_POPS_toEXPresult();
                        }
                    }
                    if(inFunction) {
                        if(function != NULL) {
                            function->data.type = get_idType(finalDataType);
                        }
                    }
                    stack_free(&stack);
                    return SYNTAX_OK;
                }
                stack_free(&stack);
                return SYNTAX_ERROR;
        }
    }
}
