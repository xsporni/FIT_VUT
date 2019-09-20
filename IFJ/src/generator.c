/**
 * @file generator.c
 * @brief Implementation of GENERATOR module
 *
 * @authors Richard Borbély | xborbe00 | xborbe00@stud.fit.vutbr.cz
 *
 * @date 27 November 2018
 */

#include "generator.h"
#include "parser.h"
#include "parser_expressions.h"

// -------------- Extern Global variables ------------- //
extern bool inFunction;

// ----------------- Global variables ---------------- //
String_DYNAMIC functions_s;
String_DYNAMIC main_s;

void convert_string2codeString(char *inputString, String_DYNAMIC *outputString) {
    char c;
    for(int i=0; i < strlen(inputString); i++) {
        c = inputString[i];
        switch(c) {
            case ' ':
                string_add_Cstr(outputString, "\\032");
                break;
            case '#':
                string_add_Cstr(outputString, "\\035");
                break;
            case '\n':
                string_add_Cstr(outputString, "\\010");
                break;
            case '\t':
                string_add_Cstr(outputString, "\\009");
                break;
            default:
                string_add_char(outputString, c);
                break;
        }
    }
}


//  --------------- Built-In functions --------------- //
// Function length(s)
#define FUNCTION_LENGTH                                 \
        "\n#Body of function length(s) [%1]"            \
        "\nLABEL $length"                               \
        "\nPUSHFRAME"                                   \
        "\nDEFVAR LF@$return"                           \
        "\nSTRLEN LF@$return LF@%1"                     \
        "\nPOPFRAME"                                    \
        "\nRETURN\n"

// Function chr(i)
#define FUNCTION_CHR                                    \
        "\n#Body of function chr(i) [%1]"               \
        "\nLABEL $chr"                                  \
        "\nPUSHFRAME"                                   \
        "\nDEFVAR LF@$return"                           \
        "\nDEFVAR LF@badsize"                           \
        "\nLT LF@badsize LF@%1 int@0"                   \
        "\nJUMPIFEQ $nomatch LF@badsize bool@true"      \
        "\nGT LF@badsize LF@%1 int@255"                 \
        "\nJUMPIFEQ $nomatch LF@badsize bool@true"      \
        "\nINT2CHAR LF@$return LF@%1"                   \
        "\nJUMP $match"                                 \
        "\nLABEL $nomatch"                              \
        "\nEXIT int@4"                                  \
        "\nLABEL $match"                                \
        "\nPOPFRAME"                                    \
        "\nRETURN\n"

// Function substr(s, i, n)
#define FUNCTION_SUBSTR                                 \
        "\n#Body of function substr(s,i,n) [%1,%2,%3]"  \
        "\nLABEL $substr"                               \
        "\nPUSHFRAME"                                   \
        "\nDEFVAR LF@$return"                           \
        "\nDEFVAR LF@length"                            \
        "\nDEFVAR LF@help"                              \
        "\nDEFVAR LF@string"                            \
        "\nDEFVAR LF@bool"                              \
        "\nDEFVAR LF@index"                             \
        "\nMOVE LF@string string@"                      \
        "\nMOVE LF@index LF@%2"                         \
        "\nSTRLEN LF@length LF@%1"                      \
        "\nGT LF@bool LF@%2 LF@length"                  \
        "\nJUMPIFEQ $fatal_end$ LF@bool bool@true"      \
        "\nLT LF@bool LF@%2 int@0"                      \
        "\nJUMPIFEQ $fatal_end$ LF@bool bool@true"      \
        "\nLT LF@bool LF@%3 int@0"                      \
        "\nJUMPIFEQ $fatal_end$ LF@bool bool@true"      \
        "\nSUB LF@length LF@length LF@index"            \
        "\nGT LF@bool LF@%3 LF@length"                  \
        "\nSTRLEN LF@length LF@%1"                      \
        "\nJUMPIFEQ $big_n$ LF@bool bool@true"          \
        "\nJUMP $its_ok$"                               \
        "\nLABEL $big_n$"                               \
        "\nLT LF@bool LF@index LF@length"               \
        "\nJUMPIFEQ $the_end$ LF@bool bool@false"       \
        "\nGETCHAR LF@help LF@%1 LF@index"              \
        "\nADD LF@index LF@index int@1"                 \
        "\nCONCAT LF@string LF@string LF@help"          \
        "\nJUMP $big_n$"                                \
        "\nLABEL $its_ok$"                              \
        "\nLABEL $its_ok_loop$"                         \
        "\nEQ LF@bool LF@%3 int@0"                      \
        "\nJUMPIFEQ $end_loop$ LF@bool bool@true"       \
        "\nGT LF@bool LF@length LF@index"               \
        "\nJUMPIFEQ $the_end$ LF@bool bool@false"       \
        "\nGETCHAR LF@help LF@%1 LF@index"              \
        "\nCONCAT LF@string LF@string LF@help"          \
        "\nADD LF@index LF@index int@1"                 \
        "\nSUB LF@%3 LF@%3 int@1"                       \
        "\nJUMP $its_ok_loop$"                          \
        "\nLABEL $end_loop$"                            \
        "\nLABEL $the_end$"                             \
        "\nMOVE LF@$return LF@string"                   \
        "\nJUMP $no_fatal_end$"                         \
        "\nLABEL $fatal_end$"                           \
        "\nMOVE LF@$return nil@nil"                     \
        "\nLABEL $no_fatal_end$"                        \
        "\nPOPFRAME"                                    \
        "\nRETURN\n"

