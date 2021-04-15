#!/usr/bin/env python3 coding: utf-8
#   IPP project: interpret.py
#   Author: Alex Sporni
#   Email: xsporn01@stud.fit.vutbr.cz
#   Date: 14.4.2020

# Imports
import xml.etree.ElementTree as ET
import sys 
import argparse 
import re

# Global variables
dict_of_labels = {}
index_of_actuall_instruction = 0
main_iterator = 0
stati_file = None 
stati_variable_counter = 0
stati_counter = 0

# File errors
ERROR_PARAMETER = 10
ERROR_FILE_OPENING = 11
ERROR_FILE_WRITE = 12

# Interpret errors
ERROR_WRONG_XML_FORMAT = 31
ERROR_WRONG_XML_SYNTAX = 32
ERROR_WRONG_SEMANTIC = 52
ERROR_WRONG_OPERAND_TYPE = 53
ERROR_NON_EXISTING_VARIABLE = 54
ERROR_NON_EXISTING_FRAME = 55
ERROR_MISSING_VALUE = 56
ERROR_WRONG_OPERAND_VALUE = 57
ERROR_STRING = 58 

# Everything is OK
RETURN_OK = 0

# Function displays help menu
def get_help():
    print("\n<-----------------------------------------~ HELP MENU ~-------------------------------------->\n" +
            "##############################################################################################\n" +
            "NÁPOVEDA:                                                                                     \n" +
            "       Program načíta XML reprezentáciu programu a tento program s využitím vstupu podľa para-\n" +
            "       metrov príkazového riadka interpretuje a generuje výstup. Vstupná XML reprezentácia je \n" +
            "       napr. generovaná skriptom parse.php (ale nie nutne) zo zdrojového kódu v IPPcode20.    \n" +
            "       -h, --help              Zobrazí túto nápovedu                                          \n" +
            "       --source=[SRC_FILE]     Vstupný súbor s XML reprezentáciou zdrojového kódu             \n" +
            "       --input=[INPUT_FILE]    Súbor so vstupmi pre samotnú interpretáciu zadaného zdroj. kódu\n" +
            "       Aspoň jeden z parametrov (--source alebo --input) musí byť vždy zadaný, pokiaľ jeden z \n" +
            "       parametrov chýba, tak sú zodpovedajúce dáta načítané zo štandardného vstupu.            \n")
    sys.exit(RETURN_OK)

# Class for working with variables
class Variable():
    def __init__(self, name):
        self.variable_name = name
        self.variable_type = None
        self.variable_value = None

