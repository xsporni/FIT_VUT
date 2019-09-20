/**
 * @file parser.c
 * @brief Implementation of PARSER module
 *
 * @author Igor Mjasojedov  | xmjaso00 | xmjaso00@stud.fit.vutbr.cz
 *
 * @date 23 November 2018
 */

#include "parser.h"

// ----------------- Global variables ---------------- //
tHTable *Gtable;
tHTable *Ltable;
Token token;
int result = SYNTAX_OK;
bool inFunction = false;
bool inIF_OR_inWHILE_condition = false;
tHTItem *l_value_id;
unsigned BufferTCounter = 0;
Token tokenBuffer[2] = {};
static tHTItem *l_value_fnc;
//-------------------------------------//
String_DYNAMIC index_param;
String_DYNAMIC value_str;
// --------------------------------------------------- //

#define IN_IF_STATMENT 0
#define IN_WHILE_STATEMENT 1

#define OPERATORS_COUNT 4
const int operators[OPERATORS_COUNT] = {TYPE_PLUS, TYPE_MINUS, TYPE_MUL, TYPE_DIV};

/* ----------------------------------------------------------------- *\
 * ----------------------- MACROS DEFINITIONS ---------------------- *
\* ----------------------------------------------------------------- */
#define CHECK_TOKEN(token_type) \
    if(token.type != token_type) { \
        return SYNTAX_ERROR; \
    }

#define CHECK_TOKEN_KEYWORD(keyword) \
    if(!((token.type == TYPE_KEYWORD) && \
       (strcmp(token.attribute.str, keyword) == 0))) { \
        return SYNTAX_ERROR; \
    }

#define IS_TOKEN(token_type) \
        (token.type == token_type)

#define IS_TOKEN_KEYWORD(keyword) \
        ((token.type == TYPE_KEYWORD) && (strcmp(token.attribute.str, keyword) == 0))

#define IS_TOKEN_TERM(token_type) ( \
        (token_type == TYPE_IDENTIFIER) ||     \
        (token_type == TYPE_TOKEN_INTEGER) ||  \
        (token_type == TYPE_TOKEN_FLOAT)   ||  \
        (token_type == TYPE_TOKEN_STRING))

#define GET_NEXT_TOKEN() \
        if((get_next_token(&token)) == LEXER_ERROR) { \
            return LEXER_ERROR; \
        }
/* ---------------------------- */
int state_list(tHTItem *symbol);

/*
 * @param token_type token type which we are are looking for
 * @return True if current token has type of 'token_type'
 * @return False if current token is not type of 'token_type'
 */
bool is_token_operator(TokenType token_type) {
    for(int i = 0; i < OPERATORS_COUNT; i++) {
        if(token_type == operators[i]) {
            return true;
        }
    }
    return false;
}
//  ---------------------------------------------------------------- //
#define INIT_SUCCESS 0

/*
 * @brief Initializes Global Hash Table with inserting all built-in functions with
 *        its information about parameters
 * @return True Insertion was successful
 * @return False Memory allocation was un-successful
 */
int init_Gtable() {
    bool internalError = false;
    tHTItem *symbol;

    if((symbol = htInsert(Gtable, "inputs", &internalError)) != NULL) {
        symbol->data.isFunction = true;
        symbol->data.isDefined = true;
        symbol->data.type = TYPE_STRING;
    }
    if((symbol = htInsert(Gtable, "inputf", &internalError)) != NULL) {
        symbol->data.isFunction = true;
        symbol->data.isDefined = true;
        symbol->data.type = TYPE_FLOAT;
    }
    if((symbol = htInsert(Gtable, "inputi", &internalError)) != NULL) {
        symbol->data.isFunction = true;
        symbol->data.isDefined = true;
        symbol->data.type = TYPE_INTEGER;
    }
    if((symbol = htInsert(Gtable, "length", &internalError)) != NULL) {
        symbol->data.isFunction = true;
        symbol->data.isDefined = true;
        if(string_init(&symbol->data.parameters)) { return INTERNAL_ERROR; }
        if(string_add_char(&symbol->data.parameters, 's')) { return INTERNAL_ERROR; }
        symbol->data.type = TYPE_INTEGER;
    }
    if((symbol = htInsert(Gtable, "substr", &internalError)) != NULL) {
        symbol->data.isFunction = true;
        symbol->data.isDefined = true;
        if(string_init(&symbol->data.parameters)) { return INTERNAL_ERROR; }
        if(string_add_char(&symbol->data.parameters, 's')) { return INTERNAL_ERROR; }
        if(string_add_char(&symbol->data.parameters, 'i')) { return INTERNAL_ERROR; }
        if(string_add_char(&symbol->data.parameters, 'i')) { return INTERNAL_ERROR; }
        symbol->data.type = TYPE_STRING;
    }
    if((symbol = htInsert(Gtable, "ord", &internalError)) != NULL) {
        symbol->data.isFunction = true;
        symbol->data.isDefined = true;
        if(string_init(&symbol->data.parameters)) { return INTERNAL_ERROR; }
        if(string_add_char(&symbol->data.parameters, 's')) { return INTERNAL_ERROR; }
        if(string_add_char(&symbol->data.parameters, 'i')) { return INTERNAL_ERROR; }
        symbol->data.type = TYPE_INTEGER;
    }
    if((symbol = htInsert(Gtable, "chr", &internalError)) != NULL) {
        symbol->data.isFunction = true;
        symbol->data.isDefined = true;
        if(string_init(&symbol->data.parameters)) { return INTERNAL_ERROR; }
        if(string_add_char(&symbol->data.parameters, 'i')) { return INTERNAL_ERROR; }
        symbol->data.type = TYPE_STRING;
    }
    if(internalError) {
        return INTERNAL_ERROR;
    }
    return INIT_SUCCESS;
}

