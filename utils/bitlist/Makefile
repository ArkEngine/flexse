CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libbitlist.a
LIBOBJ= bitlist.o

MAIN= test
MAINOBJ= test.o

MAINLIBS= -L ./lib/ -lbitlist \
		  -L ../structmask/lib/ -lstructmask \
		  -L ../MyException/lib/ -lMyException \
		  -L ../mylog/lib/ -lmylog

INCS= -I ../mylog/include/ -I ../MyException/include/ -I ../structmask/include -I ./include/
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
	cp bitlist.h include

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) $< -c -o $@

clean:
	rm $(LIB) -f
	rm *.o -f
	rm test -f
	rm -rf lib/ include/

LDFLAGS=  -g $(MAINLIBS) 

$(MAIN): $(MAINOBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