# Class Interpret, the main class in this project
class Interpret():
    # main frame stack where the frames will be stored
    frameStack = []
    # call stack
    callStack = []
    # data stack
    dataStack = []

    # Memory model (frames)
    globalFrame = {}
    localFrame = None
    temporaryFrame = None

    read_counter = 0

    # STATI exstension, used to count variables in Frames and in the data stack
    def stati_vars_counter(self, globalFrame, localFrame, temporaryFrame, dataStack):
        var_counter = 0
        for x in globalFrame:
            if x is not None:
                var_counter += 1
        if localFrame is not None:
            for x in localFrame:
                if x is not None:
                    var_counter += 1
        if temporaryFrame is not None:
            for x in temporaryFrame:
                if x is not None:
                    var_counter += 1
        if dataStack is not None:
            for x in dataStack:
                for i in x:
                    if i is not None:
                        var_counter += 1
        return var_counter

    # Function checks if instructions have correct number of parameters and returns the number of parameters
    def parse_ins_args(self, opcode, number_of_ins_args):
        # Instructions without parameters
        if (opcode == "CREATEFRAME" or opcode == "PUSHFRAME" or opcode == "POPFRAME" or
            opcode == "RETURN" or opcode == "BREAK"):
            if number_of_ins_args > 0:
                print(f"Error, wrong number of arguments, you have provided {number_of_ins_args} arguments !", file=sys.stderr)
                sys.exit(ERROR_WRONG_XML_SYNTAX)
            return number_of_ins_args
            # Instructions with one parameter
        if (opcode == "DEFVAR" or opcode == "CALL" or opcode == "PUSHS" or opcode == "POPS" or
            opcode == "WRITE" or opcode == "LABEL" or opcode == "JUMP" or opcode == "EXIT" or 
            opcode == "DPRINT"):
            if number_of_ins_args != 1:
                print(f"Error, wrong number of arguments, you have provided {number_of_ins_args} arguments !", file=sys.stderr)
                sys.exit(ERROR_WRONG_XML_SYNTAX)
            return number_of_ins_args
            # Instructions with two parameters
        if (opcode == "MOVE" or opcode == "NOT" or opcode == "INT2CHAR" or opcode == "READ" or
            opcode == "STRLEN" or opcode == "TYPE"):
            if number_of_ins_args != 2:
                print(f"Error, wrong number of arguments, you have provided {number_of_ins_args} arguments !", file=sys.stderr)
                sys.exit(ERROR_WRONG_XML_SYNTAX)
            return number_of_ins_args
            # Instructions with three parameters
        if (opcode == "ADD" or opcode == "SUB" or opcode == "MUL" or opcode == "IDIV" or
            opcode == "LT" or opcode == "GT" or opcode == "EQ" or opcode == "AND" or 
            opcode == "OR" or opcode == "STRI2INT" or opcode == "CONCAT" or opcode == "GETCHAR" or
            opcode == "SETCHAR" or opcode == "JUMPIFEQ" or opcode == "JUMPIFNEQ"):
            if number_of_ins_args != 3:
                print(f"Error, wrong number of arguments, you have provided {number_of_ins_args} arguments !", file=sys.stderr)
                sys.exit(ERROR_WRONG_XML_SYNTAX)
            return number_of_ins_args
        print(f"Lexical error, {opcode} is non existing opcode !", file=sys.stderr)
        sys.exit(ERROR_WRONG_XML_SYNTAX)
    
    # Function is used with JUMP operations to determine the correct value and type
    def convert(self, variable_type, variable_value):
        if variable_type == "int":
            return int(variable_value)
        if variable_type == "nil":
            return None
        if variable_type == "bool":
            if variable_value == "true":
                return True
            else:
                return False
        if variable_type == "string":
            return str(variable_value)

    # Function creates a label for every "LABEL" instruction in the XML code
    def create_label_for_xml_appearance(self, opcode, number_of_instructions):
        global stati_counter
        for actual_instruction in range(number_of_instructions):
            if opcode[actual_instruction].get("opcode", actual_instruction) == "LABEL":
                self.create_label(opcode[actual_instruction], actual_instruction)
                stati_counter = stati_counter + 1
    
    # Function creates the label if the function is called and checks if it is already created
    def create_label(self, opcode, actual_instruction):
        label = list(opcode)[0].text
        self.check_label_format(opcode, label)
        if label in dict_of_labels:
            print(f"Error, label: \"{label}\" already exists !", file=sys.stderr)
            sys.exit(ERROR_WRONG_SEMANTIC)
        dict_of_labels[label] = actual_instruction

    # Function checks if the provided label has the correct format 
    def check_label_format(self, opcode, label):
        expected_argument = "label"
        provided_argument = list(opcode)[0].get("type")
        if expected_argument != provided_argument:
            print(f"Error, {provided_argument} is not supported argument type !", file=sys.stderr)
            sys.exit(ERROR_WRONG_XML_SYNTAX)
        if not re.search(r"^[a-zA-Z\-_$&%*!?][a-zA-Z0-9\-_$&%*!?]*$", label):
            print(f"Error, \"{label}\" is not a valid label !",file=sys.stderr)
            sys.exit(ERROR_WRONG_XML_SYNTAX)    
    
    # Function jumps on label, returns instruction number
    def call_label_instruction(self, label, index):
        if label not in dict_of_labels:
            print(f"Error, label: \"{label}\" does not exist !", file=sys.stderr)
            sys.exit(ERROR_WRONG_SEMANTIC)
        self.callStack.append(index)
        return dict_of_labels[label]
    
    # Function jumps on desired label, return instruction number
    def jump_to_label_instruction(self, label_value):
        if label_value not in dict_of_labels:
            print(f"Error, label: \"{label_value}\" does not exist !", file=sys.stderr)
            sys.exit(ERROR_WRONG_SEMANTIC)
        return dict_of_labels[label_value]

    # Function checks the symbol type and value, if it is correct  
    def check_symbol_format(self, opcode, arg1_type, argument_type, argument_value, number_of_ins_args):
        if argument_type == "int" and argument_value is not None:
            if not re.search(r"^(?:[-+]?\d+)$", argument_value):
                print(f"Error, \"{argument_value}\" is not a valid integer value !", file=sys.stderr)
                sys.exit(ERROR_WRONG_XML_SYNTAX)
        
        elif argument_type == "nil" and argument_value is not None:
            if argument_value != "nil":
                print(f"Error, \"{argument_value}\" is not a valid nil value !", file=sys.stderr)
                sys.exit(ERROR_WRONG_XML_SYNTAX)
        
        elif argument_type == "string" and argument_value is not None:
             if re.search(r"(?!\\[0-9]{3})[\s\\#]", argument_value):
                 print(f"Error, \"{argument_value}\" is not a valid string value !", file=sys.stderr)
                 sys.exit(ERROR_WRONG_XML_SYNTAX)
        
        elif argument_type == "bool" and argument_value is not None:
            if argument_value not in ["true", "false"]:
                print(f"Error, \"{argument_value}\" is not a valid bool value !", file=sys.stderr)
                sys.exit(ERROR_WRONG_XML_SYNTAX)
        
        elif argument_type == "var":
            if number_of_ins_args == 1:
                argument_test_value = list(opcode)[0].text
                try:
                    frame_prefix, frame_posix = argument_test_value.split("@")
                except:
                    print("Error, invalid XML syntax !", file=sys.stderr)
                    sys.exit(ERROR_WRONG_XML_SYNTAX)
            
            elif number_of_ins_args == 2:
                argument_test_value = argument_value
                try:
                    frame_prefix, frame_posix = argument_test_value.split("@")
                except:
                    print("Error, invalid XML syntax !", file=sys.stderr)
                    sys.exit(ERROR_WRONG_XML_SYNTAX)
            elif number_of_ins_args == 3:
                argument_test_value = argument_value
                try:
                    frame_prefix, frame_posix = argument_test_value.split("@")
                except:
                    print("Error, invalid XML syntax !", file=sys.stderr)
                    sys.exit(ERROR_WRONG_XML_SYNTAX)

            zavinac = "@"
            frame_prefix = frame_prefix + zavinac
            if not re.match(r"^(?:GF|LF|TF)@", argument_test_value):
                print("Error, invalid XML syntax", file=sys.stderr)
                sys.exit(ERROR_WRONG_XML_SYNTAX)
            self.check_variable_format(opcode, arg1_type, frame_prefix, frame_posix)    

    # Function pops the value from the top of the data stack
    def pops_instruction(self, opcode, frame_prefix, frame_posix):
        if not self.dataStack:
            print("Error, data stack cannot be empty when trying to pop !", file=sys.stderr)
            sys.exit(ERROR_MISSING_VALUE)
        var_type = self.dataStack.pop()
        try:
            if var_type[1] == None:
                var_val = ""
            else:
                var_val = int(var_type[1])
        except ValueError:
            var_val = self.conver_escape_seq(var_type[1])
        self.set_value_of_variable(frame_prefix, frame_posix, var_type[0], var_val)
    
    # Function sets the type and value of variable 
    def set_value_of_variable(self, frame_prefix, frame_posix, variable_type, variable_value):
        variable = self.get_variable_from_frame(frame_prefix, frame_posix)
        variable.variable_type = variable_type
        variable.variable_value = variable_value
    
    # Function converts escape sequences in the string  
    def conver_escape_seq(self, raw_string):
        raw_string = str(raw_string or '')
        converted_string=""
        j = 0
        while j < len(raw_string):
            if raw_string[j]=="#" or raw_string[j].isspace():
                print("Error, unsupported symbols !", file=sys.stderr)
                sys.exit(ERROR_WRONG_XML_SYNTAX)
            if raw_string[j]=="\\":
                try:
                    eval_string = int(raw_string[j+1] + raw_string[j+2] + raw_string[j+3])
                    if eval_string < 0:
                        print("Error, could not covert string !", file=sys.stderr)
                        sys.exit(ERROR_NON_EXISTING_FRAME)
                except Exception:
                    print("Error, could not covert string !", file=sys.stderr)
                    sys.exit(ERROR_NON_EXISTING_FRAME)
                try:
                     raw_string[j+1] + raw_string[j+2] + raw_string[j+3]
                except Exception:
                    print("Error, could not covert string !", file=sys.stderr)
                    sys.exit(ERROR_NON_EXISTING_FRAME)
                converted_string = converted_string + chr(int(raw_string[j+1] + raw_string[j+2] + raw_string[j+3]))
                j = j + 3
            else:
                converted_string = converted_string + raw_string[j] 
            j = j + 1
        return converted_string
        

    # Function provides conditional jump on a label when equal
    def jump_if_eq(self, opcode, label, arg2, arg3, arg2_type, arg2_value, arg3_type, arg3_value):
        global main_iterator
        global stati_counter
        if label not in dict_of_labels:
           print(f"Error, label: \"{label}\" does not exist !", file=sys.stderr)
           sys.exit(ERROR_WRONG_SEMANTIC)

        if arg2_type == "var":
            frame2_prefix = arg2.text[:3]
            frame2_posix = arg2.text[3:]
            variable2 = self.get_variable_from_frame(frame2_prefix, frame2_posix)
            type_of_variable2 = variable2.variable_type
            value_of_variable2 = variable2.variable_value
        else:
            type_of_variable2 = arg2_type
            value_of_variable2 = arg2_value
        
        if arg3_type == "var":
            frame3_prefix = arg3.text[:3]
            frame3_posix = arg3.text[3:]
            variable3 = self.get_variable_from_frame(frame3_prefix, frame3_posix)
            type_of_variable3 = variable3.variable_type
            value_of_variable3 = variable3.variable_value
        else:
            type_of_variable3 = arg3_type
            value_of_variable3 = arg3_value
        
        if type_of_variable2 != type_of_variable3:
            print("Error, symb1 and symb2 must be the same type !", file=sys.stderr)
            sys.exit(ERROR_WRONG_OPERAND_TYPE)
        #stati_counter += 1
        compare_value2 =  self.convert(type_of_variable2, value_of_variable2)
        compare_value3 =  self.convert(type_of_variable3, value_of_variable3)
        if ((compare_value2 == compare_value3) and (type(compare_value2) == type(compare_value3)) or (type_of_variable2 == "nil") or (type_of_variable3 == "nil")):
            main_iterator = int(dict_of_labels[label])

    # Function provides conditional jump on a label when not equal
    def jump_if_not_eq(self, opcode, label, arg2, arg3, arg2_type, arg2_value, arg3_type, arg3_value):
        global main_iterator
        global stati_counter
        if label not in dict_of_labels:
           print(f"Error, label: \"{label}\" does not exist !", file=sys.stderr)
           sys.exit(ERROR_WRONG_SEMANTIC)

        if arg2_type == "var":
            frame2_prefix = arg2.text[:3]
            frame2_posix = arg2.text[3:]
            variable2 = self.get_variable_from_frame(frame2_prefix, frame2_posix)
            type_of_variable2 = variable2.variable_type
            value_of_variable2 = variable2.variable_value
        else:
            type_of_variable2 = arg2_type
            value_of_variable2 = arg2_value
        
        if arg3_type == "var":
            frame3_prefix = arg3.text[:3]
            frame3_posix = arg3.text[3:]
            variable3 = self.get_variable_from_frame(frame3_prefix, frame3_posix)
            type_of_variable3 = variable3.variable_type
            value_of_variable3 = variable3.variable_value
        else:
            type_of_variable3 = arg3_type
            value_of_variable3 = arg3_value
        
        if type_of_variable2 != type_of_variable3:
            print("Error, symb1 and symb2 must be the same type !", file=sys.stderr)
            sys.exit(ERROR_WRONG_OPERAND_TYPE)
        #stati_counter += 1
        compare_value2 =  self.convert(type_of_variable2, value_of_variable2)
        compare_value3 =  self.convert(type_of_variable3, value_of_variable3)
        if ((compare_value2 == compare_value3) and (type(compare_value2) != type(compare_value3)) or (type_of_variable2 == "nil") or (type_of_variable3 == "nil")):
            main_iterator = int(dict_of_labels[label])
    
    # Function changes the char of a string 
    def setchar_instruction(self, opcode, arg2, arg3, arg1_type, frame1_prefix, frame1_posix, arg2_type, arg2_value, arg3_type, arg3_value):
        variable1 = self.get_variable_from_frame(frame1_prefix, frame1_posix)
        if variable1.variable_type != "string":
            print(f"Error, \"{variable1.variable_type}\" must be string type !", file=sys.stderr)
            sys.exit(ERROR_WRONG_OPERAND_TYPE)
        if arg2_type == "var":
            frame2_prefix = arg2.text[:3]
            frame2_posix = arg2.text[3:]
            self.check_variable_format(opcode, arg1_type, frame2_prefix, frame2_posix)
            variable2 = self.get_variable_from_frame(frame2_prefix, frame2_posix)
            if variable2.variable_type != "int":
                print(f"Error, \"{variable2.variable_type}\" must be a integer type", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_TYPE)
            string_index = int(variable2.variable_value)
            if string_index >= len(variable1.variable_value):
                print("Error, index is out of string range !", file=sys.stderr)
                sys.exit(ERROR_STRING)
        else:
            string_index = int(arg2_value)
            if string_index >= len(variable1.variable_value):
                print("Error, index is out of string range !", file=sys.stderr)
                sys.exit(ERROR_STRING)
        if arg3_type == "var":
            frame3_prefix = arg3.text[:3]
            frame3_posix = arg3.text[3:]
            variable3 = self.get_variable_from_frame(frame3_prefix, frame3_posix)
            if variable3.variable_type != "string":
                print(f"Error, \"{variable3.variable_type}\" must be string type !", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_TYPE)
            if len(variable3.variable_value) is None:
                print(f"Error, \"{variable3.variable_value}\" cannot be empty !", file=sys.stderr)
                sys.exit(ERROR_STRING)
            copy_char = variable3.variable_value[0]
        else:
            if arg3_value is None:
                print(f"Error, \"{arg3_value}\" cannot be empty !", file=sys.stderr)
                sys.exit(ERROR_STRING)
            copy_char = arg3_value[0]
        temp_string = list(variable1.variable_value)
        temp_string[string_index] = copy_char
        concat_string = "".join(temp_string)
        self.set_value_of_variable(frame1_prefix, frame1_posix, "string", concat_string)

    # Function returns the char of a string
    def getchar_instruction(self, opcode, frame1_prefix, frame1_posix, arg2_type, arg2_value, arg3_type, arg3_value):
        if arg2_type == "var":
            frame2_prefix = list(opcode)[1].text[:3]
            frame2_posix = list(opcode)[1].text[3:]
            variable2 = self.get_variable_from_frame(frame2_prefix, frame2_posix)
            if variable2.variable_type != "string":
                print(f"Error, \"{variable2.variable_type}\" must be a string type", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_TYPE)
            if variable2.variable_value == None:
                print(f"Error, \"{variable2.variable_value}\" cannot be without value", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_VALUE)
            value_of_variable2 = variable2.variable_value
        else:
            if arg2_value == None:
                print(f"Error, \"{arg2_value}\" cannot be without value !", file=sys.stderr)
                sys.exit(ERROR_STRING)
            value_of_variable2 = arg2_value
        
        if arg3_type == "var":
            frame3_prefix = list(opcode)[2].text[:3]
            frame3_posix = list(opcode)[2].text[3:]
            variable3 = self.get_variable_from_frame(frame3_prefix, frame3_posix)
            if variable3.variable_type != "int":
                print(f"Error, \"{variable3.variable_type}\" must be a integer type", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_TYPE)
            if variable2.variable_value == None:
                print(f"Error, \"{variable3.variable_value}\" cannot be without value", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_VALUE)
            string_index = int(variable3.variable_value)
        else:
            if arg3_value == None:
                print(f"Error, \"{arg2_value}\" cannot be without value !", file=sys.stderr)
                sys.exit(ERROR_STRING)
            string_index = int(arg3_value)
        
        if string_index >= len(value_of_variable2):
            print("Error, index is out of string range !", file=sys.stderr)
            sys.exit(ERROR_STRING)
        position_pointer = value_of_variable2[string_index]
        self.set_value_of_variable(frame1_prefix, frame1_posix, "string", position_pointer)

    # Function concatenates two strings
    def concat_instruction(self, opcode, arg2, arg3, frame1_prefix, frame1_posix, arg2_type, arg2_value, arg3_type, arg3_value):
        if arg2_type == "var":
            frame2_prefix = arg2.text[:3]
            frame2_posix = arg2.text[3:]
            variable2 = self.get_variable_from_frame(frame2_prefix, frame2_posix)
            if variable2.variable_type != "string":
                print(f"Error, \"{variable2.variable_type}\" must be a string type !", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_TYPE)
            if variable2.variable_value == None:
                print(f"Error, \"{variable2.variable_value}\" cannot be without value !", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_VALUE)
            value_of_variable2 = variable2.variable_value
        else:
            value_of_variable2 = arg2_value

        if arg3_type == "var":
            frame3_prefix = arg3.text[:3]
            frame3_posix = arg3.text[3:]
            variable3 = self.get_variable_from_frame(frame3_prefix, frame3_posix)
            if variable3.variable_type != "string":
                print(f"Error, \"{variable3.variable_type}\" must be a string type", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_TYPE)
            if variable3.variable_value == None:
                print(f"Error, \"{variable3.variable_value}\" cannot be without value", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_VALUE)
            value_of_variable3 = variable3.variable_value
        else:
            value_of_variable3 = arg3_value
        concat = value_of_variable2 + value_of_variable3
        self.set_value_of_variable(frame1_prefix, frame1_posix, "string", concat)

    # Function for STRI2INT instruction
    def stri2int_instruction(self, opcode, arg2, arg3, frame1_prefix, frame1_posix, arg2_type, arg2_value, arg3_type, arg3_value):
        if arg2_type == "var":
            frame2_prefix = arg2.text[:3]
            frame2_posix = arg2.text[3:]
            variable2 = self.get_variable_from_frame(frame2_prefix, frame2_posix)
            if variable2.variable_type != "string":
                print(f"Error, type: {variable2.variable_type} must be string type !", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_TYPE)
            temp_string = variable2.variable_value
        else:
            temp_string = arg2_value
        if arg3_type == "var":
            frame3_prefix = arg3.text[:3]
            frame3_posix = arg3.text[3:]
            variable3 = self.get_variable_from_frame(frame3_prefix, frame3_posix)
            if variable3.variable_type != "int":
                print(f"Error, type: {variable3.variable_type} must be string type !", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_TYPE)
            string_index = variable3.variable_value
        else:
            string_index = arg3_value

        if (int(string_index) >= len (temp_string)):
            print("Error, index is out of string range !", file=sys.stderr)
            sys.exit(ERROR_STRING)
        ord_var = temp_string[int(string_index)]
        self.set_value_of_variable(frame1_prefix, frame1_posix, "int", ord(ord_var))

    # Function provides support for basic boolean operators
    def and_or_instruction(self, opcode, arg2, arg3, actual_opcode, frame1_prefix, frame1_posix, arg2_type, arg2_value, arg3_type, arg3_value):
        if arg2_type == "var" or arg2_type == "bool":
            if arg2_type == "var":
                frame2_prefix = arg2.text[:3]
                frame2_posix = arg2.text[3:]
                variable2 = self.get_variable_from_frame(frame2_prefix, frame2_posix)
                if variable2.variable_type != "bool":
                    print(f"Error, type: {variable2.variable_type} must be boolean type !", file=sys.stderr)
                    sys.exit(ERROR_WRONG_OPERAND_TYPE)
                value_of_variable2 = variable2.variable_value
            elif arg2_type == "bool":
                value_of_variable2 = arg2_value
            else:
                print(f"Error, type: {variable2.variable_type} must be boolean type !", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_TYPE)
        if arg3_type == "var" or arg3_type == "bool":
            if arg3_type == "var":
                frame3_prefix = arg3.text[:3]
                frame3_posix = arg3.text[3:]
                variable3 = self.get_variable_from_frame(frame3_prefix, frame3_posix)
                if variable3.variable_type != "bool":
                    print(f"Error, type: {variable3.variable_type} must be boolean type !", file=sys.stderr)
                    sys.exit(ERROR_WRONG_OPERAND_TYPE)
                value_of_variable3 = variable3.variable_value	
            elif arg3_type == "bool":
                value_of_variable3 = arg3_value
            else:
                print(f"Error, type: {variable3.variable_type} must be boolean type !", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_TYPE)
        
        if actual_opcode == "OR":
            if value_of_variable2 == "true" or value_of_variable3 == "true":
                self.set_value_of_variable(frame1_prefix, frame1_posix, "bool", "true")
            else:
                self.set_value_of_variable(frame1_prefix, frame1_posix, "bool", "false")

        if actual_opcode == "AND":
            if value_of_variable2 == "true" and value_of_variable3 == "true":
                self.set_value_of_variable(frame1_prefix, frame1_posix, "bool", "true")
            else:
                self.set_value_of_variable(frame1_prefix, frame1_posix, "bool", "false")
            
    # Function provides support for relational operators
    def lt_gt_eq_instruction(self, opcode, arg2, arg3, actual_opcode, frame1_prefix, frame1_posix, arg2_type, arg2_value, arg3_type, arg3_value):
        if arg2_type == "var":
            frame2_prefix = arg2.text[:3]
            frame2_posix = arg2.text[3:]
            variable2 = self.get_variable_from_frame(frame2_prefix, frame2_posix)
            type_of_variable2 = variable2.variable_type
            value_of_variable2 = variable2.variable_value
        else:
            type_of_variable2 = arg2_type
            value_of_variable2 = arg2_value
        if arg3_type == "var":
            frame3_prefix = arg3.text[:3]
            frame3_posix = arg3.text[3:]
            variable3 = self.get_variable_from_frame(frame3_prefix, frame3_posix)
            type_of_variable3 = variable3.variable_type
            value_of_variable3 = variable3.variable_value
        else:
            type_of_variable3 = arg3_type
            value_of_variable3 = arg3_value

        if actual_opcode == "EQ":
            if type_of_variable2 == "string" and type_of_variable3 == "string":
                compare_result = value_of_variable2 == value_of_variable3
            elif type_of_variable2 == "int" and type_of_variable3 == "int":
                compare_result = int(value_of_variable2) == int(value_of_variable3)
            elif type_of_variable2 == "bool" and type_of_variable3 == "bool":
                if value_of_variable2 == value_of_variable3:
                    compare_result = True
                else:
                    compare_result = False
            elif type_of_variable2 == "nil" or type_of_variable3 == "nil":
                if value_of_variable2 == "nil" and value_of_variable3 == "nil":
                    compare_result = True
                else:
                    compare_result = False
            else:
                print(f"Error, Type \"{type_of_variable2}\" is not equal to \"{type_of_variable3}\" !", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_TYPE)

        elif actual_opcode == "LT":
            if type_of_variable2 == "string" and type_of_variable3 == "string":
                compare_result = value_of_variable2 < value_of_variable3
            elif type_of_variable2 == "int" and type_of_variable3 == "int":
                compare_result = int(value_of_variable2) < int(value_of_variable3)
            elif type_of_variable2 == "bool" and type_of_variable3 == "bool":
                if value_of_variable2 == value_of_variable3:
                    compare_result = False
                elif value_of_variable2 == "true" and value_of_variable3 == "false":
                    compare_result = False
                else:
                    compare_result = True
            else:
                print(f"Error, Type \"{type_of_variable2}\" is not equal to \"{type_of_variable3}\" !", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_TYPE)

        elif actual_opcode == "GT":
            if type_of_variable2 == "string" and type_of_variable3 == "string":
                compare_result = value_of_variable2 > value_of_variable3
            elif type_of_variable2 == "int" and type_of_variable3 == "int":
                compare_result = int(value_of_variable2) > int(value_of_variable3)
            elif type_of_variable2 == "bool" and type_of_variable3 == "bool":
                if value_of_variable2 == value_of_variable3:
                    compare_result = False
                elif value_of_variable2 == "true" and value_of_variable3 == "false":
                    compare_result = True
                else:
                    compare_result = False
            else:
                print(f"Error, Type \"{type_of_variable2}\" is not equal to \"{type_of_variable3}\", or \"nil\" used with GT or LT!", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_TYPE)
        if compare_result == False:
            self.set_value_of_variable(frame1_prefix, frame1_posix, "bool", "false")
        elif compare_result == True:
            self.set_value_of_variable(frame1_prefix, frame1_posix, "bool", "true")

    # Function provides support for basic arithmetic operations 
    def arithmetic_operations(self, opcode, arg2, arg3, actual_opcode, frame1_prefix, frame1_posix, arg2_type, arg2_value, arg3_type, arg3_value):
        if arg2_type == "var":
            frame2_prefix = arg2.text[:3]
            frame2_posix = arg2.text[3:]
            variable2 = self.get_variable_from_frame(frame2_prefix, frame2_posix)
            value_of_variable2 = variable2.variable_value
            if variable2.variable_type != "int":
                print(f"Error, {variable2.variable_type} must be an integer type !", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_TYPE)
        if arg2_type == "int":
            value_of_variable2 = int(arg2_value)    
        if arg3_type == "var":
            frame3_prefix = arg3.text[:3]
            frame3_posix = arg3.text[3:]
            variable3 = self.get_variable_from_frame(frame3_prefix, frame3_posix)
            value_of_variable3 = variable3.variable_value
            if variable3.variable_type != "int":
                print(f"Error, {variable3.variable_type} must be an integer type !", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_TYPE)
        if arg3_type == "int":
            value_of_variable3 = int(arg3_value)
        if actual_opcode == "ADD":
            arithmetic_result = int(value_of_variable2) + int(value_of_variable3)
        elif actual_opcode == "SUB":
            arithmetic_result = int(value_of_variable2) - int(value_of_variable3)
        elif actual_opcode == "MUL":
            arithmetic_result = int(value_of_variable2) * int(value_of_variable3)
        elif actual_opcode == "IDIV":
            if int(value_of_variable3) == 0:
                print("Error, you cannot divide by ZERO !", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_VALUE)
            arithmetic_result = int(value_of_variable2) // int(value_of_variable3)
        self.set_value_of_variable(frame1_prefix, frame1_posix, "int", arithmetic_result)    

    # Function reads value of STDIN
    def read_instruction(self, arg2_type, arg2_value, frame1_prefix, frame1_posix, inputData, inputFile):
        variable1 = self.get_variable_from_frame(frame1_prefix, frame1_posix)
        if (arg2_type != "type" or arg2_value != "int" and arg2_value != "bool" and arg2_value != "string"):
            print("Error, wrong operand type !", file=sys.stderr)
            sys.exit(ERROR_WRONG_XML_SYNTAX)
            if not inputFile:
                read_data = input()
            else:
                read_data = inputData[self.read_counter][:-1]
                self.read_counter = self.read_counter + 1 
            if arg2_value == "int":
                if all(number in "0123456789+-" for number in read_data):
                    variable1.variable_value = read_data
                else:
                    variable1.variable_value = "nil@nil"
                variable1.variable_type = "int"
            elif arg2_value == "bool":
                if read_data.lower() == "true":
                    variable1.variable_value = read_data.lower()
                else:
                    variable1.variable_value = "nil@nil"
                variable1.variable_type = "bool"
            elif arg2_value == "string":
                if re.search(r'[#\s]', read_data):
                    variable1.variable_value = "nil@nil"
                else:
                     variable1.variable_value = read_data
                variable1.var_type = "string"
                
    # Function determines the type of provided symbol 
    def type_instruction(self, arg1_value, arg2_type, arg2_value, frame1_prefix, frame1_posix, frame2_prefix, frame2_posix):
        if arg2_type == "var":
            variable2 = self.get_variable_from_frame(frame2_prefix, frame2_posix)
            if variable2.variable_type != None:
                self.set_value_of_variable(frame1_prefix, frame1_posix, "string", variable2.variable_type)
            else:
                 self.set_value_of_variable(frame1_prefix, frame1_posix, "string", "")
        else:
            self.set_value_of_variable(frame1_prefix, frame1_posix, "string", arg2_type)

    # Function determines the length of a string
    def strlen_instruction(self, arg2_type, arg2_value, frame1_prefix, frame1_posix, frame2_prefix, frame2_posix):
        if arg2_type == "var":
            variable1 = self.get_variable_from_frame(frame2_prefix, frame2_posix)
            if variable1.variable_type != "string":
                print(f"Error, variable type: \"{variable1.variable_type}\" is not a string !", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_TYPE)
            if variable1.variable_value == None:
                print(f"Error, variable is not initialized !", file=sys.stderr)
                sys.exit(ERROR_MISSING_VALUE)
            else:
                len_of_string = len(variable1.variable_value)
                self.set_value_of_variable(frame1_prefix, frame1_posix, "int", len_of_string)
        elif arg2_type == "string":
            if arg2_value == None:
                self.set_value_of_variable(frame1_prefix, frame1_posix, "int", 0)
            else:
                temp_string = self.conver_escape_seq(arg2_value)
                len_of_temp_string = len(temp_string)
                self.set_value_of_variable(frame1_prefix, frame1_posix, "int", len_of_temp_string)
        else:
            print(f"Error, \"{arg2_type}\" type must be string !", file=sys.stderr)
            sys.exit(ERROR_WRONG_OPERAND_TYPE)
    
    # Function converts an integer value to char 
    def int2char_instruction(self, arg2_type, arg2_value, frame1_prefix, frame1_posix, frame2_prefix, frame2_posix):
        if arg2_type == "int":
            try:
                to_char = chr(int(arg2_value))
                self.set_value_of_variable(frame1_prefix, frame1_posix, "string", to_char)
            except ValueError:
                print(f"Error, not a valid unicode value !", file=sys.stderr)
                sys.exit(ERROR_STRING)   
        elif arg2_type == "var":
            variable1 = self.get_variable_from_frame(frame2_prefix, frame2_posix)
            if variable1.variable_value == None:
                print("Error, variable is not initialized !", file=sys.stderr)
                sys.exit(ERROR_MISSING_VALUE)
            elif variable1.variable_type != "int":
                print(f"Error, \"{variable1.variable_type}\" is not \"int\" !", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_TYPE)
            else:
                try:
                    to_char = chr(int(variable1.variable_value))
                    self.set_value_of_variable(frame1_prefix, frame1_posix, "string", to_char)
                except ValueError:
                    print(f"Error, not a valid unicode value !", file=sys.stderr)
                    sys.exit(ERROR_STRING)
        else:
            print(f"Error, \"{arg2_type}\" type must be integer !", file=sys.stderr)
            sys.exit(ERROR_WRONG_OPERAND_TYPE)
    
    # Function provides support for NOT instruction
    def not_instruction(self, arg1_value, arg2_type, arg2_value, frame1_prefix, frame1_posix, frame2_prefix, frame2_posix):
        if arg2_type == "var":
            variable2 = self.get_variable_from_frame(frame2_prefix, frame2_posix)
            if variable2.variable_type != "bool":
                print(f"Error, {variable2.variable_type} is not bool !", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_TYPE)
            if variable2.variable_value == "true":
                self.set_value_of_variable(frame1_prefix, frame1_posix, "bool", "false")
            elif variable2.variable_value =="false":
                self.set_value_of_variable(frame1_prefix, frame1_posix, "bool", "true")
        else:
            if arg2_type != "bool":
                print(f"Error, {arg2_type} must be bool !", file=sys.stderr)
                sys.exit(ERROR_WRONG_OPERAND_TYPE)
            if arg2_value == "true":
                self.set_value_of_variable(frame1_prefix, frame1_posix, "bool", "false")
            elif arg2_value == "false":
                self.set_value_of_variable(frame1_prefix, frame1_posix, "bool", "true")
    
    # Function moves a value to variable
    def move_instruction(self, frame1_prefix, frame1_posix, frame2_prefix, frame2_posix, arg2_type, arg2_value):
        variable = self.get_variable_from_frame(frame1_prefix, frame1_posix)
        variable.variable_type = arg2_type
        if arg2_type == "string" and arg2_value != None:
            variable.variable_value = self.conver_escape_seq(arg2_value)
        elif arg2_value == None:
            variable.variable_value = ""
        elif arg2_type == "var":
            variable_var = self.get_variable_from_frame(frame2_prefix, frame2_posix)
            if variable_var.variable_type != None:
                self.set_value_of_variable(frame1_prefix, frame1_posix, variable_var.variable_type, variable_var.variable_value)
            else:
                print("Error, variable is not initialized !", file=sys.stderr)
                sys.exit(ERROR_MISSING_VALUE)
        else:
            variable.variable_value = arg2_value
    
    # Function provides support for DPRINT and WRITE instructions
    def print_to_stdin(self, opcode, argument_type, argument_value, frame_prefix, frame_posix):
        if argument_type == "var":
            variable = self.get_variable_from_frame(frame_prefix, frame_posix)
            if variable.variable_value == None:
                print("Error, variable is not initialized !", file=sys.stderr)
                sys.exit(ERROR_MISSING_VALUE)
            if opcode == "DPRINT":
                print(variable.variable_value, end='', file=sys.stderr)
            else:
                print(variable.variable_value, end='')
        elif argument_type == "string":
            string = self.conver_escape_seq(argument_value)
            if opcode == "DPRINT":
                print(string, end='', file=sys.stderr)
            else:
                print(string, end='')
        elif argument_type == "nil":
            if opcode == "DPRINT":
                print("", end= '', file=sys.stderr)
            else:
                print("", end='')
        else:
            if opcode == "DPRINT":
                print(argument_value, end='', file=sys.stderr)
            else:
                print(argument_value, end='')
                
    # Function exits the interpretation with a exit code         
    def exit_instruction(self, argument_type, argument_value, frame_prefix, frame_posix):
        global stati_file
        global stati_counter
        global stati_variable_counter
        if argument_type == "int" or argument_type == "var":
            if argument_type == "int":
                if int(argument_value) < 0 or int(argument_value) > 49:
                    print(f"Error, {argument_value} is not a valid integer number, value must be in range <0,49> !", file=sys.stderr)
                    sys.exit(ERROR_WRONG_OPERAND_VALUE)
                else:
                    sys.exit(int(argument_value))
            elif argument_type == "var":
                variable = self.get_variable_from_frame(frame_prefix, frame_posix)
                if variable.variable_type == None:
                    print("Error, no integer value while using EXIT instruction !", file=sys.exit)
                    sys.exit(ERROR_MISSING_VALUE)
                if variable.variable_type != "int":
                    print("Error, no integer value while using EXIT instruction !", file=sys.exit)
                    sys.exit(ERROR_WRONG_OPERAND_TYPE)
                if int(variable.variable_value) < 0 or int(variable.variable_value) > 49:
                    print(f"Error, {argument_value} is not a valid integer number, value must be in range <0,49> !", file=sys.stderr)
                    sys.exit(ERROR_WRONG_OPERAND_VALUE)     
                for argument in sys.argv:
                    if argument == "--insts": stati_file.write(str(stati_counter) + "\n")
                    if argument == "--vars": stati_file.write(str(stati_variable_counter) + "\n")
                if stati_file is not None:
                    stati_file.close()
                sys.exit(int(variable.variable_value))  
        else:
            print("Error, EXIT instruction must be a valid integer number !", file=sys.stderr)
            sys.exit(ERROR_WRONG_OPERAND_VALUE)

    # Function checks the variable if it is the correct variable format
    def check_variable_format(self, opcode, arg1_type, frame_prefix, frame_posix):
        self.check_frame(frame_prefix)
        expected_argument = "var"
        provided_argument = arg1_type
        if expected_argument != provided_argument:
            print(f"Error, {provided_argument} is not supported argument type !", file=sys.stderr)
            sys.exit(ERROR_WRONG_XML_SYNTAX)
        if not re.search(r"[a-zA-Z\-_$&%*!?][a-zA-Z0-9\-_$&%*!?]*$", frame_posix):
            print(f"Error, \"{frame_posix}\" is not a valid variable !",file=sys.stderr)
            sys.exit(ERROR_WRONG_XML_SYNTAX)

    # Function checks the variable format and defines the variable
    def check_and_define_variable(self, opcode, argument_type, frame_prefix, frame_posix):
        self.check_variable_format(opcode, argument_type, frame_prefix, frame_posix)
        self.add_variable_to_frame(frame_prefix, frame_posix)
    
    # Function adds variable to the correct frame
    def add_variable_to_frame(self, frame_prefix, frame_posix):
        new_variable_instance = Variable(frame_posix)
        if frame_prefix == "GF@":
            if new_variable_instance.variable_name in self.globalFrame:
                print(f"Error, \"{new_variable_instance.variable_name}\"is already defined in the Global Frame !", file=sys.stderr)
                sys.exit(ERROR_WRONG_SEMANTIC)
            self.globalFrame[new_variable_instance.variable_name] = new_variable_instance # {"THIS_IS_GLOBAL_FRAME":"THIS_IS_GLOBAL_FRAME"}
        elif frame_prefix == "LF@":
            if self.localFrame == None:
                print("Error, Local Frame is not initialized !", file=sys.stderr)
                sys.exit(ERROR_NON_EXISTING_FRAME)
            if new_variable_instance.variable_name in self.localFrame:
                print(f"Error, variable \"{new_variable_instance.variable_name}\" is already defined in the Local Frame !", file=sys.stderr)
                sys.exit(ERROR_WRONG_SEMANTIC)
            self.localFrame[new_variable_instance.variable_name] = new_variable_instance
        elif frame_prefix == "TF@":
            if self.temporaryFrame == None:
                print("Error, Temporary Frame is not initialized !", file=sys.stderr)
                sys.exit(ERROR_NON_EXISTING_FRAME)
            if new_variable_instance.variable_name in self.temporaryFrame:
                print(f"Error, variable \"{new_variable_instance.variable_name}\" is already defined in the Temporary Frame !", file=sys.stderr)
                sys.exit(ERROR_WRONG_SEMANTIC)
            self.temporaryFrame[new_variable_instance.variable_name] = new_variable_instance # {"varName":Variable()}
    
    # Functions checks if the variable is in the frame and returns the value 
    def get_variable_from_frame(self, frame_prefix, frame_posix):
        if frame_prefix == "GF@":
            if frame_posix not in self.globalFrame:
                print(f"Error, variable {frame_posix} does not exist in the Global Frame !", file=sys.stderr)
                sys.exit(ERROR_NON_EXISTING_VARIABLE)
            else:
                return self.globalFrame[frame_posix]
        elif frame_prefix == "LF@":
            if self.localFrame == None:
                print(f"Error, Local Frame does not exist !")
                sys.exit(ERROR_NON_EXISTING_FRAME)
            if frame_posix not in self.localFrame:
                print(f"Error, variable {frame_posix} does not exist in the Local Frame !", file=sys.stderr)
                sys.exit(ERROR_NON_EXISTING_VARIABLE)
            else:
                return self.localFrame[frame_posix]
        elif frame_prefix == "TF@":
            if self.temporaryFrame == None:
                print(f"Error, Temporary Frame does not exist !")
                sys.exit(ERROR_NON_EXISTING_FRAME)
            if frame_posix not in self.temporaryFrame:
                print(f"Error, variable {frame_posix} does not exist in the Local Frame !", file=sys.stderr)
                sys.exit(ERROR_NON_EXISTING_VARIABLE)
            else:
                return self.temporaryFrame[frame_posix]
    
    # Function check frame if the prefix of the frame (first three chars) are valid
    def check_frame(self, frame_prefix):
        if (frame_prefix == 'LF@' or frame_prefix == 'TF@' or frame_prefix == 'GF@'):
            return True
        else:
            print(f"Error, {frame_prefix} not supported frame type !", file=sys.stderr)
            sys.exit(ERROR_WRONG_XML_SYNTAX)

    # Function provides support for instruction PUSHFRAME
    def push_frame(self):
        if self.temporaryFrame == None:
            print("Error, temporary frame is undefined !", file=sys.stderr)
            sys.exit(ERROR_NON_EXISTING_FRAME)        
        else:
            self.frameStack.append(self.temporaryFrame.copy()) # append on top of stack
            self.localFrame = self.temporaryFrame # accesible via local frame 
            self.temporaryFrame = None  # after beeing pushed it will become undefined again
            
    
    # Function provides support for instruction POPFRAME
    def pop_frame(self):
        if self.localFrame != None:
            self.temporaryFrame = self.frameStack.pop() # pops the first item in stack
            try:
                self.localFrame = self.frameStack[-1] # if there is no frame is in LF --> error  
            except IndexError:
                self.localFrame = None
        else:
            print("Error, local frame is undefined !", file=sys.stderr)
            sys.exit(ERROR_NON_EXISTING_FRAME)
    
    # Function provides basic debugging outputs
    def break_debug(self):
        print("<--------~~# DEBUG_MENU #~~-------->", file=sys.stderr)
        print("**********START**********",file=sys.stderr)
        print("Global Frame content: ", file=sys.stderr)
        for x in self.globalFrame.values(): # "GF@THIS_IS_GLOBAL_FRAME" : Variable(), "sfsdf": Variable()
            print(f"VARIABLE NAME: {x.variable_name} VARIABLE TYPE: {x.variable_type} VARIABLE VALUE: {x.variable_value}", file=sys.stderr)
        
        print("\nLocal Frame content: ", file=sys.stderr)
        if self.localFrame is None or self.localFrame == {}:
            print("Local frame is not initialized !",file=sys.stderr)
        else:
            for x in self.localFrame.values():
                print(f" VARIABLE NAME: {x.variable_name} VARIABLE TYPE: {x.variable_type} VARIABLE VALUE: {x.variable_value}", file=sys.stderr)       
        print("\nTemporary Frame content: ", file=sys.stderr)
        if self.temporaryFrame == None:
            print("Temporary frame is not initialized !",file=sys.stderr)
        else:
            for x in self.temporaryFrame.values():
                print(f" VARIABLE NAME: {x.variable_name} VARIABLE TYPE: {x.variable_type} VARIABLE VALUE: {x.variable_value}", file=sys.stderr)
        print(f"\nNumber of instructions completed: {index_of_actuall_instruction} ",file=sys.stderr)
        print("***********END***********",file=sys.stderr)

    # The main function of the whole interpret
    def main_parser(self, opcode, actual_opcode, index, inputData, inputFile):
        global index_of_actuall_instruction
        global main_iterator
        index_of_actuall_instruction = index_of_actuall_instruction + 1
        number_of_ins_args = self.parse_ins_args(actual_opcode, len(list(opcode)))

        # Now we are going to check overy opcode, sorted by number of parameters 
        # First: instructions without parameters
        if number_of_ins_args == 0:
            if actual_opcode == "CREATEFRAME":
                self.temporaryFrame = {}
            elif actual_opcode == "PUSHFRAME":
                self.push_frame()
            elif actual_opcode == "POPFRAME":
                self.pop_frame()
            elif actual_opcode == "RETURN":
                if len(self.callStack) == 0:
                    print("Error, call stack is empty !", file=sys.stderr)
                    sys.exit(ERROR_MISSING_VALUE)
                main_iterator = self.callStack.pop()+1  
            elif actual_opcode == "BREAK":
                self.break_debug()
        # Second: instructions with one parameter
        elif number_of_ins_args == 1:
            if list(opcode)[0].tag != "arg1":
                print("Error, missing argument!", file=sys.stderr)
                sys.exit(ERROR_WRONG_XML_SYNTAX)
            try:
                frame_prefix = list(opcode)[0].text[:3]
                frame_posix = list(opcode)[0].text[3:]
                argument_type = list(opcode)[0].get('type')
                argument_value = list(opcode)[0].text
            except:
                print("Error, not supported XML syntax", file=sys.stderr)
                sys.exit(ERROR_WRONG_XML_SYNTAX)
            # DEFVAR <var>
            if actual_opcode == "DEFVAR":
                self.check_and_define_variable(opcode, argument_type, frame_prefix, frame_posix)
            # PUSHS <symb>
            elif actual_opcode == "PUSHS":
                self.check_symbol_format(opcode, argument_type, argument_type, argument_value, number_of_ins_args)
                if argument_type not in ["var", "string", "nil", "int", "bool"]:
                    print(f"Error, \"{argument_type}\" is not a valid type !", file=sys.stderr)
                    sys.exit(ERROR_WRONG_XML_SYNTAX)
                if argument_type == "var":
                    data_to_push = self.get_variable_from_frame(frame_prefix, frame_posix)
                    if data_to_push.variable_value == None:
                        print("Error, missing value for variable !", file=sys.stderr)
                        sys.exit(ERROR_MISSING_VALUE)
                    if data_to_push.variable_value == "nil":
                       print("Error, you cannot push nil !", file=sys.stderr)
                       sys.exit(ERROR_MISSING_VALUE)
                    self.dataStack.append(data_to_push.variable_type, data_to_push.variable_value)
                elif argument_type == "nil":
                    print("Error, nil is not a valid value and cannot be pushed !", file= sys.stderr)
                    sys.exit(ERROR_MISSING_VALUE)
                else:
                    self.dataStack.append([argument_type, argument_value]) # [['string', '123']] [['string', 'GF@123']]
            # POPS <var>
            elif actual_opcode == "POPS":
                self.check_variable_format(opcode, argument_type, frame_prefix, frame_posix)
                self.pops_instruction(opcode, frame_prefix, frame_posix)                
            # WRITE | DPRINT <symb>
            elif actual_opcode == "WRITE" or actual_opcode == "DPRINT":
                self.check_symbol_format(opcode, argument_type, argument_type, argument_value, number_of_ins_args)
                if argument_type not in ["var", "string", "nil", "int", "bool"]:
                    print(f"Error, \"{argument_type}\" is not a valid type !", file=sys.stderr)
                    sys.exit(ERROR_WRONG_XML_SYNTAX)
                self.print_to_stdin(opcode, argument_type, argument_value, frame_prefix, frame_posix)
            # LABEL | JUMP | CALL --> <label> 
            elif actual_opcode == "LABEL" or actual_opcode == "JUMP" or actual_opcode == "CALL":
                label = list(opcode)[0].text #<arg1 type="label">navestie</arg1> --> "navestie"
                self.check_label_format(opcode, label)
                if actual_opcode == "CALL":
                    main_iterator = self.call_label_instruction(label, index)
                elif actual_opcode == "JUMP":
                    main_iterator = self.jump_to_label_instruction(argument_value)
                    
            # EXIT <symb>
            elif actual_opcode == "EXIT":
                self.check_symbol_format(opcode, argument_type, argument_type, argument_value, number_of_ins_args)
                self.exit_instruction(argument_type, argument_value, frame_prefix, frame_posix)
        # Third: instructions with two parameters
        elif number_of_ins_args == 2:
            if (list(opcode)[0].tag == "arg1" and list(opcode)[1].tag == "arg2"):
                arg1 = list(opcode)[0]
                arg2 = list(opcode)[1]
            # if the arguments are switched...
            elif (list(opcode)[0].tag == "arg2" and list(opcode)[1].tag == "arg1"):
                arg1 = list(opcode)[1]
                arg2 = list(opcode)[0]
            else:
                print("Error, invalid XML format !", file=sys.stderr)
                sys.exit(ERROR_WRONG_XML_SYNTAX)
            arg1_type = arg1.get("type")
            arg1_value = arg1.text
            arg2_type = arg2.get("type")
            arg2_value = arg2.text
            try:
                frame1_prefix = arg1.text[:3]
                frame1_posix = arg1.text[3:]
            except:
                print("Error, invalid XML format, first argument must be <var> type !", file=sys.stderr)
                sys.exit(ERROR_WRONG_XML_SYNTAX)

            try:
                frame2_prefix = arg2.text[:3]
                frame2_posix = arg2.text[3:]
            except:
                frame2_prefix = ""
                frame2_posix = ""
            # MOVE <var> <symb>
            if actual_opcode == "MOVE":
                self.check_variable_format(opcode, arg1_type, frame1_prefix, frame1_posix)  
                if arg1_type not in ["var"] or arg2_type not in ["var", "int", "string", "bool", "nil"]:
                    print(f"Error, \"{arg1_type}\" must be: <var> and \"{arg2_type}\" must be: <var>, <int>, <string> or <bool> !", file=sys.stderr)
                    sys.exit(ERROR_WRONG_OPERAND_TYPE)
                if arg2_value is not None:
                    self.check_symbol_format(opcode, arg1_type, arg2_type, arg2_value, number_of_ins_args)
                self.move_instruction(frame1_prefix, frame1_posix, frame2_prefix, frame2_posix, arg2_type, arg2_value)
            # NOT <var> <symb>
            elif actual_opcode == "NOT":
                self.check_variable_format(opcode, arg1_type, frame1_prefix, frame1_posix)
                if arg1_type not in ["var"] or arg2_type not in ["bool"]:
                    print(f"Error, \"{arg1_type}\" must be: <var> and \"{arg2_type}\" must be: <bool> !", file=sys.stderr)
                    sys.exit(ERROR_WRONG_OPERAND_TYPE)
                if arg2_value is not None:
                    self.check_symbol_format(opcode, arg1_type, arg2_type, arg2_value, number_of_ins_args)
                self.not_instruction(arg1_value, arg2_type, arg2_value, frame1_prefix, frame1_posix, frame2_prefix, frame2_posix)

            # INT2CHAR | STRLEN | TYPE --> <var> <symb>
            elif actual_opcode == "INT2CHAR" or actual_opcode == "STRLEN" or actual_opcode == "TYPE":
                self.check_variable_format(opcode, arg1_type, frame1_prefix, frame1_posix)
                if arg2_value is not None:
                    self.check_symbol_format(opcode, arg1_type, arg2_type, arg2_value, number_of_ins_args)
                if actual_opcode == "INT2CHAR":
                    if arg1_type not in ["var"] or arg2_type not in ["var", "int"]:
                        print(f"Error, \"{arg1_type}\" must be: <var> and \"{arg2_type}\" must be: <var> or <int> !", file=sys.stderr)
                        sys.exit(ERROR_WRONG_OPERAND_TYPE)
                    self.int2char_instruction(arg2_type, arg2_value, frame1_prefix, frame1_posix, frame2_prefix, frame2_posix)
                elif actual_opcode == "STRLEN":
                    if arg1_type not in ["var"] or arg2_type not in ["var", "string"]:
                        print(f"Error, \"{arg1_type}\" must be: <var> and \"{arg2_type}\" must be: <var> or <string> !", file=sys.stderr)
                        sys.exit(ERROR_WRONG_OPERAND_TYPE)
                    self.strlen_instruction(arg2_type, arg2_value, frame1_prefix, frame1_posix, frame2_prefix, frame2_posix)
                elif actual_opcode == "TYPE":
                    if arg1_type not in ["var"] or arg2_type not in ["var", "int", "string", "bool", "nil"]:
                       print(f"Error, \"{arg1_type}\" must be: <var> and \"{arg2_type}\" must be: <var>, <int>, <string>, <bool> or <nil> !", file=sys.stderr)
                       sys.exit(ERROR_WRONG_OPERAND_TYPE)
                    self.type_instruction(arg1_value, arg2_type, arg2_value, frame1_prefix, frame1_posix, frame2_prefix, frame2_posix)
            # READ <var> <type>
            elif actual_opcode == "READ":
                self.check_variable_format(opcode, arg1_type, frame1_prefix, frame1_posix)
                self.read_instruction(arg2_type, arg2_value, frame1_prefix, frame1_posix, inputData, inputFile)
        # Fourth: instructions with three parameters
        elif number_of_ins_args == 3:
            arg1 = ""
            arg2 = ""
            arg3 = ""
            if (list(opcode)[0].tag == "arg1" and list(opcode)[1].tag == "arg2" and list(opcode)[2].tag == "arg3"): 
                arg1 = list(opcode)[0]
                arg2 = list(opcode)[1]
                arg3 = list(opcode)[2]
            elif (list(opcode)[0].tag == "arg1" and list(opcode)[1].tag == "arg3" and list(opcode)[2].tag == "arg2"):
                arg1 = list(opcode)[0]
                arg2 = list(opcode)[2]
                arg3 = list(opcode)[1]
            elif (list(opcode)[0].tag == "arg2" and list(opcode)[1].tag == "arg1" and list(opcode)[2].tag == "arg3"):
                arg1 = list(opcode)[1]
                arg2 = list(opcode)[0]
                arg3 = list(opcode)[2]
            elif (list(opcode)[0].tag == "arg2" and list(opcode)[1].tag == "arg3" and list(opcode)[2].tag == "arg1"):
                arg1 = list(opcode)[2]
                arg2 = list(opcode)[0]
                arg3 = list(opcode)[1]
            elif (list(opcode)[0].tag == "arg3" and list(opcode)[1].tag == "arg2" and list(opcode)[2].tag == "arg1"):
                arg1 = list(opcode)[2]
                arg2 = list(opcode)[1]
                arg3 = list(opcode)[0]
            elif (list(opcode)[0].tag == "arg3" and list(opcode)[1].tag == "arg1" and list(opcode)[2].tag == "arg2"):
                arg1 = list(opcode)[1]
                arg2 = list(opcode)[2]
                arg3 = list(opcode)[0]
            elif (arg1 == "" or arg2 == "" or arg3 == ""):
                print("Error, invalid XML format !", file=sys.stderr)
                sys.exit(ERROR_WRONG_XML_SYNTAX)
            else:
                print("Error, invalid XML format !", file=sys.stderr)
                sys.exit(ERROR_WRONG_XML_SYNTAX)
            arg1_type = arg1.get("type")
            arg1_value = arg1.text
            arg2_type = arg2.get("type")
            arg2_value = arg2.text
            arg3_type = arg3.get("type")
            arg3_value = arg3.text

            # ADD | SUB | MUL | IDIV --> <var> <symb1> <symb2> 
            if actual_opcode == "ADD" or actual_opcode == "SUB" or actual_opcode == "MUL" or actual_opcode == "IDIV":
                try:
                    frame1_prefix = arg1.text[:3]
                    frame1_posix = arg1.text[3:]
                except:
                    print("Error, not supported XML syntax !", file=sys.stderr)
                    sys.exit(ERROR_WRONG_XML_SYNTAX)
                self.check_variable_format(opcode, arg1_type, frame1_prefix, frame1_posix)
                self.check_symbol_format(opcode, arg1_type, arg2_type, arg2_value, number_of_ins_args)
                self.check_symbol_format(opcode, arg1_type, arg3_type, arg3_value, number_of_ins_args)
                if arg2_type not in ["int", "var"] or arg3_type not in ["int", "var"]:
                    print(f"Error, \"{arg2_type}\" and \"{arg3_type}\" must be \"var\" or \"int\"", file=sys.stderr)
                    sys.exit(ERROR_WRONG_OPERAND_TYPE)
                self.arithmetic_operations(opcode, arg2, arg3, actual_opcode, frame1_prefix, frame1_posix, arg2_type, arg2_value, arg3_type, arg3_value)
            # LT | GT | EQ --> <var> <symb1> <symb2>
            elif actual_opcode == "LT" or actual_opcode == "GT" or actual_opcode == "EQ":
                try:
                    frame1_prefix = arg1.text[:3]
                    frame1_posix = arg1.text[3:]
                except:
                    print("Error, not supported XML syntax !", file=sys.stderr)
                    sys.exit(ERROR_WRONG_XML_SYNTAX)
                self.check_variable_format(opcode, arg1_type, frame1_prefix, frame1_posix)
                self.check_symbol_format(opcode, arg1_type, arg2_type, arg2_value, number_of_ins_args)
                self.check_symbol_format(opcode, arg1_type, arg3_type, arg3_value, number_of_ins_args)
                if arg2_type not in ["int", "var", "string", "bool", "nil"] or arg3_type not in ["int", "var", "string", "bool", "nil"]:
                    print(f"Error, \"{arg2_type}\" and \"{arg3_type}\" must be \"var\" | \"int\" | \"string\" | \"bool\" or \"nil\" ", file=sys.stderr)
                    sys.exit(ERROR_WRONG_OPERAND_TYPE)
                self.lt_gt_eq_instruction(opcode, arg2, arg3, actual_opcode, frame1_prefix, frame1_posix, arg2_type, arg2_value, arg3_type, arg3_value)
            # AND | OR --> <var> <symb> 
            elif actual_opcode == "AND" or actual_opcode == "OR":
                try:
                    frame1_prefix = arg1.text[:3]
                    frame1_posix = arg1.text[3:]
                except:
                    print("Error, not supported XML syntax !", file=sys.stderr)
                    sys.exit(ERROR_WRONG_XML_SYNTAX)
                self.check_variable_format(opcode, arg1_type, frame1_prefix, frame1_posix)
                self.check_symbol_format(opcode, arg1_type, arg2_type, arg2_value, number_of_ins_args)
                self.check_symbol_format(opcode, arg1_type, arg3_type, arg3_value, number_of_ins_args)
                if arg2_type not in ["var","bool"] or arg3_type not in ["var","bool"]:
                    print(f"Error, \"{arg2_type}\" and \"{arg3_type}\" must be \"bool\" ", file=sys.stderr)
                    sys.exit(ERROR_WRONG_OPERAND_TYPE)
                self.and_or_instruction(opcode, arg2, arg3, actual_opcode, frame1_prefix, frame1_posix, arg2_type, arg2_value, arg3_type, arg3_value)
            # STRI2INT <var> <symb1> <symb2>
            elif actual_opcode == "STRI2INT":
                try:
                    frame1_prefix = arg1.text[:3]
                    frame1_posix = arg1.text[3:]
                except:
                    print("Error, not supported XML syntax !", file=sys.stderr)
                    sys.exit(ERROR_WRONG_XML_SYNTAX)
                self.check_variable_format(opcode, arg1_type, frame1_prefix, frame1_posix)
                self.check_symbol_format(opcode, arg1_type, arg2_type, arg2_value, number_of_ins_args)
                self.check_symbol_format(opcode, arg1_type, arg3_type, arg3_value, number_of_ins_args)
                if arg2_type not in ["var", "string"] or arg3_type not in ["var", "int"]:
                    print(f"Error, \"{arg2_type}\" and \"{arg3_type}\" must be \"var\" | \"string\" or \"int\" ! ", file=sys.stderr)
                    sys.exit(ERROR_WRONG_OPERAND_TYPE)
                self.stri2int_instruction(opcode, arg2, arg3, frame1_prefix, frame1_posix, arg2_type, arg2_value, arg3_type, arg3_value)
            # CONCAT <var> <symb1> <symb2>
            elif actual_opcode == "CONCAT":
                try:
                    frame1_prefix = arg1.text[:3]
                    frame1_posix = arg1.text[3:]
                except:
                    print("Error, not supported XML syntax !", file=sys.stderr)
                    sys.exit(ERROR_WRONG_XML_SYNTAX)
                self.check_variable_format(opcode, arg1_type, frame1_prefix, frame1_posix)
                self.check_symbol_format(opcode, arg1_type, arg2_type, arg2_value, number_of_ins_args)
                self.check_symbol_format(opcode, arg1_type, arg3_type, arg3_value, number_of_ins_args)
                if arg2_type not in ["var", "string"] or arg3_type not in ["var", "string"]:
                    print(f"Error, \"{arg2_type}\" and \"{arg3_type}\" must be \"var\" or \"string\" ! ", file=sys.stderr)
                    sys.exit(ERROR_WRONG_OPERAND_TYPE)
                self.concat_instruction(opcode, arg2, arg3, frame1_prefix, frame1_posix, arg2_type, arg2_value, arg3_type, arg3_value)
            # GETCHAR <var> <symb1> <symb2>
            elif actual_opcode == "GETCHAR":
                try:
                    frame1_prefix = arg1.text[:3]
                    frame1_posix = arg1.text[3:]
                except:
                    print("Error, not supported XML syntax !", file=sys.stderr)
                    sys.exit(ERROR_WRONG_XML_SYNTAX)
                self.check_variable_format(opcode, arg1_type, frame1_prefix, frame1_posix)
                self.check_symbol_format(opcode, arg1_type, arg2_type, arg2_value, number_of_ins_args)
                self.check_symbol_format(opcode, arg1_type, arg3_type, arg3_value, number_of_ins_args)
                if arg2_type not in ["var", "string"] or arg3_type not in ["var", "int"]:
                    print(f"Error, \"{arg2_type}\" and \"{arg3_type}\" must be \"var\" | \"string\" or \"int\" ! ", file=sys.stderr)
                    sys.exit(ERROR_WRONG_OPERAND_TYPE)
                self.getchar_instruction(opcode, frame1_prefix, frame1_posix, arg2_type, arg2_value, arg3_type, arg3_value)          
            # SETCHAR <var> <symb1> <symb2>
            elif actual_opcode == "SETCHAR":
                try:
                    frame1_prefix = arg1.text[:3]
                    frame1_posix = arg1.text[3:]
                except:
                    print("Error, not supported XML syntax !", file=sys.stderr)
                    sys.exit(ERROR_WRONG_XML_SYNTAX)
                self.check_variable_format(opcode, arg1_type, frame1_prefix, frame1_posix)
                self.check_symbol_format(opcode, arg1_type, arg2_type, arg2_value, number_of_ins_args)
                self.check_symbol_format(opcode, arg1_type, arg3_type, arg3_value, number_of_ins_args)
                if arg2_type not in ["var", "int"] or arg3_type not in ["var", "string"]:
                    print(f"Error, \"{arg2_type}\" and \"{arg3_type}\" must be \"var\" | \"string\" or \"int\" ! ", file=sys.stderr)
                    sys.exit(ERROR_WRONG_OPERAND_TYPE)
                self.setchar_instruction(opcode, arg2, arg3, arg1_type, frame1_prefix, frame1_posix, arg2_type, arg2_value, arg3_type, arg3_value)    
            # JUMPIFEQ <label> <symb1> <symb2>
            elif actual_opcode == "JUMPIFEQ":
                if arg1_type not in ["label"] or arg2_type not in ["var", "int", "string", "bool"] or arg3_type not in ["var", "int", "string", "bool"]:
                    print("Error, wrong operand type ! ", file=sys.stderr)
                    sys.exit(ERROR_WRONG_OPERAND_TYPE)
                label = arg1.text
                self.check_label_format(opcode, label)
                self.jump_if_eq(opcode, label, arg2, arg3, arg2_type, arg2_value, arg3_type, arg3_value)         
            # JUMPIFNEQ <label> <symb1> <symb2>
            elif actual_opcode == "JUMPIFNEQ":
                if arg1_type not in ["label"] or arg2_type not in ["var", "int", "string", "bool"] or arg3_type not in ["var", "int", "string", "bool"]:
                    print("Error, wrong operand type ! ", file=sys.stderr)
                    sys.exit(ERROR_WRONG_OPERAND_TYPE)
                label = arg1.text
                self.check_label_format(opcode, label)
                self.jump_if_not_eq(opcode, label, arg2, arg3, arg2_type, arg2_value, arg3_type, arg3_value)

