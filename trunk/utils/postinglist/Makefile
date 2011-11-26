CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libpostinglist.a
LIBOBJ= postinglist.o

MAIN= test
MAINOBJ= test.o

MAINLIBS= -L ./lib/ -lpostinglist \
		  -L ../MyException/lib -lMyException \
		  -L ../structmask/lib/ -lstructmask \
		  -L ../memblocks/lib/ -lmemblocks \
		  -L ../../opensrc/jsoncpp/lib/ -ljson \
		  -L ../mylog/lib/ -lmylog \

INCS= -I ../mylog/include/ \
	  -I ../MyException/include \
	  -I ../memblocks/include/

CPPFLAGS= $(INCS) -g -pipe -fPIC -W -Wall -Wshadow -Wreturn-type \
		  -Wtrigraphs -Wformat -Wparentheses -Wpointer-arith \
		  -Wwrite-strings -Wswitch -Wconversion -Wsign-promo -Wredundant-decls -Wreorder \
		  #			-Weffc++ \-Werror \

all: clean $(LIB) $(MAIN)
#all: clean $(LIB)
	rm *.o -f

$(LIB): $(LIBOBJ)
	ar cr $(LIB) $(LIBOBJ)
	$(RANLIB) $(LIB)
	rm -rf lib include
	mkdir lib/
	mkdir include/
	mv $(LIB) lib/
	cp postinglist.h include

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) $< -c -o $@

clean:
	rm $(LIB) -f
	rm *.o -f
	rm -rf lib/ include/
	rm test -f

LDFLAGS=  -g $(MAINLIBS) -lpthread

$(MAIN): $(MAINOBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

