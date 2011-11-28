CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libalgo.a
LIBOBJ= algo.o

MAINOBJ= test.o

INCS= -I ./include/ \
      -I ../mylog/include/ \
      -I ../structmask/include/ \
      -I ../../opensrc/jsoncpp/include/

LDFLAGS= -L ../structmask -lstructmask \
		 -L ../MyException/lib/ -lMyException\
		 -L ../../opensrc/jsoncpp/lib/ -ljson\
		 -L ./lib/ -lalgo\
		 -L ../mylog/lib/ -lmylog\

CPPFLAGS= $(INCS) -O3 -pipe -fPIC -W -Wall -Wshadow -Wreturn-type \
		  -Wtrigraphs -Wformat -Wparentheses -Wpointer-arith \
		  -Wwrite-strings -Wswitch -Wconversion -Wsign-promo -Wredundant-decls -Wreorder \
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
	mv $(LIB) lib/
	cp algo.h include

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) $< -c -o $@

clean:
	rm $(LIB) -f
	rm *.o -f
	rm -rf lib/ include/
	rm -f test

test:
	rm -f test
	g++ $(CPPFLAGS) test.cpp -o test $(LDFLAGS)