// Function ord(s, i)
#define FUNCTION_ORD                                    \
        "\n#Body of function ord(s,i) [%1,%2]"          \
        "\nLABEL $ord"                                  \
        "\nPUSHFRAME"                                   \
        "\nDEFVAR LF@$return"                           \
        "\nDEFVAR LF@bool"                              \
        "\nDEFVAR LF@length"                            \
        "\nMOVE LF@$return nil@nil"                     \
        "\nSTRLEN LF@length LF@%1"                      \
        "\nLT LF@bool LF@%2 int@0"                      \
        "\nJUMPIFEQ $ord_end$ LF@bool bool@true"        \
        "\nLT LF@bool LF@%2 LF@length"                  \
        "\nJUMPIFEQ $ord_end$ LF@bool bool@false"       \
        "\nSTRI2INT LF@$return LF@%1 LF@%2"             \
        "\nLABEL $ord_end$"                             \
        "\nPOPFRAME"                                    \
        "\nRETURN\n"
//  -------------------------------------------------- //

void generate_functions_header() {
    string_add_Cstr(&functions_s, ".IFJcode18\n");
    string_add_Cstr(&functions_s, "DEFVAR GF@%exp_result\n");
    string_add_Cstr(&functions_s, "DEFVAR GF@%operand1\n");
    string_add_Cstr(&functions_s, "DEFVAR GF@%operand3\n");
    string_add_Cstr(&functions_s, "DEFVAR GF@%op1_type\n");
    string_add_Cstr(&functions_s, "DEFVAR GF@%op3_type\n");
    string_add_Cstr(&functions_s, "JUMP $$main$$\n");
}

void generate_main_header() {
    string_add_Cstr(&main_s, "LABEL $$main$$\n");
    string_add_Cstr(&main_s, "CREATEFRAME\n");
    string_add_Cstr(&main_s, "PUSHFRAME\n");
}

void generate_built_in_functions() {
    string_add_Cstr(&functions_s, FUNCTION_LENGTH);
    string_add_Cstr(&functions_s, FUNCTION_CHR);
    string_add_Cstr(&functions_s, FUNCTION_SUBSTR);
    string_add_Cstr(&functions_s, FUNCTION_ORD);
}

void generator_start() {
    string_init(&functions_s);
    string_init(&main_s);
    generate_functions_header();
    generate_built_in_functions();
    generate_main_header();
}

void generator_code_printout() {
    printf("%s\n%s\n", string_get_string(&functions_s), string_get_string(&main_s));
}

void generate_function_start(char* function_name) {
    string_add_Cstr(&functions_s, "LABEL $");
    string_add_Cstr(&functions_s, function_name);
    string_add_Cstr(&functions_s, "\nPUSHFRAME\n");
    string_add_Cstr(&functions_s, "DEFVAR LF@$return\n");
    string_add_Cstr(&functions_s, "MOVE LF@$return nil@nil\n");
    string_add_Cstr(&functions_s, "MOVE GF@%exp_result nil@nil\n");
}

void generate_function_end() {
    string_add_Cstr(&functions_s, "POPFRAME\n");
    string_add_Cstr(&functions_s, "RETURN\n\n");
}

void generate_new_param(char* param, char* index) {
    string_add_Cstr(&functions_s, "DEFVAR LF@");
    string_add_Cstr(&functions_s, param);
    string_add_Cstr(&functions_s, "\nMOVE LF@");
    string_add_Cstr(&functions_s, param);
    string_add_Cstr(&functions_s, " LF@%");
    string_add_Cstr(&functions_s, index);
    string_add_char(&functions_s, '\n');
}

void generate_create_frame() {
    (inFunction)
    ? string_add_Cstr(&functions_s, "CREATEFRAME\n")
    : string_add_Cstr(&main_s, "CREATEFRAME\n");
}

void generate_new_argument_id(char* arg_name, char *index) {
    if(inFunction) {
            string_add_Cstr(&functions_s, "DEFVAR TF@%");
            string_add_Cstr(&functions_s, index);
            string_add_Cstr(&functions_s, "\nMOVE TF@%");
            string_add_Cstr(&functions_s, index);
            string_add_char(&functions_s, ' ');
            string_add_Cstr(&functions_s, "LF@");
            string_add_Cstr(&functions_s, arg_name);
            string_add_char(&functions_s, '\n');
    }
    else {
            string_add_Cstr(&main_s, "DEFVAR TF@%");
            string_add_Cstr(&main_s, index);
            string_add_Cstr(&main_s, "\nMOVE TF@%");
            string_add_Cstr(&main_s, index);
            string_add_char(&main_s, ' ');
            string_add_Cstr(&main_s, "LF@");
            string_add_Cstr(&main_s, arg_name);
            string_add_char(&main_s, '\n');
    }
}

