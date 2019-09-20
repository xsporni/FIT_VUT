/**
 * @file generator.h
 * @brief Implementation of GENERATOR module
 *
 * @authors Richard Borbély | xborbe00 | xborbe00@stud.fit.vutbr.cz
 *
 * @date 27 November 2018
 */

#ifndef IFJ_PROJECT_GENERATOR_H
#define IFJ_PROJECT_GENERATOR_H

#include <ctype.h>
#include <stdbool.h>
#include "scanner.h"
#include "string_dynamic.h"
#include "parser_expressions.h"

/**
 * @brief Converts specific characters used for Escape sequences
 * @param inputString -- String from the input
 * @param outputString -- String for the output
 */
void convert_string2codeString(char *inputString, String_DYNAMIC *outputString);

/**
 * @brief Generates the header of the Dynamic String *functions*
 */
void generate_functions_header();

/**
 * @brief Generates the header of the Dynamic String *main*
 */
void generate_main_header();

/**
 * @brief Generates the 4 built in functions to the Dynamic String *functions*
 */
void generate_built_in_functions();

/**
 * @brief Starts the generator by calling the three previous functions
 */
void generator_start();

/**
 * @brief Prints out the content of the two Dynamic strings
 */
void generator_code_printout();

/**
 * @brief Starts a defined function
 * @param function_name -- Name of the function
 */
void generate_function_start(char *function_name);

/**
 * @brief Ends the defined function
 */
void generate_function_end();

/**
 * @brief Creates a new parameter with a value
 * @param param -- Name of the parameter
 * @param index -- Value of the parameter
 */
void generate_new_param(char *param, char *index);

/**
 * @brief Writes a line in the output language, specifically for creating a frame
 */
void generate_create_frame();

/**
 * @brief Generates new argument identifier
 * @param arg_name -- Name of the argument
 * @param index -- Temporary frame index
 */
void generate_new_argument_id(char *arg_name, char *index);

/**
 * @brief Generates new argument value
 * @param value -- Value of the variable
 * @param type -- Type of the variable
 * @param index -- Temporary frame index
 */
void generate_new_argument_value(char* value, char* type, char *index);

/**
 * @brief In the output language, calls a specific function
 * @param function_name Name of the called function
 */
void generate_function_call(char *function_name);

/**
 * @brief Moves the returned value to L value
 * @param l_value_fnc -- Name of the L value
 */
void generate_MOVE_retval_to_Lvalue(char *l_value_fnc);

/**
 * @brief Generates the head of the while loop
 * @param labelID unique ID of the used label
 */
void generate_while_head(char *labelID);

/**
 * @brief Generates the conditional jump in both of IF and WHILE
 * @param in_while -- bool parameter, decides between work with IF or with WHILE
 * @param labelID -- unique ID of the used label
 */
void generate_conditional_jump(bool in_while, char *labelID);

/**
 * @brief Generates the end of the while loop
 * @param labelID -- unique ID of the used label
 */
void generate_while_end(char *labelID);

/**
 * @brief Generates the head of the if statement
 */
void generate_if_head();

/**
 * @brief Generates the "ELSE" part of the if statement
 * @param labelID -- unique ID of the used label
 */
void generate_else(char *labelID);

/**
 * @brief Generates the end of the if statement
 * @param labelID -- unique ID of the used label
 */
void generate_if_end(char *labelID);

/**
 * @brief Moves a NIL value to a variable
 * @param identifier -- Identifier of the variable
 */
void generate_NIL_value(char *identifier);

/**
 * @brief Performs a stack PUSH on a variable
 * @param identifier -- Identifier of the variable
 */
void generate_PUSHS_id(char* identifier);

/**
 * @brief Performs a stack PUSH on a value
 * @param type -- Type of the value
 * @param value -- The value
 */
void generate_PUSHS_value(char* type, char* value);

/**
 * @brief Performs the INT2FLOAT command on one of the two operands
 * @param operand1 -- bool parameter, decides which operand to convert
 */
void generate_INT2FLOAT(bool operand1);

/**
 * @brief Generates stack operations between operands
 * @param rule -- Decides which rule to use
 */
void generate_stack_OPERATION(RuleType rule);

/**
 * @brief Performs a command that copies the value
 * from the stack to a variable on the Global Frame named exp_result
 */
void generate_POPS_toEXPresult();

/**
 * @brief Moves the value from the exp_result variable to the L value
 * @param lvalue_id -- ID of the L value
 */
void generate_MOVE_exp_to_Lvalue(char* lvalue_id);

/**
 * @brief Moves the value from the exp_result variable to the $return variable
 */
void generate_update_return();

/**
 * @brief Performs a READ from the stdin in the output language
 * @param value_id -- ID of the variable
 * @param value_type  -- Type of the variable
 */
void generate_READ(char* value_id, char* value_type);

/**
 * @brief Performs a WRITE to the stdout in the output language,
 * writes the value of a variable
 * @param identifier -- Identifier of the variable
 */
void generate_WRITE_id(char *identifier);

/**
 * @brief Performs a WRITE to the stdout in the output language,
 * writes a pure value
 * @param string -- The value
 */
void generate_WRITE(char *string);

/**
 * @brief Generates a DEFVAR command for variable defining in the output language
 * @param identifier -- Identifier of the variable
 */
void generate_DEFVAR(char *identifier);

/**
 * @brief Treats the problem of dividing by 0 with integers
 */
void generate_DIV_by_0();

/**
 * @brief Treats the problem of dividing by 0 with floats
 */
void generate_DIV_by_0f();

/**
 * @brief Treats the problem with operations between non-compatible types
 * for integers and floats
 * @param operand_1 -- bool parameter, use for recognition which operand is unknown
 * @param known_type -- macro for the known type
 * @param function_id -- ID of the function which uses the operation,
 * used just for the unique name of labels
 * @param finalDataType  -- Final data type after converting
 */
void generate_TYPE_control(bool operand_1, unsigned known_type,
                           char* function_id, sItemDataType *finalDataType);

/**
 * @brief Treats the problem with operations between non-compatible types for strings
 * @param function_id -- ID of the function which uses the operation,
 * used just for the unique name of labels
 */
void generate_TYPE_S_control(char* function_id);

/**
 * @brief Provides stack POP on exp_result and stack PUSH on bool values
 * true and false to receive false after equal instruction
 */
void generate_truly_false();

#endif //IFJ_PROJECT_GENERATOR_H

