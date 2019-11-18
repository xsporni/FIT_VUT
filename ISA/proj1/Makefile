CC =g++
CFLAGS =-Wall -pedantic -Wextra
PROJNAME =dns.cpp
RESULT =dns
LOGIN = xsporn01
FILES = Makefile README.md manual.pdf dns.cpp

$(RESULT): $(PROJNAME)
	$(CC) $(CFLAGS) $(PROJNAME) -o $(RESULT)

clean:
	rm -f *~
	rm -f $(RESULT)

tar: clean
	tar -cf $(LOGIN).tar $(FILES)

rmtar:
	rm -f $(LOGIN).tar