void generate_new_argument_value(char* value, char* type, char *index) {
    if(inFunction) {
        string_add_Cstr(&functions_s, "DEFVAR TF@%");
        string_add_Cstr(&functions_s, index);
        string_add_Cstr(&functions_s, "\nMOVE TF@%");
        string_add_Cstr(&functions_s, index);
        string_add_char(&functions_s, ' ');
        string_add_Cstr(&functions_s, type);
        string_add_Cstr(&functions_s, "@");
        string_add_Cstr(&functions_s, value);
        string_add_char(&functions_s, '\n');
    }
    else {
        string_add_Cstr(&main_s, "DEFVAR TF@%");
        string_add_Cstr(&main_s, index);
        string_add_Cstr(&main_s, "\nMOVE TF@%");
        string_add_Cstr(&main_s, index);
        string_add_char(&main_s, ' ');
        string_add_Cstr(&main_s, type);
        string_add_Cstr(&main_s, "@");
        string_add_Cstr(&main_s, value);
        string_add_char(&main_s, '\n');
    }
}

void generate_function_call(char* function_name) {
    if(inFunction) {
            string_add_Cstr(&functions_s, "CALL $");
            string_add_Cstr(&functions_s, function_name);
            string_add_char(&functions_s, '\n');
    }
    else {
            string_add_Cstr(&main_s, "CALL $");
            string_add_Cstr(&main_s, function_name);
            string_add_char(&main_s, '\n');
    }
}

void generate_MOVE_retval_to_Lvalue(char *l_value_fnc) {
    if(inFunction) {
        string_add_Cstr(&functions_s, "MOVE LF@");
        string_add_Cstr(&functions_s, l_value_fnc);
        string_add_Cstr(&functions_s, " TF@$return\n");
    }
    else {
        string_add_Cstr(&main_s, "MOVE LF@");
        string_add_Cstr(&main_s, l_value_fnc);
        string_add_Cstr(&main_s, " TF@$return\n");
    }
}

void generate_while_head(char *labelID) {
    if(inFunction) {
        string_add_Cstr(&functions_s, "# ************************ WHILE <> DO ************************* #\n");
        string_add_Cstr(&functions_s, "LABEL $loop%start%");
        string_add_Cstr(&functions_s, labelID);
        string_add_char(&functions_s, '\n');
    }
    else {
        string_add_Cstr(&main_s, "# ************************ WHILE <> DO ************************* #\n");
        string_add_Cstr(&main_s, "LABEL $loop%start%");
        string_add_Cstr(&main_s, labelID);
        string_add_char(&main_s, '\n');
    }
}

void generate_conditional_jump(bool in_while, char *labelID) {
    if(inFunction){
        if(in_while) {
            string_add_Cstr(&functions_s, "JUMPIFEQ $loop%end%");
            string_add_Cstr(&functions_s, labelID);
            string_add_Cstr(&functions_s, " GF@%exp_result bool@false\n");
        }
        else {
            string_add_Cstr(&functions_s, "JUMPIFEQ $%else%");
            string_add_Cstr(&functions_s, labelID);
            string_add_Cstr(&functions_s, " GF@%exp_result bool@false\n");
        }
    }
    else {
        if(in_while) {
            string_add_Cstr(&main_s, "JUMPIFEQ $loop%end%");
            string_add_Cstr(&main_s, labelID);
            string_add_Cstr(&main_s, " GF@%exp_result bool@false\n");
        }
        else {
            string_add_Cstr(&main_s, "JUMPIFEQ $%else%");
            string_add_Cstr(&main_s, labelID);
            string_add_Cstr(&main_s, " GF@%exp_result bool@false\n");
        }
    }
}

void generate_while_end(char *labelID) {
    if(inFunction) {
        string_add_Cstr(&functions_s, "JUMP $loop%start%");
        string_add_Cstr(&functions_s, labelID);
        string_add_char(&functions_s, '\n');
        string_add_Cstr(&functions_s, "# ************************* END WHILE ************************** #\n");
        string_add_Cstr(&functions_s, "LABEL $loop%end%");
        string_add_Cstr(&functions_s, labelID);
        string_add_char(&functions_s, '\n');
    }
    else {
        string_add_Cstr(&main_s, "JUMP $loop%start%");
        string_add_Cstr(&main_s, labelID);
        string_add_char(&main_s, '\n');
        string_add_Cstr(&main_s, "# ************************* END WHILE ************************** #\n");
        string_add_Cstr(&main_s, "LABEL $loop%end%");
        string_add_Cstr(&main_s, labelID);
        string_add_char(&main_s, '\n');
    }
}

void generate_if_head() {
    (inFunction)
    ? string_add_Cstr(&functions_s, "# ************************* IF <> THEN ************************* #\n")
    : string_add_Cstr(&main_s, "# ************************* IF <> THEN ************************* #\n");
}

