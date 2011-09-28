CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libconnectmanager.a
LIBOBJ= connectmap.o connectmanager.o

MAIN= test
MAINOBJ= test.o

MAINLIBS= -L ./lib/ -lconnectmanager \
		  -L ../../opensrc/jsoncpp/lib/ -ljson \
		  -L ../creat_sign/lib/ -lcreat_sign \
		  -L ../myutils/lib/ -lmyutils \
		  -L ../MyException/lib/ -lMyException \
		  -L ../mylog/lib/ -lmylog

INCS= -I ../mylog/include/ \
	  -I ../../opensrc/jsoncpp/include/ \
	  -I ../creat_sign/include/ \
	  -I ../myutils/include/ \
	  -I ../MyException/include/ \

CPPFLAGS= $(INCS) -g -pipe -fPIC -W -Wall -Wshadow -Wreturn-type \
		  -Wtrigraphs -Wformat -Wparentheses -Wpointer-arith \
		  -Wwrite-strings -Wswitch -Wconversion -Wsign-promo -Wredundant-decls -Wreorder \
		  -Werror \
		  #-Weffc++ \

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
	cp *.h include

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

