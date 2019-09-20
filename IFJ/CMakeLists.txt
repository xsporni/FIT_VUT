cmake_minimum_required(VERSION 3.12)
project(IFJ_project C)

set(CMAKE_C_STANDARD 11)

add_executable(IFJ_project
        src/main.c
        src/scanner.c
        src/scanner.h
        src/string_dynamic.c
        src/string_dynamic.h
        src/parser.c
        src/parser.h
        src/symtable.c
        src/symtable.h
        src/errors.h
        src/parser_expressions.c
        src/parser_expressions.h
        src/stack.c
        src/stack.h
        src/generator.c
        src/generator.h)