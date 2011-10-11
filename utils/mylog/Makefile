CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libmylog.a
LIBOBJ= mylog.o

INCS= -I ../MyException/include/\
	  -I ../include/

LDFLAGS= -L ./lib -lmylog \
		 -L ../MyException/lib/ -lMyException\

CPPFLAGS= $(INCS) -g -pipe -fPIC -W -Wall -Wshadow -Wreturn-type \
		  -Wtrigraphs -Wformat -Wparentheses -Wpointer-arith \
		  -Wwrite-strings -Wswitch -Wconversion -Wsign-promo -Wredundant-decls -Wreorder \
		  -Werror \
		  #			-Weffc++ \

COVER = false
ifeq ($(COVER), true)
LDFLAGS+= -L ../../unittest/lib/ -lgcover
CPPFLAGS+= -fprofile-arcs -ftest-coverage
endif


all: clean $(LIB) test 
#all: clean $(LIB)
	rm *.o -f

$(LIB): $(LIBOBJ)
	ar cr $(LIB) $(LIBOBJ)
	$(RANLIB) $(LIB)
	rm -rf lib include
	mkdir lib/
	mkdir include/
	cp $(LIB) lib/
	cp mylog.h include

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) $< -c -o $@

clean:
	rm $(LIB) -f
	rm *.o -f
	rm test -f
	rm -rf lib/ include/ log/

test:
	rm -f test
	g++ $(CPPFLAGS) test.cpp -o test $(LDFLAGS)