int param_next(tHTItem *new_function) {
    static unsigned counter = 1;
    GET_NEXT_TOKEN();
    bool internalError = false;
    tHTItem *new_variable;
    switch(token.type) {

        /// Rule 17: <param_next> -> , id <param_next>
        case TYPE_COMMA:
            GET_NEXT_TOKEN();
            CHECK_TOKEN(TYPE_IDENTIFIER);
            if(htSearch(Gtable, token.attribute.str) != NULL ||
               htSearch(Ltable, token.attribute.str) != NULL) {
                return SEMANTIC_DEF_ERROR;
            }
            new_variable = htInsert(Ltable, token.attribute.str, &internalError);
            if(internalError) {
                return INTERNAL_ERROR;
            }
            new_variable->data.type = TYPE_UNDEFINED;
            if(new_function->data.isDefined) {
                string_add_char(&new_function->data.parameters, 'u');
            }
            sprintf(index_param.str, "%d", ++counter);
            generate_new_param(token.attribute.str, index_param.str);
            string_clear(&index_param);
            return param_next(new_function);

        /// Rule 18: <param_next> -> ϵ
        case TYPE_RIGHT_BRACKET:
            if(new_function->data.isDefined == false) {
                if(new_function->data.parameters.length != counter) {
                    return SEMANTIC_DEF_ERROR;
                }
                new_function->data.isDefined = true;
            }
            return SYNTAX_OK;
        default:
            return SYNTAX_ERROR;
    }
}

int param_list(tHTItem *new_function) {
    GET_NEXT_TOKEN();
    bool internalError = false;
    tHTItem *new_variable;
    switch(token.type) {
        case TYPE_IDENTIFIER:
            if(htSearch(Gtable, token.attribute.str) != NULL ||
               htSearch(Ltable, token.attribute.str) != NULL) {
                return SEMANTIC_DEF_ERROR;
            }
            new_variable = htInsert(Ltable, token.attribute.str, &internalError);
            if(internalError) {
                return INTERNAL_ERROR;
            }
            new_variable->data.type = TYPE_UNDEFINED;
            if(new_function->data.isDefined) {
                string_add_char(&(new_function->data.parameters), 'u');
            }
            sprintf(index_param.str, "%d", 1);
            generate_new_param(token.attribute.str, index_param.str);
            string_clear(&index_param);
            return param_next(new_function);
        case TYPE_RIGHT_BRACKET:
            if(new_function->data.isDefined == false) {
                if(new_function->data.parameters.length != 0) {
                    return SEMANTIC_DEF_ERROR;
                }
                new_function->data.isDefined = true;
            }
            return SYNTAX_OK;
        default:
            return SYNTAX_ERROR;
    }
}

int print_arguments() {
    if(IS_TOKEN(TYPE_RIGHT_BRACKET) || IS_TOKEN(TYPE_EOL)) {
        return SEMANTIC_PARAM_ERROR;
    }
    tHTItem *symbol;
    String_DYNAMIC outputString;
    string_init(&outputString);

    if(IS_TOKEN_TERM(token.type)) {
        while(true) {
            if(!(IS_TOKEN_TERM(token.type))) { return SYNTAX_ERROR; }
            if(IS_TOKEN(TYPE_IDENTIFIER)) {
                if(inFunction) {
                    if((symbol = htSearch(Ltable, token.attribute.str)) == NULL) {
                        GET_NEXT_TOKEN();
                        return SEMANTIC_DEF_ERROR;
                    }
                } else {
                    if((symbol = htSearch(Gtable, token.attribute.str)) == NULL) {
                        GET_NEXT_TOKEN();
                        return SEMANTIC_DEF_ERROR;
                    }
                }
                if(symbol->data.isFunction) { return SEMANTIC_OTHERS_ERROR; }
                switch(symbol->data.type) {
                    case TYPE_INTEGER:
                        generate_WRITE_id(symbol->key);
                        break;
                    case TYPE_FLOAT:
                        generate_WRITE_id(symbol->key);
                        break;
                    case TYPE_STRING:
                        generate_WRITE_id(symbol->key);
                        break;
                    case TYPE_UNDEFINED:
                        generate_WRITE_id(symbol->key);
                        break;
                }
            } else {
                switch(token.type) {
                    case TYPE_TOKEN_INTEGER:
                        sprintf(value_str.str, "%d", token.attribute.valueInt);
                        generate_WRITE(value_str.str);
                        string_clear(&value_str);
                        break;
                    case TYPE_TOKEN_FLOAT:
                        sprintf(value_str.str, "%a", token.attribute.valueDouble);
                        generate_WRITE(value_str.str);
                        string_clear(&value_str);
                        break;
                    case TYPE_TOKEN_STRING:
                        convert_string2codeString(token.attribute.str, &outputString);
                        generate_WRITE(outputString.str);
                        string_clear(&outputString);
                        break;

                }
            }
            GET_NEXT_TOKEN();
            if(IS_TOKEN(TYPE_COMMA)) {
                GET_NEXT_TOKEN();
                continue;
            } else if(IS_TOKEN(TYPE_EOL) || IS_TOKEN(TYPE_RIGHT_BRACKET)) {
                return SYNTAX_OK;
            }
            return SYNTAX_ERROR;
        }
    }
    return SYNTAX_ERROR;
}

