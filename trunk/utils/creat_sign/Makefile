CC= gcc
SHELL= /bin/sh
RANLIB=ranlib

LIB= libcreat_sign.a
LIBOBJ= creat_sign.o

PYEXT= creat_sign.so
PYEXTOBJ= creat_sign.o pycreat_sign.o

MAIN= test
MAINOBJ= test.o

INCS= -I -fpic -I/usr/include/python2.7 
CPPFLAGS= $(INCS) -g -pipe -fPIC -W -Wall -Wshadow -Wreturn-type \
		  -Wtrigraphs -Wformat -Wparentheses -Wpointer-arith \
		  -Wwrite-strings -Wswitch -Wconversion -Wredundant-decls \
		  #			-Weffc++ \-Werror \

all: clean $(LIB) $(PYEXT) $(MAIN)
#all: clean $(LIB)
	rm *.o -f

$(LIB): $(LIBOBJ)
	ar cr $(LIB) $(LIBOBJ)
	$(RANLIB) $(LIB)
	rm -rf lib include
	mkdir lib/
	mkdir include/
	mv $(LIB) lib/
	cp creat_sign.h include/

$(PYEXT): $(PYEXTOBJ)
	gcc -o $@ $^ -shared
	mv creat_sign.so lib/

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) $< -c -o $@

clean:
	rm $(LIB) -f
	rm *.o -f
	rm -rf lib/ include/
	rm test -f

LDFLAGS=  -g -L ./lib/ -lcreat_sign

$(MAIN): $(MAINOBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

