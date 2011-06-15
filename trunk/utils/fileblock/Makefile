CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libfileblock.a
LIBOBJ= fileblock.o

INCS= -I ../include/
CPPFLAGS= -g -pipe -fPIC -W -Wall -Wshadow -Wreturn-type \
		  -Wtrigraphs -Wformat -Wparentheses -Wpointer-arith \
		  -Wwrite-strings -Wswitch -Wconversion -Wsign-promo -Wredundant-decls -Wreorder \
		  #			-Weffc++ \-Werror \

all: clean $(LIB)
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
	rm lib/ -rf
	rm include/ -rf