int arguments(tHTItem *function) {
    tHTItem *symbol;
    String_DYNAMIC outputString;
    string_init(&outputString);
    result = SYNTAX_OK;
    // test if function has no parameters (func())
    if(function->data.parameters.length == 0) {
        GET_NEXT_TOKEN();
        return SYNTAX_OK;
    } else {
        if(IS_TOKEN(TYPE_RIGHT_BRACKET) || IS_TOKEN(TYPE_EOL)) {
            return SEMANTIC_PARAM_ERROR;
        }
    }

    if(IS_TOKEN_TERM(token.type)) {
        int i;
        for(i=0; i < function->data.parameters.length; i++) {
            sprintf(index_param.str, "%d", i+1);
            if(i != 0) {
                if(IS_TOKEN(TYPE_RIGHT_BRACKET) || IS_TOKEN(TYPE_EOL)) {
                    return SEMANTIC_PARAM_ERROR;
                }
                CHECK_TOKEN(TYPE_COMMA);
                GET_NEXT_TOKEN()
            }
            if(!(IS_TOKEN_TERM(token.type))) { return SYNTAX_ERROR; }
            if(IS_TOKEN(TYPE_IDENTIFIER)) {
                if(inFunction) {
                    if((symbol=htSearch(Ltable, token.attribute.str)) == NULL) {
                        GET_NEXT_TOKEN();
                        result = SEMANTIC_DEF_ERROR;
                        continue;
                    }
                } else {
                    if((symbol=htSearch(Gtable, token.attribute.str)) == NULL) {
                        GET_NEXT_TOKEN();
                        result =  SEMANTIC_DEF_ERROR;
                        continue;
                    }
                }
                if(symbol->data.isFunction) { return SEMANTIC_OTHERS_ERROR; }

                switch((function->data.parameters.str)[i]) {
                    case 'i':
                        if(symbol->data.type != TYPE_INTEGER) {
                            GET_NEXT_TOKEN();
                            result = SEMANTIC_OTHERS_ERROR;
                            continue;
                        }
                        generate_new_argument_id(token.attribute.str, index_param.str);
                        GET_NEXT_TOKEN();
                        break;
                    case 'f':
                        if(symbol->data.type != TYPE_FLOAT) {
                            GET_NEXT_TOKEN();
                            result = SEMANTIC_OTHERS_ERROR;
                            continue;
                        }
                        generate_new_argument_id(token.attribute.str, index_param.str);
                        GET_NEXT_TOKEN();
                        break;
                    case 's':
                        if(symbol->data.type != TYPE_STRING) {
                            GET_NEXT_TOKEN();
                            result = SEMANTIC_OTHERS_ERROR;
                            continue;
                        }
                        generate_new_argument_id(token.attribute.str, index_param.str);
                        GET_NEXT_TOKEN();
                        break;
                    default:
                        return SEMANTIC_OTHERS_ERROR;
                }
                string_clear(&index_param);
                continue;
            }
            // VALUE INT,FLOAT,STRING
            switch((function->data.parameters.str)[i]) {
                case 'i':
                    if(token.type != TYPE_TOKEN_INTEGER) {
                        GET_NEXT_TOKEN();
                        result = SEMANTIC_OTHERS_ERROR;
                        continue;
                    }
                    sprintf(value_str.str, "%d", token.attribute.valueInt);
                    generate_new_argument_value(value_str.str, "int", index_param.str);
                    string_clear(&value_str);
                    GET_NEXT_TOKEN();
                    break;
                case 'f':
                    if(token.type != TYPE_TOKEN_FLOAT) {
                        GET_NEXT_TOKEN();
                        result =  SEMANTIC_OTHERS_ERROR;
                        continue;
                    }
                    sprintf(value_str.str, "%a", token.attribute.valueDouble);
                    generate_new_argument_value(value_str.str, "float", index_param.str);
                    string_clear(&value_str);
                    GET_NEXT_TOKEN();
                    break;
                case 's':
                    if(token.type != TYPE_TOKEN_STRING) {
                        GET_NEXT_TOKEN();
                        result =  SEMANTIC_OTHERS_ERROR;
                        continue;
                    }
                    convert_string2codeString(token.attribute.str, &outputString);
                    generate_new_argument_value(outputString.str, "string", index_param.str);
                    string_clear(&outputString);
                    GET_NEXT_TOKEN();
                    break;
                default:
                    return SEMANTIC_OTHERS_ERROR;
            }
            // continues will be changed to breaks
            //generate_new_argument_value(inMain, ... , index); // i = index
            string_clear(&index_param);
        }
        if(i == function->data.parameters.length) {
            if(IS_TOKEN(TYPE_RIGHT_BRACKET) || IS_TOKEN(TYPE_EOL)) {
                return result;
            } else if(IS_TOKEN(TYPE_COMMA)) {
                GET_NEXT_TOKEN();
                if(IS_TOKEN_TERM(token.type)) {
                    GET_NEXT_TOKEN();
                    return SEMANTIC_PARAM_ERROR;
                }
                return SYNTAX_ERROR;
            }
            return SYNTAX_ERROR;
        }
        return SEMANTIC_PARAM_ERROR;
    }
    return SYNTAX_ERROR;
}

