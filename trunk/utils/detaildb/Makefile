CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libdetaildb.a
LIBOBJ= detaildb.o

MAIN= test
MAINOBJ= test.o

MAINLIBS= -L ../fileblock/lib/ -lfileblock \
		  -L ../diskv/lib/ -ldiskv\
		  -L ../MyException/lib/ -lMyException \
		  -L ../mylog/lib/ -lmylog\

INCS= -I ../mylog/include/ \
	  -I ../MyException/include/\
	  -I ../fileblock/include/\
	  -I ../diskv/include/\

CPPFLAGS= $(INCS) -g -pipe -fPIC -W -Wall -Wshadow -Wreturn-type \
		  -Wtrigraphs -Wformat -Wparentheses -Wpointer-arith \
		  -Wwrite-strings -Wswitch -Wconversion -Wsign-promo -Wredundant-decls -Wreorder \
		  #			-Weffc++ \-Werror \

all: clean $(LIB) $(MAIN)
	rm *.o -f

$(LIB): $(LIBOBJ)
	ar cr $(LIB) $(LIBOBJ)
	$(RANLIB) $(LIB)
	rm -rf lib include
	mkdir lib/
	mkdir include/
	cp $(LIB) lib/
	cp detaildb.h include

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