void generate_else(char *labelID) {
    if(inFunction) {
        string_add_Cstr(&functions_s, "JUMP $if%end%");
        string_add_Cstr(&functions_s, labelID);
        string_add_char(&functions_s, '\n');
        string_add_Cstr(&functions_s, "# **************************** ELSE **************************** #\n");
        string_add_Cstr(&functions_s, "LABEL $%else%");
        string_add_Cstr(&functions_s, labelID);
        string_add_char(&functions_s, '\n');
    }
    else {
        string_add_Cstr(&main_s, "JUMP $if%end%");
        string_add_Cstr(&main_s, labelID);
        string_add_char(&main_s, '\n');
        string_add_Cstr(&main_s, "# **************************** ELSE **************************** #\n");
        string_add_Cstr(&main_s, "LABEL $%else%");
        string_add_Cstr(&main_s, labelID);
        string_add_char(&main_s, '\n');
    }
}

void generate_if_end(char *labelID) {
    if(inFunction) {
        string_add_Cstr(&functions_s, "# *************************** END IF *************************** #\n");
        string_add_Cstr(&functions_s, "LABEL $if%end%");
        string_add_Cstr(&functions_s, labelID);
        string_add_char(&functions_s, '\n');
    }
    else {
        string_add_Cstr(&main_s, "# *************************** END IF *************************** #\n");
        string_add_Cstr(&main_s, "LABEL $if%end%");
        string_add_Cstr(&main_s, labelID);
        string_add_char(&main_s, '\n');
    }
}

void generate_NIL_value(char *identifier) {
    if(inFunction) {
        string_add_Cstr(&functions_s, "MOVE LF@");
        string_add_Cstr(&functions_s, identifier);
        string_add_Cstr(&main_s, " nil@nil");
        string_add_char(&functions_s,'\n');
    }
    else {
        string_add_Cstr(&main_s, "MOVE LF@");
        string_add_Cstr(&main_s, identifier);
        string_add_Cstr(&main_s, " nil@nil");
        string_add_char(&main_s,'\n');
    }
}

void generate_PUSHS_id(char* identifier) {

    if(inFunction) {
        string_add_Cstr(&functions_s, "PUSHS LF@");
        string_add_Cstr(&functions_s, identifier);
        string_add_char(&functions_s,'\n');
    }
    else {
        string_add_Cstr(&main_s, "PUSHS LF@");
        string_add_Cstr(&main_s, identifier);
        string_add_char(&main_s,'\n');
    }
}

void generate_PUSHS_value(char* type, char* value) {
    if(inFunction) {
        string_add_Cstr(&functions_s, "PUSHS ");
        string_add_Cstr(&functions_s, type);
        string_add_Cstr(&functions_s, "@");
        string_add_Cstr(&functions_s, value);
        string_add_char(&functions_s,'\n');
    }
    else {
        string_add_Cstr(&main_s, "PUSHS ");
        string_add_Cstr(&main_s, type);
        string_add_Cstr(&main_s, "@");
        string_add_Cstr(&main_s, value);
        string_add_char(&main_s,'\n');
    }
}

void generate_INT2FLOAT(bool operand1) {
    if(inFunction) {
        if(operand1) {
            string_add_Cstr(&functions_s, "POPS GF@%operand1\n");
            string_add_Cstr(&functions_s, "INT2FLOATS\n");
            string_add_Cstr(&functions_s, "PUSHS GF@%operand1\n");
        }
        else {
            string_add_Cstr(&functions_s, "INT2FLOATS\n");
        }
    }
    else {
        if(operand1) {
            string_add_Cstr(&main_s, "POPS GF@%operand1\n");
            string_add_Cstr(&main_s, "INT2FLOATS\n");
            string_add_Cstr(&main_s, "PUSHS GF@%operand1\n");
        }
        else {
            string_add_Cstr(&main_s, "INT2FLOATS\n");
        }
    }
}