int argument_list(tHTItem *function) {
    GET_NEXT_TOKEN();
    if(IS_TOKEN(TYPE_ASSIGMENT)) {
        return SEMANTIC_DEF_ERROR;
    }
    if(IS_TOKEN(TYPE_LEFT_BRACKET)) {
        GET_NEXT_TOKEN();
        if(!(result=arguments(function))){
            CHECK_TOKEN(TYPE_RIGHT_BRACKET);
            GET_NEXT_TOKEN()
            return SYNTAX_OK;
        }
        CHECK_TOKEN(TYPE_RIGHT_BRACKET);
        return result;
    } else if(IS_TOKEN_TERM(token.type)) {
        if(!(result=arguments(function))){
            CHECK_TOKEN(TYPE_EOL);
            return SYNTAX_OK;
        }
        CHECK_TOKEN(TYPE_EOL);
        return result;
    }
    return SYNTAX_ERROR;
}

int arguments_ID(tHTItem *function) {
    tHTItem *symbol;
    // test if function has no parameters (func())
    if (function->data.parameters.length == 0) {
        if(IS_TOKEN_TERM(token.type)) {
            return SEMANTIC_PARAM_ERROR;
        }
        return SYNTAX_OK;
    } else {
        if (IS_TOKEN(TYPE_RIGHT_BRACKET) || IS_TOKEN(TYPE_EOL)) {
            return SEMANTIC_PARAM_ERROR;
        }
    }

    if (IS_TOKEN_TERM(token.type)) {
        int i;
        for (i = 0; i < function->data.parameters.length; i++) {
            sprintf(index_param.str, "%d", i+1);
            if (i != 0) {
                if (IS_TOKEN(TYPE_RIGHT_BRACKET) || IS_TOKEN(TYPE_EOL)) {
                    return SEMANTIC_PARAM_ERROR;
                }
                CHECK_TOKEN(TYPE_COMMA);
                GET_NEXT_TOKEN()
            }
            if (!(IS_TOKEN_TERM(token.type))) { return SYNTAX_ERROR; }
            if (IS_TOKEN(TYPE_IDENTIFIER)) {
                if (inFunction) {
                    if ((symbol = htSearch(Ltable, token.attribute.str)) == NULL) {
                        return SEMANTIC_DEF_ERROR;
                    }
                } else {
                    if ((symbol = htSearch(Gtable, token.attribute.str)) == NULL) {
                        return SEMANTIC_DEF_ERROR;
                    }
                }
                if (symbol->data.isFunction) { return SEMANTIC_OTHERS_ERROR; }
                switch(symbol->data.type) {
                    case TYPE_INTEGER:
                        generate_new_argument_id(token.attribute.str, index_param.str);
                        break;
                    case TYPE_FLOAT:
                        generate_new_argument_id(token.attribute.str, index_param.str);
                        break;
                    case TYPE_STRING:
                        generate_new_argument_id(token.attribute.str, index_param.str);
                        break;
                    case TYPE_UNDEFINED:
                        generate_new_argument_id(token.attribute.str, "nil");
                        break;
                }
            } else {
                switch(token.type) {
                    case TYPE_TOKEN_INTEGER:
                        sprintf(value_str.str, "%d", token.attribute.valueInt);
                        generate_new_argument_value(value_str.str, "int", index_param.str);
                        string_clear(&value_str);
                        break;
                    case TYPE_TOKEN_FLOAT:
                        sprintf(value_str.str, "%a", token.attribute.valueDouble);
                        generate_new_argument_value(value_str.str, "float", index_param.str);
                        string_clear(&value_str);
                        break;
                    case TYPE_TOKEN_STRING:
                        generate_new_argument_value(token.attribute.str, "string", index_param.str);
                        break;
                    default:
                        generate_new_argument_value(token.attribute.str, "nil", "nil");
                        break;
                }
            }
            GET_NEXT_TOKEN();
            string_clear(&index_param);
        }
        if (i == function->data.parameters.length) {
            if (IS_TOKEN(TYPE_RIGHT_BRACKET) || IS_TOKEN(TYPE_EOL)) {
                return SYNTAX_OK;
            } else if (IS_TOKEN(TYPE_COMMA)) {
                GET_NEXT_TOKEN();
                if (IS_TOKEN_TERM(token.type)) {
                    GET_NEXT_TOKEN();
                    return SEMANTIC_PARAM_ERROR;
                }
                return SYNTAX_ERROR;
            }
            return SYNTAX_ERROR;
        }
        return SEMANTIC_PARAM_ERROR;
    }
    return SYNTAX_ERROR;
}

int argument_list_ID(tHTItem *function) {
    GET_NEXT_TOKEN();
    if(IS_TOKEN(TYPE_ASSIGMENT)) {
        return SEMANTIC_DEF_ERROR;
    }

    if(IS_TOKEN(TYPE_LEFT_BRACKET)) {
        GET_NEXT_TOKEN();
        if(!(result=arguments_ID(function))){
            CHECK_TOKEN(TYPE_RIGHT_BRACKET);
            GET_NEXT_TOKEN()
            return SYNTAX_OK;
        }
        return result;
    } else if(IS_TOKEN_TERM(token.type)) {
        if(!(result=arguments_ID(function))){
            CHECK_TOKEN(TYPE_EOL);
            return SYNTAX_OK;
        }
        return result;
    } else if(IS_TOKEN(TYPE_EOL)) {
        if(function->data.parameters.length == 0) {
            return SYNTAX_OK;
        }
    }
    return SYNTAX_ERROR;
}