def main():
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("-h", "--help", action="store_true", dest="help")
    parser.add_argument("--source", dest="source_file", action="store")
    parser.add_argument("--input", dest="input_file", action="store")
    parser.add_argument("--stats", nargs=1)
    parser.add_argument("--insts", action="store_const", const="stats_const")
    parser.add_argument("--vars", action="store_const", const="stats_const")

    try:
        parse_args = parser.parse_args()
    except SystemExit:
        sys.exit(ERROR_PARAMETER)
    # Checking if parameter --help is combined with another parameters
    if parse_args.help == True:
        if len(sys.argv) != 2:
            print("Error, --help parameter cannot be combined with different parameteres!", file=sys.stderr)
            sys.exit(ERROR_PARAMETER)
        get_help()
    
    # Input parsing
    sourceFile = parse_args.source_file
    inputFile = parse_args.input_file
    global stati_file
    global stati_variable_counter

    stati_file = None
    try:
        if parse_args.stats is not None:
            stati_file = open(parse_args.stats[0], "w")
    except:
        print("Error, check your permission to write !", file=sys.stderr)
        sys.exit(ERROR_FILE_WRITE)
    if ((parse_args.insts == "stats_const" or parse_args.vars == "stats_const") and stati_file is None):
        print("Error, wrong stats syntax", file=sys.stderr)
        sys.exit(ERROR_PARAMETER)
    if (sourceFile is None) and (inputFile is None):
        print("Error, --source or --input is mandatory !", file=sys.stderr)
        sys.exit(ERROR_PARAMETER)
    if sourceFile is None:
        sourceFile = sys.stdin
    if inputFile:
        try:
            filePointer = open(inputFile)
            inputData = filePointer.readlines()
        except Exception:
            print("An error occured while opening a file !", file=sys.stderr)
            sys.exit(ERROR_FILE_OPENING)
        finally:
            filePointer.close()
    else:
        inputData = ''
    try:
        xml_tree = ET.parse(sourceFile)
    except Exception:
        print("Error, invalid XML format !", file=sys.stderr)
        sys.exit(ERROR_WRONG_XML_FORMAT)
    
    # XML tree parsing and ordering, always sorts XML instruction in ascending order 
    root = xml_tree.getroot()
    try:
        root[:] = sorted(root, key = lambda child: (child.tag, int(child.get("order"))))
    except:
        print("Error, invalid XML syntax, probably missing order number !", file=sys.stderr)
        sys.exit(ERROR_WRONG_XML_SYNTAX)

    # Header check
    if (root.tag != "program"):
        print("Error, invalid XML header !", file=sys.stderr)
        sys.exit(ERROR_WRONG_XML_FORMAT)
    if (root.attrib["language"] != "IPPcode20") or (len(root.attrib) > 3):
        print("Error, invalid XML structure", file=sys.stderr)
        sys.exit(ERROR_WRONG_XML_SYNTAX)
    if ("language" not in root.attrib):
        print("Error, invalid XML structure", file=sys.stderr)
        sys.exit(ERROR_WRONG_XML_FORMAT)
    
    length_of_root_attributes = len(root.attrib)

    # Optional attributes handling
    if (length_of_root_attributes == 2 and ('name' not in root.attrib and 'description' not in root.attrib) or
        length_of_root_attributes == 3 and ('name' not in root.attrib or 'description' not in root.attrib)):
        print("Error, invalid XML format, too many attributes !", file=sys.stderr)
        sys.exit(ERROR_WRONG_XML_FORMAT)
    
    # Optional attributes deleting
    if (length_of_root_attributes == 2 or length_of_root_attributes == 3):
        if "description" in root.attrib: del root.attrib["description"]
        if "name" in root.attrib: del root.attrib["name"]
    modified_length_of_root_attributes = len(root.attrib)
    if modified_length_of_root_attributes != 1:
        print("Error, invalid XML structure !")
        exit(ERROR_WRONG_XML_SYNTAX)

    # Instruction order check
    if len(root.findall("*")) != len(root.findall('instruction')):
        print("Error, wrong XML format !", file=sys.stderr)
        sys.exit(ERROR_WRONG_XML_SYNTAX)
    for i in root:
        if (len(i.attrib) != 2):
            print("Error, wrong number of attributes !", file=sys.stderr)
            sys.exit(ERROR_WRONG_XML_SYNTAX)

        argument_test = list(i.attrib)
        if ("order" != argument_test[0]):
            print(f"Error, missing attribute {argument_test[0]} !", file=sys.stderr)
            sys.exit(ERROR_WRONG_XML_SYNTAX)
        if ("opcode" != argument_test[1]):
            print(f"Error, missing attribute {argument_test[1]} !", file=sys.stderr)
            sys.exit(ERROR_WRONG_XML_SYNTAX)
    
    # new interpret instance
    interpret = Interpret()
    # getting all instructions
    instruction = root.findall('instruction')
    number_of_instructions = len(instruction)
    interpret.create_label_for_xml_appearance(instruction, number_of_instructions)
    global index_of_actuall_instruction # will help me to determine the order of actuall instruction
    global main_iterator # for jump operations
    global stati_counter
    while main_iterator < number_of_instructions:
        if int(instruction[main_iterator].get('order')) < 1:
            print("Error, invalid XML syntax !", file=sys.stderr)
            sys.exit(ERROR_WRONG_XML_SYNTAX)
        if int(instruction[main_iterator].get('order')) == main_iterator:
            print("Error, order number cannot be duplicit !", file=sys.stderr)
            sys.exit(ERROR_WRONG_XML_SYNTAX)
        actual_instruction_set = instruction[main_iterator].get('opcode', main_iterator)
        actual_instruction_set = actual_instruction_set.upper()
        if actual_instruction_set is None:
            print("Error, invalid XML format !", file=sys.stderr)
            sys.exit(ERROR_WRONG_XML_FORMAT)
        interpret.main_parser(instruction[main_iterator], actual_instruction_set, main_iterator, inputData, inputFile)
        if stati_file is not None:
            if actual_instruction_set != "LABEL":
                stati_counter = stati_counter + 1
            stati_variable_counter = max(stati_variable_counter, interpret.stati_vars_counter(interpret.globalFrame, interpret.localFrame, interpret.temporaryFrame, interpret.dataStack)) 
        main_iterator += 1
    for argument in sys.argv:
        if argument == "--insts": stati_file.write(str(stati_counter) + "\n")
        if argument == "--vars": stati_file.write(str(stati_variable_counter) + "\n")
    if stati_file is not None: stati_file.close()
       
if __name__ == "__main__": 
    main()