void generate_stack_OPERATION(RuleType rule) {
    switch(rule) {
        case E_PLUS_E:
            (inFunction)
            ? string_add_Cstr(&functions_s, "ADDS\n")
            : string_add_Cstr(&main_s, "ADDS\n");
            break;
        case E_CONCAT_E:
            if(inFunction) {
                string_add_Cstr(&functions_s, "POPS GF@%operand3\n");
                string_add_Cstr(&functions_s, "POPS GF@%operand1\n");
                string_add_Cstr(&functions_s, "CONCAT GF@%exp_result GF@%operand1 GF@%operand3\n");
                string_add_Cstr(&functions_s, "PUSHS GF@%exp_result\n");
            }
            else {
                string_add_Cstr(&main_s, "POPS GF@%operand3\n");
                string_add_Cstr(&main_s, "POPS GF@%operand1\n");
                string_add_Cstr(&main_s, "CONCAT GF@%exp_result GF@%operand1 GF@%operand3\n");
                string_add_Cstr(&main_s, "PUSHS GF@%exp_result\n");
            }
            break;
        case E_MINUS_E:
            (inFunction)
            ? string_add_Cstr(&functions_s, "SUBS\n")
            : string_add_Cstr(&main_s, "SUBS\n");
            break;
        case E_MUL_E:
            (inFunction)
            ? string_add_Cstr(&functions_s, "MULS\n")
            : string_add_Cstr(&main_s, "MULS\n");
            break;
        case E_DIV_E:
            (inFunction)
            ? string_add_Cstr(&functions_s, "DIVS\n")
            : string_add_Cstr(&main_s, "DIVS\n");
            break;
        case E_IDIV_E:
            (inFunction)
            ? string_add_Cstr(&functions_s, "IDIVS\n")
            : string_add_Cstr(&main_s, "IDIVS\n");
            break;
        case E_LT_E:
            (inFunction)
            ? string_add_Cstr(&functions_s, "LTS\n")
            : string_add_Cstr(&main_s, "LTS\n");
            break;
        case E_LE_E:
            if(inFunction) {
                string_add_Cstr(&functions_s, "GTS\n");
                string_add_Cstr(&functions_s, "NOTS\n");
            }
            else {
                string_add_Cstr(&main_s, "GTS\n");
                string_add_Cstr(&main_s, "NOTS\n");
            }
            break;
        case E_GT_E:
            (inFunction)
            ? string_add_Cstr(&functions_s, "GTS\n")
            : string_add_Cstr(&main_s, "GTS\n");
            break;
        case E_GE_E:
            if(inFunction) {
                string_add_Cstr(&functions_s, "LTS\n");
                string_add_Cstr(&functions_s, "NOTS\n");
            }
            else {
                string_add_Cstr(&main_s, "LTS\n");
                string_add_Cstr(&main_s, "NOTS\n");
            }
            break;
        case E_EQ_E:
            (inFunction)
            ? string_add_Cstr(&functions_s, "EQS\n")
            : string_add_Cstr(&main_s, "EQS\n");
            break;
        case E_NEQ_E:
            if(inFunction) {
                string_add_Cstr(&functions_s, "EQS\n");
                string_add_Cstr(&functions_s, "NOTS\n");
            }
            else {
                string_add_Cstr(&main_s, "EQS\n");
                string_add_Cstr(&main_s, "NOTS\n");
            }
            break;
    }
}

void generate_POPS_toEXPresult() {
    (inFunction)
    ? string_add_Cstr(&functions_s, "POPS GF@%exp_result\n")
    : string_add_Cstr(&main_s, "POPS GF@%exp_result\n");
}

void generate_MOVE_exp_to_Lvalue(char* lvalue_id) {
    if (inFunction) {
        string_add_Cstr(&functions_s, "MOVE LF@");
        string_add_Cstr(&functions_s, lvalue_id);
        string_add_Cstr(&functions_s, " GF@%exp_result\n");
    } else {
        string_add_Cstr(&main_s, "MOVE LF@");
        string_add_Cstr(&main_s, lvalue_id);
        string_add_Cstr(&main_s, " GF@%exp_result\n");
    }
}

void generate_update_return() {
    (inFunction)
    ? string_add_Cstr(&functions_s, "MOVE LF@$return GF@%exp_result\n")
    : string_add_Cstr(&main_s, "MOVE LF@$return GF@%exp_result\n");
}

void generate_READ(char* value_id, char* value_type) {
    if (inFunction) {
        string_add_Cstr(&functions_s, "READ LF@");
        string_add_Cstr(&functions_s, value_id);
        string_add_char(&functions_s, ' ');
        string_add_Cstr(&functions_s, value_type);
        string_add_char(&functions_s,'\n');
    }
    else {
        string_add_Cstr(&main_s, "READ LF@");
        string_add_Cstr(&main_s, value_id);
        string_add_char(&main_s, ' ');
        string_add_Cstr(&main_s, value_type);
        string_add_char(&main_s,'\n');
    }
}

void generate_WRITE_id(char *identifier) {
    if (inFunction) {
        string_add_Cstr(&functions_s, "WRITE LF@");
        string_add_Cstr(&functions_s, identifier);
        string_add_char(&functions_s,'\n');
    }
    else {
        string_add_Cstr(&main_s, "WRITE LF@");
        string_add_Cstr(&main_s, identifier);
        string_add_char(&main_s,'\n');
    }
}

void generate_WRITE(char *string) {
    if (inFunction) {
        string_add_Cstr(&functions_s, "WRITE string@");
        string_add_Cstr(&functions_s, string);
        string_add_char(&functions_s,'\n');
    }
    else {
        string_add_Cstr(&main_s, "WRITE string@");
        string_add_Cstr(&main_s, string);
        string_add_char(&main_s,'\n');
    }
}

void generate_DEFVAR(char *identifier) {
    if (inFunction) {
        string_add_Cstr(&functions_s, "DEFVAR LF@");
        string_add_Cstr(&functions_s, identifier);
        string_add_char(&functions_s,'\n');
    }
    else {
        string_add_Cstr(&main_s, "DEFVAR LF@");
        string_add_Cstr(&main_s, identifier);
        string_add_char(&main_s,'\n');
    }
}

