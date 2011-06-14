CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libmemblocks.a
LIBOBJ= memblocks.o

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
	rm -rf output
	mkdir output/lib/ -p
	mkdir output/include/
	cp $(LIB) output/lib/
	cp *.h output/include

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) $(INCS) $< -c -o $@

clean:
	rm $(LIB) -f
	rm *.o -f
	rm -rf output