int fnc_call() {
    tHTItem *symbol;

    if(IS_TOKEN_KEYWORD("inputs") ||
       IS_TOKEN_KEYWORD("inputf") ||
       IS_TOKEN_KEYWORD("inputi")) {

        // current token is inputs/f/i keyword
        if(strcmp(token.attribute.str, "inputs") == 0) {
            generate_READ(l_value_id->key, "string");
        }
        else if(strcmp(token.attribute.str, "inputf") == 0) {
            generate_READ(l_value_id->key, "float");
        }
        else {
            generate_READ(l_value_id->key, "int");
        }

        GET_NEXT_TOKEN();
        if(IS_TOKEN(TYPE_ASSIGMENT)) {
            return SEMANTIC_DEF_ERROR;
        }
        if(IS_TOKEN(TYPE_EOL)) {
            return SYNTAX_OK;
        }
        CHECK_TOKEN(TYPE_LEFT_BRACKET);
        GET_NEXT_TOKEN();
        CHECK_TOKEN(TYPE_RIGHT_BRACKET);
        GET_NEXT_TOKEN(); // for state_list(), statement() call
        return SYNTAX_OK;
    }
    else if(IS_TOKEN_KEYWORD("print")) {
        GET_NEXT_TOKEN();
        if(IS_TOKEN(TYPE_ASSIGMENT)) {
            return SEMANTIC_DEF_ERROR;
        }

        if(IS_TOKEN(TYPE_LEFT_BRACKET)) {
            GET_NEXT_TOKEN()
            if(!(result=print_arguments())) {
                CHECK_TOKEN(TYPE_RIGHT_BRACKET);
                GET_NEXT_TOKEN();
                return SYNTAX_OK;
            }
            CHECK_TOKEN(TYPE_RIGHT_BRACKET);
            GET_NEXT_TOKEN();
            return result;
        }
        if(!(result=print_arguments())) {
            CHECK_TOKEN(TYPE_EOL);
            return SYNTAX_OK;
        }
        CHECK_TOKEN(TYPE_EOL);
        return result;
    }
    else if(IS_TOKEN_KEYWORD("length") || IS_TOKEN_KEYWORD("substr") ||
           IS_TOKEN_KEYWORD("ord") || IS_TOKEN_KEYWORD("chr")) {

        if(((symbol=htSearch(Gtable, token.attribute.str)) == NULL) ||
           !(symbol->data.isFunction)) {
            return SEMANTIC_DEF_ERROR;
        }
        generate_create_frame(); //CREATEFRAME
        if(!(result = argument_list(symbol))) {
            generate_function_call(symbol->key);
            if(l_value_fnc != NULL) {
                generate_MOVE_retval_to_Lvalue(l_value_fnc->key);
            }
            l_value_fnc = NULL;
            return SYNTAX_OK;
        }
        return result;
    } else if(IS_TOKEN(TYPE_IDENTIFIER)) {
        if(((symbol=htSearch(Gtable, token.attribute.str)) == NULL) ||
           !(symbol->data.isFunction)) {
            return SEMANTIC_DEF_ERROR;
        }
        generate_create_frame(); //CREATEFRAME
        if(!(result = argument_list_ID(symbol))) {
            generate_function_call(symbol->key);
            if(l_value_fnc != NULL) {
                generate_MOVE_retval_to_Lvalue(l_value_fnc->key);
            }
            return SYNTAX_OK;
        }
        return result;
    }
    return SYNTAX_ERROR;
}

int id_assign(tHTItem *fnc) {
    GET_NEXT_TOKEN();
    if(IS_TOKEN(TYPE_IDENTIFIER) || IS_TOKEN(TYPE_KEYWORD)) {
        tHTItem *symbol;
        if (((symbol = htSearch(Gtable, token.attribute.str)) != NULL) &&
            symbol->data.isFunction &&
            symbol->data.isDefined) {
            if (l_value_id->data.type == TYPE_UNDEFINED) {
                l_value_id->data.type = symbol->data.type;
            } else {
                if (l_value_id->data.type != symbol->data.type) {
                    return SEMANTIC_DEF_ERROR;
                }
            }
            l_value_fnc = l_value_id;
            return fnc_call();
        } else {
            if (((symbol = htSearch(Gtable, token.attribute.str)) != NULL) &&
                (symbol->data.isFunction) && !(symbol->data.isDefined)) {
                if (inFunction) {
                    return SEMANTIC_DEF_ERROR;
                } else {
                    return SEMANTIC_DEF_ERROR;
                }
            }
            if(IS_TOKEN_KEYWORD("nil")) {
                generate_NIL_value(l_value_id->key);
                GET_NEXT_TOKEN();
                return result;
            }
            if (inFunction) {
                if ((htSearch(Ltable, token.attribute.str)) != NULL) {
                    tokenBuffer[1] = token;
                    if ((result = expression(tokenBuffer, 1, fnc))) {
                        GET_NEXT_TOKEN();
                        return result;
                    }
                    return SYNTAX_OK;
                } else if(l_value_id->key == token.attribute.str) {
                    l_value_id->data.type = TYPE_UNDEFINED;
                    generate_NIL_value(l_value_id->key);
                    return SYNTAX_OK;
                }
                else {
                    return SEMANTIC_DEF_ERROR;
                }
            } else {
                if (((symbol = htSearch(Gtable, token.attribute.str)) != NULL) &&
                    !(symbol->data.isFunction)) {
                    tokenBuffer[1] = token;
                    if ((result = expression(tokenBuffer, 1, NULL))) {
                        GET_NEXT_TOKEN();
                        return result;
                    }
                    return SYNTAX_OK;
                } else if(l_value_id->key == token.attribute.str) {
                    l_value_id->data.type = TYPE_UNDEFINED;
                    generate_NIL_value(l_value_id->key);
                    return SYNTAX_OK;
                }
                else {
                    return SEMANTIC_DEF_ERROR;
                }
            }
        }
    }
    tokenBuffer[1] = token;
    if ((result = expression(tokenBuffer, 1, fnc))) {
        return result;
    }
    return SYNTAX_OK;
}