void generate_DIV_by_0() {
    if (inFunction) {
        string_add_Cstr(&functions_s, "POPS GF@%operand3\n");
        string_add_Cstr(&functions_s, "JUMPIFNEQ $NO_DIV_BY_0 GF@%operand3 int@0\n");
        string_add_Cstr(&functions_s, "EXIT int@9\n");
        string_add_Cstr(&functions_s, "LABEL $NO_DIV_BY_0\n");
        string_add_Cstr(&functions_s, "PUSHS GF@%operand3\n");
    }
    else {
        string_add_Cstr(&main_s, "POPS GF@%operand3\n");
        string_add_Cstr(&main_s, "JUMPIFNEQ $NO_DIV_BY_0 GF@%operand3 int@0\n");
        string_add_Cstr(&main_s, "EXIT int@9\n");
        string_add_Cstr(&main_s, "LABEL $NO_DIV_BY_0\n");
        string_add_Cstr(&main_s, "PUSHS GF@%operand3\n");
    }
}

void generate_DIV_by_0f() {
    if (inFunction) {
        string_add_Cstr(&functions_s, "POPS GF@%operand3\n");
        string_add_Cstr(&functions_s, "JUMPIFNEQ $NO_DIV_BY_0 GF@%operand3 float@0x0p+0\n");
        string_add_Cstr(&functions_s, "EXIT int@9\n");
        string_add_Cstr(&functions_s, "LABEL $NO_DIV_BY_0\n");
        string_add_Cstr(&functions_s, "PUSHS GF@%operand3\n");
    }
    else {
        string_add_Cstr(&main_s, "POPS GF@%operand3\n");
        string_add_Cstr(&main_s, "JUMPIFNEQ $NO_DIV_BY_0 GF@%operand3 float@0x0p+0\n");
        string_add_Cstr(&main_s, "EXIT int@9\n");
        string_add_Cstr(&main_s, "LABEL $NO_DIV_BY_0\n");
        string_add_Cstr(&main_s, "PUSHS GF@%operand3\n");
    }
}


#define STRING_kT 0
#define INT_kT 1
#define FLOAT_kT 2

