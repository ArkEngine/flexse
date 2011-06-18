CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libstructmask.a
LIBOBJ= structmask.o

MAIN= test
MAINOBJ= test.o

MAINLIBS= -L ./lib/ -lstructmask \
		  -L ../../opensrc/jsoncpp/lib/ -ljson \
		  -L ../MyException/ -lMyException \
		  -L ../mylog/lib/ -lmylog

INCS= -I ../mylog/include/ -I ../../opensrc/jsoncpp/include/ -I ../MyException/include/
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
	cp $(LIB) lib/
	cp structmask.h include

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) $< -c -o $@

clean:
	rm $(LIB) -f
	rm *.o -f
	rm -rf lib/ include/

LDFLAGS=  -g $(MAINLIBS) 

$(MAIN): $(MAINOBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