/*
 * @pre next_token must be received
 */
int statement(tHTItem *fnc) {
    static unsigned indexLable = 0;
    char currentLable[8] = {};
    sprintf(currentLable, "%d", indexLable++);

    /// Rule 9: <statement> -> if <expr> then EOL <state_list> else EOL <state_list> end
    if(IS_TOKEN_KEYWORD("if")) {
        generate_if_head();
        inIF_OR_inWHILE_condition = true;
        if(!(result = expression(tokenBuffer, BufferTCounter, fnc))) {
            inIF_OR_inWHILE_condition = false;
            generate_conditional_jump(IN_IF_STATMENT, currentLable);
            CHECK_TOKEN_KEYWORD("then");
        }
        else {
            GET_NEXT_TOKEN();
            return result;
        }
        GET_NEXT_TOKEN();
        CHECK_TOKEN(TYPE_EOL);
        if(!(result = state_list(fnc))) {
            //GET_NEXT_TOKEN(); // already next token received in state_list()
            CHECK_TOKEN_KEYWORD("else");
            generate_else(currentLable); // ########## ELSE ########## + label
            GET_NEXT_TOKEN();
            CHECK_TOKEN(TYPE_EOL);
            if (!(result = state_list(fnc))) {
                //GET_NEXT_TOKEN(); // already next token received in state_list()
                generate_if_end(currentLable);
                CHECK_TOKEN_KEYWORD("end");
                GET_NEXT_TOKEN();   // for state_list(), statement() call
                return SYNTAX_OK;
            }
        }
        return result;
    }
    /// Rule 10: <statement> -> while <expr> do EOL <state_list> end
    else if(IS_TOKEN_KEYWORD("while")) {
        generate_while_head(currentLable);
        inIF_OR_inWHILE_condition = true;
        if(!(result = expression(tokenBuffer, BufferTCounter, fnc))) {
            inIF_OR_inWHILE_condition = false;
            generate_conditional_jump(IN_WHILE_STATEMENT, currentLable); // 1
            CHECK_TOKEN_KEYWORD("do")
        } // need to pass Gtable,Ltable,inDef...
        else {
            GET_NEXT_TOKEN();
            return result;
        }
        GET_NEXT_TOKEN();
        CHECK_TOKEN(TYPE_EOL);
        if (!(result = state_list(fnc))) {
            //GET_NEXT_TOKEN(); // already next token received in state_list()
            generate_while_end(currentLable);
            CHECK_TOKEN_KEYWORD("end");
            GET_NEXT_TOKEN();   // for state_list(), statement() call
            return SYNTAX_OK;
        }
    }
    /// Rule 13: <statement> -> <fnc_call>
    else if(IS_TOKEN_KEYWORD("inputs") ||
            IS_TOKEN_KEYWORD("inputf") ||
            IS_TOKEN_KEYWORD("inputi") ||
            IS_TOKEN_KEYWORD("print")  ||
            IS_TOKEN_KEYWORD("length") ||
            IS_TOKEN_KEYWORD("substr") ||
            IS_TOKEN_KEYWORD("ord")    ||
            IS_TOKEN_KEYWORD("chr")) {

        return fnc_call();
    }
    else if(IS_TOKEN(TYPE_IDENTIFIER)) {
        // in MAIN
        tHTItem *symbol;
        bool internalError = false;

        if(inFunction) {
            if(((symbol=htSearch(Gtable, token.attribute.str)) != NULL) &&
                 symbol->data.isFunction && symbol->data.isDefined) {

                return fnc_call();
            } else if((symbol=htSearch(Ltable, token.attribute.str)) != NULL) {
                /// is variable
                l_value_id = symbol;
                tokenBuffer[0] = token;
                string_clear(&value_str);
                string_add_Cstr(&value_str, token.attribute.str);

                GET_NEXT_TOKEN();
                if(IS_TOKEN(TYPE_ASSIGMENT)) { // '='
                    return id_assign(fnc);
                } else if(is_token_operator(token.type)) {
                    tokenBuffer[0] = token;
                    tokenBuffer[0].attribute.str = value_str.str;
                    BufferTCounter = 2;
                    if(!(result = expression(tokenBuffer, BufferTCounter, fnc))) { // 2 tokens already received
                        BufferTCounter = 0;
                        CHECK_TOKEN(TYPE_EOL);
                        return SYNTAX_OK;
                    }
                    return result;
                } else {
                    tokenBuffer[0].attribute.str = value_str.str;
                    tokenBuffer[1] = token;
                    if(!(result = expression(tokenBuffer, 2, fnc))) { // 2 tokens already received
                        BufferTCounter = 0;
                        CHECK_TOKEN(TYPE_EOL);
                        return SYNTAX_OK;
                    }
                    return result;
                }
            } else {
                String_DYNAMIC previous_id;
                if(string_init(&previous_id)) { return INTERNAL_ERROR; }
                if(string_copy_Cstr(&previous_id, token.attribute.str)) { return INTERNAL_ERROR; }
                GET_NEXT_TOKEN();
                if(IS_TOKEN(TYPE_ASSIGMENT)) { // prev_id is variable -> Ltable
                    symbol = htInsert(Ltable, string_get_string(&previous_id), &internalError);
                    if(internalError) {
                        return INTERNAL_ERROR;
                    }
                    generate_DEFVAR(symbol->key);
                    symbol->data.isFunction = false;
                    symbol->data.type = TYPE_UNDEFINED;

                    l_value_id = symbol;

                    return id_assign(fnc); // return of fncCall or expresion is type of previous_id
                } else if(IS_TOKEN(TYPE_LEFT_BRACKET) || IS_TOKEN_TERM(token.type)) {

                    symbol = htInsert(Gtable, string_get_string(&previous_id), &internalError);
                    if(internalError) {
                        return INTERNAL_ERROR;
                    }
                    symbol->data.isFunction = true;
                    symbol->data.isDefined = false;
                    symbol->data.type = TYPE_UNDEFINED;
                    if(string_init(&symbol->data.parameters)) { return INTERNAL_ERROR; }
                    if(IS_TOKEN(TYPE_LEFT_BRACKET)) {
                        GET_NEXT_TOKEN();
                        if(IS_TOKEN(TYPE_RIGHT_BRACKET)) { // if function is foo() without parameters
                            GET_NEXT_TOKEN()
                            return SYNTAX_OK;
                        }
                        while(IS_TOKEN_TERM(token.type)) {
                            if (string_add_char(&symbol->data.parameters, 'u')) {
                                return INTERNAL_ERROR;
                            }
                            GET_NEXT_TOKEN();
                            if(IS_TOKEN(TYPE_RIGHT_BRACKET)) {
                                GET_NEXT_TOKEN()
                                return SYNTAX_OK;
                            }
                            CHECK_TOKEN(TYPE_COMMA);
                            GET_NEXT_TOKEN();
                        }
                        return SYNTAX_ERROR;
                    } else { // token is term
                        if(IS_TOKEN(TYPE_EOL)) { // if function is foo() without parameters
                            GET_NEXT_TOKEN()
                            return SYNTAX_OK;
                        }
                        while(IS_TOKEN_TERM(token.type)) {
                            if (string_add_char(&symbol->data.parameters, 'u')) {
                                return INTERNAL_ERROR;
                            }
                            GET_NEXT_TOKEN();
                            if(IS_TOKEN(TYPE_EOL)) {
                                return SYNTAX_OK;
                            }
                            CHECK_TOKEN(TYPE_COMMA);
                            GET_NEXT_TOKEN();
                        }
                        return SYNTAX_ERROR;
                    }
                } else {
                    return SEMANTIC_DEF_ERROR;
                }
            }
        } else {
            if((symbol=htSearch(Gtable, token.attribute.str)) != NULL) {
                if(symbol->data.isFunction) {
                    if(symbol->data.isDefined){
                        return fnc_call();
                    }
                    return SEMANTIC_DEF_ERROR;
                }
                else {
                    l_value_id = symbol;
                    tokenBuffer[0] = token;
                    string_clear(&value_str);
                    string_add_Cstr(&value_str, token.attribute.str);

                    GET_NEXT_TOKEN();
                    if(IS_TOKEN(TYPE_ASSIGMENT)) { // '='
                        return id_assign(NULL);
                    }
                    else if(is_token_operator(token.type)) {
                        tokenBuffer[1] = token;
                        tokenBuffer[0].attribute.str = value_str.str;
                        BufferTCounter = 2;
                        if(!(result = expression(tokenBuffer, BufferTCounter, fnc))) {
                            BufferTCounter = 0;
                            CHECK_TOKEN(TYPE_EOL);
                            return SYNTAX_OK;
                        }
                        else {
                            return result;
                        }
                    }
                    else if(IS_TOKEN(TYPE_EOL)) {
                        tokenBuffer[1] = token;
                        tokenBuffer[0].attribute.str = value_str.str;
                        BufferTCounter = 2;
                        if(!(result = expression(tokenBuffer, BufferTCounter, fnc))) {
                            BufferTCounter = 0;
                            CHECK_TOKEN(TYPE_EOL);
                            return SYNTAX_OK;
                        }
                        else {
                            return result;
                        }
                    }
                    else {
                        return SYNTAX_ERROR;
                    }
                }
            } else {
                symbol = htInsert(Gtable, token.attribute.str, &internalError);
                if(internalError) {
                    return INTERNAL_ERROR;
                }
                generate_DEFVAR(symbol->key);
                symbol->data.isFunction = false;
                symbol->data.type = TYPE_UNDEFINED;
                l_value_id = symbol;

                GET_NEXT_TOKEN()
                if(IS_TOKEN(TYPE_ASSIGMENT)) {
                    return id_assign(NULL); // return of fncCall or expresion is type of l_value_id
                }
                else if(is_token_operator(token.type)) {
                    return SEMANTIC_DEF_ERROR;
                }
                else {
                    return SEMANTIC_DEF_ERROR; // ERROR or created new variable type nil
                }
            }
        }
    }
    switch(token.type) {
        case TYPE_TOKEN_INTEGER:
        case TYPE_TOKEN_FLOAT:
        case TYPE_TOKEN_STRING:
            tokenBuffer[1] = token;
            BufferTCounter = 1;
            if(!(result = expression(tokenBuffer, BufferTCounter, fnc))) {
                BufferTCounter = 0;
                CHECK_TOKEN(TYPE_EOL);
                return SYNTAX_OK;
            }
            else {
                return result;
            }
        default:
            return SYNTAX_ERROR;
    }
}

