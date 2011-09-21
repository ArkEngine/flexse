CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libidmap.a
LIBOBJ= idmap.o

MAIN= test
MAINOBJ= test.o

MAINLIBS= -L ./lib/ -lidmap \
		  -L ../bitmap/lib/ -lbitmap \
		  -L ../myutils/lib/ -lmyutils \
		  -L ../mylog/lib/ -lmylog \
		  -L ../MyException/lib/ -lMyException \

INCS= -I ../mylog/include/\
	  -I ../MyException/include/\
	  -I ../bitmap/include\
	  -I ../myutils/include\
	  -I ./include/ \

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
	cp idmap.h include

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) $< -c -o $@

clean:
	rm $(LIB) -f
	rm *.o -f
	rm test -f
	rm -rf lib/ include/
	rm *.max *list -f

LDFLAGS=  -g $(MAINLIBS) 

$(MAIN): $(MAINOBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