void generate_TYPE_control(bool operand_1, unsigned known_type, char* function_id, sItemDataType *finalDataType) {
    static unsigned indexLabel = 0;
    char currentLabel[8] = {};
    sprintf(currentLabel, "%d", indexLabel++);

    string_add_Cstr(&functions_s, "POPS GF@%operand3\n");
    string_add_Cstr(&functions_s, "POPS GF@%operand1\n");

    if(known_type == FLOAT_kT) {
        *finalDataType = sI_TYPE_FLOAT;
    }
    else if(known_type == INT_kT){
        *finalDataType = sI_TYPE_UNDEFINED;
    }

    if(known_type != STRING_kT) {
        if(operand_1) {
            string_add_Cstr(&functions_s, "TYPE GF@%op1_type GF@%operand1\n");
            string_add_Cstr(&functions_s, "JUMPIFEQ $end$");
            string_add_Cstr(&functions_s, function_id);
            string_add_Cstr(&functions_s, "$");
            string_add_Cstr(&functions_s, currentLabel);
            string_add_Cstr(&functions_s, " GF@%op1_type string@");
            (known_type == INT_kT)
            ? string_add_Cstr(&functions_s, "int\n")
            : string_add_Cstr(&functions_s, "float\n");
            string_add_Cstr(&functions_s, "JUMPIFEQ $convert$");
            string_add_Cstr(&functions_s, function_id);
            string_add_Cstr(&functions_s, "$");
            string_add_Cstr(&functions_s, currentLabel);
            string_add_Cstr(&functions_s, " GF@%op1_type string@");
            (known_type == INT_kT)
            ? string_add_Cstr(&functions_s, "float\n") // !!!
            : string_add_Cstr(&functions_s, "int\n");
            string_add_Cstr(&functions_s, "EXIT int@4\nLABEL $convert$");               /// LABEL $convert$
            string_add_Cstr(&functions_s, function_id);
            string_add_Cstr(&functions_s, "$");
            string_add_Cstr(&functions_s, currentLabel);

            string_add_Cstr(&functions_s, "\nJUMPIFEQ $convertOP3$");                   /// UMPIFEQ $convertOP3$
            string_add_Cstr(&functions_s, function_id);
            string_add_Cstr(&functions_s, "$");
            string_add_Cstr(&functions_s, currentLabel);
            string_add_Cstr(&functions_s, " GF@%op1_type string@float\n");

            string_add_Cstr(&functions_s, "PUSHS GF@%operand1\nINT2FLOATS\nPOPS GF@%operand1\n");

            string_add_Cstr(&functions_s, "JUMP $endConverts$");                        /// JUMP $endConverts$
            string_add_Cstr(&functions_s, function_id);
            string_add_Cstr(&functions_s, "$");
            string_add_Cstr(&functions_s, currentLabel);
            string_add_char(&functions_s, '\n');
            string_add_Cstr(&functions_s, "LABEL $convertOP3$");                        /// LABEL $convertOP3$
            string_add_Cstr(&functions_s, function_id);
            string_add_Cstr(&functions_s, "$");
            string_add_Cstr(&functions_s, currentLabel);
            string_add_char(&functions_s, '\n');
            string_add_Cstr(&functions_s, "PUSHS GF@%operand3\nINT2FLOATS\nPOPS GF@%operand3\n");
            string_add_Cstr(&functions_s, "LABEL $endConverts$");                       /// LABEL $endConverts$
            string_add_Cstr(&functions_s, function_id);
            string_add_Cstr(&functions_s, "$");
            string_add_Cstr(&functions_s, currentLabel);
            string_add_char(&functions_s, '\n');
        }
        else {
            string_add_Cstr(&functions_s, "TYPE GF@%op3_type GF@%operand3\n");
            string_add_Cstr(&functions_s, "JUMPIFEQ $end$");
            string_add_Cstr(&functions_s, function_id);
            string_add_Cstr(&functions_s, "$");
            string_add_Cstr(&functions_s, currentLabel);
            string_add_Cstr(&functions_s, " GF@%op3_type string@");
            (known_type == INT_kT)
            ? string_add_Cstr(&functions_s, "int\n")
            : string_add_Cstr(&functions_s, "float\n");
            string_add_Cstr(&functions_s, "JUMPIFEQ $convert$");
            string_add_Cstr(&functions_s, function_id);
            string_add_Cstr(&functions_s, "$");
            string_add_Cstr(&functions_s, currentLabel);
            string_add_Cstr(&functions_s, " GF@%op3_type string@");
            (known_type == INT_kT)
            ? string_add_Cstr(&functions_s, "float\n")
            : string_add_Cstr(&functions_s, "int\n");
            string_add_Cstr(&functions_s, "EXIT int@4\nLABEL $convert$");
            string_add_Cstr(&functions_s, function_id);
            string_add_Cstr(&functions_s, "$");
            string_add_Cstr(&functions_s, currentLabel);

            string_add_Cstr(&functions_s, "\nJUMPIFEQ $convertOP1$");                   /// UMPIFEQ $convertOP1$
            string_add_Cstr(&functions_s, function_id);
            string_add_Cstr(&functions_s, "$");
            string_add_Cstr(&functions_s, currentLabel);
            string_add_Cstr(&functions_s, " GF@%op3_type string@float\n");

            string_add_Cstr(&functions_s, "\nPUSHS GF@%operand3\nINT2FLOATS\nPOPS GF@%operand3\n");

            string_add_Cstr(&functions_s, "JUMP $endConverts$");                        /// JUMP $endConverts$
            string_add_Cstr(&functions_s, function_id);
            string_add_Cstr(&functions_s, "$");
            string_add_Cstr(&functions_s, currentLabel);
            string_add_char(&functions_s, '\n');
            string_add_Cstr(&functions_s, "LABEL $convertOP1$");                        /// LABEL $convertOP3$
            string_add_Cstr(&functions_s, function_id);
            string_add_Cstr(&functions_s, "$");
            string_add_Cstr(&functions_s, currentLabel);
            string_add_char(&functions_s, '\n');
            string_add_Cstr(&functions_s, "PUSHS GF@%operand1\nINT2FLOATS\nPOPS GF@%operand1\n");
            string_add_Cstr(&functions_s, "LABEL $endConverts$");                       /// LABEL $endConverts$
            string_add_Cstr(&functions_s, function_id);
            string_add_Cstr(&functions_s, "$");
            string_add_Cstr(&functions_s, currentLabel);
            string_add_char(&functions_s, '\n');
        }
        string_add_Cstr(&functions_s, "LABEL $end$");
        string_add_Cstr(&functions_s, function_id);
        string_add_Cstr(&functions_s, "$");
        string_add_Cstr(&functions_s, currentLabel);
        string_add_char(&functions_s, '\n');
    }
    else {
        if(operand_1) {
            string_add_Cstr(&functions_s, "TYPE GF@%op1_type GF@%operand1\n");
            string_add_Cstr(&functions_s, "JUMPIFEQ $end_string$");
            string_add_Cstr(&functions_s, function_id);
            string_add_Cstr(&functions_s, "$");
            string_add_Cstr(&functions_s, currentLabel);
            string_add_Cstr(&functions_s, " GF@%op1_type string@");
            string_add_Cstr(&functions_s, "string\n");
            string_add_Cstr(&functions_s, "EXIT int@4\nLABEL $end_string$");
            string_add_Cstr(&functions_s, function_id);
            string_add_Cstr(&functions_s, "$");
            string_add_Cstr(&functions_s, currentLabel);
            string_add_char(&functions_s, '\n');
        }
        else {
            string_add_Cstr(&functions_s, "TYPE GF@%op3_type GF@%operand3\n");
            string_add_Cstr(&functions_s, "JUMPIFEQ $end_string$");
            string_add_Cstr(&functions_s, function_id);
            string_add_Cstr(&functions_s, "$");
            string_add_Cstr(&functions_s, currentLabel);
            string_add_Cstr(&functions_s, " GF@%op3_type string@");
            string_add_Cstr(&functions_s, "string\n");
            string_add_Cstr(&functions_s, "EXIT int@4\nLABEL $end_string$");
            string_add_Cstr(&functions_s, function_id);
            string_add_Cstr(&functions_s, "$");
            string_add_Cstr(&functions_s, currentLabel);
            string_add_char(&functions_s, '\n');
        }
        *finalDataType = sI_TYPE_STRING;
    }
    string_add_Cstr(&functions_s, "PUSHS GF@%operand1\nPUSHS GF@%operand3\n");
}