int state_list(tHTItem *symbol) {
    GET_NEXT_TOKEN();
    /// Rule 7: <state_list> -> ϵ
    if((IS_TOKEN_KEYWORD("end")) ||
        IS_TOKEN_KEYWORD("else")) {
        return SYNTAX_OK;
    }
    /// Rule 8: <state_list> -> EOL <state_list>
    else if(IS_TOKEN(TYPE_EOL)) {
        return state_list(symbol);
    }
    /// Rule 6: <state_list> -> <statement> EOL <state_list>
    else if(!(result = statement(symbol))) {
        CHECK_TOKEN(TYPE_EOL);
        return state_list(symbol);
    }
    else {
        return result;
    }
}

int def() {
    GET_NEXT_TOKEN();

    /// Rule 5: <def> -> id (<param_list>) EOL <state_list> end
    CHECK_TOKEN(TYPE_IDENTIFIER);
    tHTItem *new_function;
    tHTItem *symbol;
    if((symbol=htSearch(Gtable, token.attribute.str)) == NULL) {
        bool internalError = false;
        new_function = htInsert(Gtable, token.attribute.str, &internalError);
        if(internalError) {
            return INTERNAL_ERROR;
        }
        generate_function_start(token.attribute.str);
        new_function->data.isDefined = true;
        new_function->data.isFunction = true;
        string_init(&new_function->data.parameters);
    } else if(!(symbol->data.isFunction && !(symbol->data.isDefined))) {
        return SEMANTIC_DEF_ERROR;
    }

    GET_NEXT_TOKEN();
    CHECK_TOKEN(TYPE_LEFT_BRACKET);
    if(!(result = param_list((symbol==NULL)?(new_function):(symbol)))) {
        CHECK_TOKEN(TYPE_RIGHT_BRACKET);
        GET_NEXT_TOKEN();
        CHECK_TOKEN(TYPE_EOL);
        inFunction = true;
        if(!(result = state_list((symbol==NULL)?(new_function):(symbol)))) {
            generate_update_return();
            generate_function_end();
            CHECK_TOKEN_KEYWORD("end");
            htClearAll(Ltable);
            return SYNTAX_OK;
        } else {
            return result;
        }
    }
    return result;
}

