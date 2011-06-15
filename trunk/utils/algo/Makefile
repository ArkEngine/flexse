CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libalgo.a
LIBOBJ= algo.o

MAIN= test
MAINOBJ= test.o

INCS= -I ../include/ \
	  -I ./include/ \
	  -I ../structmask/include/ \
	  -I ../../opensrc/jsoncpp/include/

LDBS= -L ../structmask -lstructmask \
	  -L ../../opensrc/jsoncpp/lib/ -ljson\
	  -L ./lib/ -lalgo

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
	cp algo.h include

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) $< -c -o $@

clean:
	rm $(LIB) -f
	rm *.o -f
	rm -rf lib/ include/
	rm -f test

LDFLAGS=  -g $(LDBS) 

$(MAIN): $(MAINOBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

