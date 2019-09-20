# Author:       Igor Mjasojedov | xmjaso00 | xmjaso00@stud.fit.vutbr.cz
# Title:        IFJ18 compiler project
# Subject:      IFJ
# Organization: Brno University of Technology, Czech Republic
# Date:         12-02-2018

# ###################################  USAGE:  ###################################
# make          Compiles project's binary files.
# make debug    Compiles project's debug version.
# make pack     Packs source files and headers into the tar.gz archive
# make zip      Packs source files and headers into the .zip archive
# make clean    Cleans object files and binaries
# make test		Runs testing script.

EXECUTABLE=ifj18
AUTHOR=xmjaso00

SOURCES=main.c generator.c parser.c parser_expressions.c scanner.c stack.c string_dynamic.c symtable.c
HEADERS=errors.h generator.h parser.h parser_expressions.h scanner.h stack.h string_dynamic.h symtable.h
OBJECTS=$(SOURCES:.c=.o)

CC=gcc
CFLAGS= -std=c99 # -Wall -Wextra -pedantic -O2 

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^


.PHONY: clean pack zip test

clean:
	rm -f $(EXECUTABLE) $(OBJECTS) $(AUTHOR).tar.gz $(AUTHOR).zip

pack: $(SOURCES) $(HEADERS) Makefile rozdeleni
	tar -czf $(AUTHOR).tar.gz $(SOURCES) $(HEADERS) Makefile rozdeleni

zip: $(SOURCES) $(HEADERS) Makefile rozdeleni
	zip $(AUTHOR).zip $(SOURCES) $(HEADERS) Makefile rozdeleni

test:
	../tests/ifj18_test.sh


