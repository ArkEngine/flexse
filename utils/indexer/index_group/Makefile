CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libindex_group.a
LIBOBJ= index_group.o

MAIN= test
MAINOBJ= test.o

INCS= -I ../mem_indexer/include \
	  -I ../disk_indexer/include \
	  -I ../base_indexer/include \
	  -I ../../diskv/include/\
	  -I ../../algo/include/\
	  -I ../../structmask/include/\
	  -I ../../fileblock/include/\
	  -I ../../memblocks/include/\
	  -I ../../postinglist/include/\
	  -I ../../mylog/include/\
	  -I ../../myutils/include/\
	  -I ../../MyException/include/\
	  -I ../../../opensrc/jsoncpp/include/\

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
	cp index_group.h include

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