void generate_TYPE_S_control(char* function_id) {
    static unsigned indexLabel = 0;
    char currentLabel[8] = {};
    sprintf(currentLabel, "%d", indexLabel++);

    string_add_Cstr(&functions_s, "POPS GF@%operand3\n");
    string_add_Cstr(&functions_s, "POPS GF@%operand1\n");
    string_add_Cstr(&functions_s, "TYPE GF@%op1_type GF@%operand1\n");
    string_add_Cstr(&functions_s, "TYPE GF@%op3_type GF@%operand3\n");
    string_add_Cstr(&functions_s, "JUMPIFEQ $end_s$");
    string_add_Cstr(&functions_s, function_id);
    string_add_Cstr(&functions_s, "$");
    string_add_Cstr(&functions_s, currentLabel);
    string_add_Cstr(&functions_s, " GF@%op1_type GF@%op3_type\n");
    string_add_Cstr(&functions_s, "JUMPIFEQ $error$");
    string_add_Cstr(&functions_s, function_id);
    string_add_Cstr(&functions_s, "$");
    string_add_Cstr(&functions_s, currentLabel);
    string_add_Cstr(&functions_s, " GF@%op1_type string@string\n");
    string_add_Cstr(&functions_s, "JUMPIFEQ $error$");
    string_add_Cstr(&functions_s, function_id);
    string_add_Cstr(&functions_s, "$");
    string_add_Cstr(&functions_s, currentLabel);
    string_add_Cstr(&functions_s, " GF@%op3_type string@string\n");

    string_add_Cstr(&functions_s, "JUMPIFEQ $convert_op3$");
    string_add_Cstr(&functions_s, function_id);
    string_add_Cstr(&functions_s, "$");
    string_add_Cstr(&functions_s, currentLabel);
    string_add_Cstr(&functions_s, " GF@%op1_type string@float\n");
    string_add_Cstr(&functions_s, "PUSHS GF@%operand1\nINT2FLOATS\nPOPS GF@%operand1\n");
    string_add_Cstr(&functions_s, "JUMP $end_s$");
    string_add_Cstr(&functions_s, function_id);
    string_add_Cstr(&functions_s, "$");
    string_add_Cstr(&functions_s, currentLabel);

    string_add_Cstr(&functions_s, "\nLABEL $convert_op3$");
    string_add_Cstr(&functions_s, function_id);
    string_add_Cstr(&functions_s, "$");
    string_add_Cstr(&functions_s, currentLabel);
    string_add_Cstr(&functions_s, "\nPUSHS GF@%operand3\nINT2FLOATS\nPOPS GF@%operand3\n");
    string_add_Cstr(&functions_s, "JUMP $end_s$");
    string_add_Cstr(&functions_s, function_id);
    string_add_Cstr(&functions_s, "$");
    string_add_Cstr(&functions_s, currentLabel);

    string_add_Cstr(&functions_s, "\nLABEL $error$");
    string_add_Cstr(&functions_s, function_id);
    string_add_Cstr(&functions_s, "$");
    string_add_Cstr(&functions_s, currentLabel);
    string_add_Cstr(&functions_s, "\nEXIT int@4\n");
    string_add_Cstr(&functions_s, "LABEL $end_s$");
    string_add_Cstr(&functions_s, function_id);
    string_add_Cstr(&functions_s, "$");
    string_add_Cstr(&functions_s, currentLabel);
    // concatenate
    string_add_Cstr(&functions_s, "\nPUSHS GF@%operand1\n");
    string_add_Cstr(&functions_s, "PUSHS GF@%operand3\n");
}

void generate_truly_false() {
    if(inFunction) {
        string_add_Cstr(&functions_s, "POPS GF@%exp_result\n");
        string_add_Cstr(&functions_s, "POPS GF@%exp_result\n");
        string_add_Cstr(&functions_s, "PUSHS bool@false\n");
        string_add_Cstr(&functions_s, "PUSHS bool@true\n");
    }
    else {
        string_add_Cstr(&main_s, "POPS GF@%exp_result\n");
        string_add_Cstr(&main_s, "POPS GF@%exp_result\n");
        string_add_Cstr(&main_s, "PUSHS bool@false\n");
        string_add_Cstr(&main_s, "PUSHS bool@true\n");
    }
}
