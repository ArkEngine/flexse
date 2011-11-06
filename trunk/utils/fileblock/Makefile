CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libfileblock.a
LIBOBJ= fileblock.o

INCS= -I ../mylog/include/\
	  -I ../MyException/include/ \

CPPFLAGS= -g -pipe -fPIC -W -Wall -Wshadow -Wreturn-type \
		  -Wtrigraphs -Wformat -Wparentheses -Wpointer-arith \
		  -Wwrite-strings -Wswitch -Wconversion -Wsign-promo -Wredundant-decls -Wreorder \
		  #			-Weffc++ \-Werror \

all: clean $(LIB) test
	rm *.o -f

$(LIB): $(LIBOBJ)
	ar cr $(LIB) $(LIBOBJ)
	$(RANLIB) $(LIB)
	mkdir lib/
	mkdir include/
	mv $(LIB) lib/
	cp fileblock.h include/

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) $(INCS) $< -c -o $@

clean:
	rm $(LIB) -f
	rm *.o -f
	rm test -f
	rm lib/ -rf
	rm include/ -rf
test:
	g++ -g test.cpp $(INCS) -L ./lib/ -lfileblock -L ../mylog/lib/ -lmylog -L ../MyException/lib/ -lMyException -o test
