CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libcreat_sign.a
LIBOBJ= creat_sign.o

PYEXT= creat_sign.so
PYEXTOBJ= creat_sign.o pycreat_sign.o

MAINOBJ= test.o

INCS= -I /usr/include/python2.7
CPPFLAGS= $(INCS) -g -pipe -fPIC -W -Wall -Wshadow -Wreturn-type \
#	  -Wtrigraphs -Wformat -Wparentheses -Wpointer-arith \
#	  -Wwrite-strings -Wswitch -Wconversion -Wsign-promo -Wredundant-decls -Wreorder \
		  #			-Weffc++ \-Werror \

LDFLAGS = -L ./lib/ -lcreat_sign

# 用来控制是否执行代码覆盖率检查
# 若想执行，在命令行中执行 export COVER=xxoo即可
# 若不想执行，则在命令行中执行 unset COVER即可
ifneq ($(origin COVER), undefined)
LDFLAGS+= -L ../unittest/lib/ -lgcover
CPPFLAGS+= -fprofile-arcs -ftest-coverage
endif


#all: clean $(LIB) $(PYEXT) test
all: clean $(LIB) test
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
	$(CC) -o $@ $^ -shared
	mv creat_sign.so lib/

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) $< -c -o $@

clean:
	rm $(LIB) -f
	rm *.o -f
	rm -rf lib/ include/
	rm test -f

test:
	rm -f test
	g++  -I ./inlcude/ $(CPPFLAGS) test.c -o test $(LDFLAGS)