int program() {

    switch(token.type) {
        /// Rule 1: <program> -> EOF
        case TYPE_EOF:
            return SYNTAX_OK;
        /// Rule 2: <program> -> EOL <program>
        case TYPE_EOL:
            GET_NEXT_TOKEN();
            return program();
        /// Rule 3: <program> -> def <def> EOL <program>
        case TYPE_KEYWORD:
            if(strcmp(token.attribute.str, "def") == 0) {
                if(!(result = def())) {
                    inFunction = false;
                    GET_NEXT_TOKEN();
                    CHECK_TOKEN(TYPE_EOL);
                    GET_NEXT_TOKEN();
                    return program();
                }
                else {
                    return result;
                }
            }
            /// Rule 4: <program> -> <statement> EOL <program>
            else {
                if(!(result = statement(NULL))) {
                    CHECK_TOKEN(TYPE_EOL);
                    GET_NEXT_TOKEN();
                    return program();
                }
                else {
                    return result;
                }
            }
        /// Rule 4: <program> -> <statement> EOL <program>
        case TYPE_IDENTIFIER:
            if(!(result = statement(NULL))) {
                CHECK_TOKEN(TYPE_EOL);
                GET_NEXT_TOKEN();
                return program();
            }
            else {
                return result;
            }
        case TYPE_TOKEN_INTEGER:
        case TYPE_TOKEN_FLOAT:
        case TYPE_TOKEN_STRING:
            tokenBuffer[1] = token;
            BufferTCounter = 1;
            if(!(result = expression(tokenBuffer, BufferTCounter, NULL))) {
                BufferTCounter = 0;
                CHECK_TOKEN(TYPE_EOL);
                return program();
            }
            else {
                return result;
            }
        default:
            return SYNTAX_ERROR;
    }
}

int parse(tHTable *global_table, tHTable *local_table) {

    Gtable = global_table;
    Ltable = local_table;
    string_init(&index_param);
    string_init(&value_str);

    if(init_Gtable()){
        return INTERNAL_ERROR;
    }
    generator_start();
    if((result = get_next_token(&token)) != LEXER_ERROR) {
        result = program();
    }
    if(!result) {
        generator_code_printout();
    }
    return result;
}
