CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libstructmask.a
LIBOBJ= structmask.o

MAINOBJ= test.o

LDFLAGS= -L ./lib/ -lstructmask \
	  -L ../../opensrc/jsoncpp/lib/ -ljson \
	  -L ../MyException/lib/ -lMyException \
	  -L ../mylog/lib/ -lmylog

INCS= -I ../mylog/include/ -I ../../opensrc/jsoncpp/include/ -I ../MyException/include/
CPPFLAGS= $(INCS) -O3 -pipe -fPIC -W -Wall -Wshadow -Wreturn-type \
		  -Wtrigraphs -Wformat -Wparentheses -Wpointer-arith \
		  -Wwrite-strings -Wswitch -Wconversion -Wsign-promo -Wredundant-decls -Wreorder \
		  -Wno-sign-conversion \
		  #			-Weffc++ \-Werror \

# 用来控制是否执行代码覆盖率检查
# 若想执行，在命令行中执行 export COVER=xxoo即可
# 若不想执行，则在命令行中执行 unset COVER即可
ifneq ($(origin COVER), undefined)
LDFLAGS+= -L ../unittest/lib/ -lgcover
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
	cp structmask.h include

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) $< -c -o $@

clean:
	rm $(LIB) -f
	rm *.o -f
	rm test -f
	rm -rf lib/ include/

test:
	rm -f test
	g++ $(CPPFLAGS) test.cpp -o test $(LDFLAGS)
