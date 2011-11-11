CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libfileblock.a
LIBOBJ= fileblock.o

INCS= -I ../mylog/include/\
	  -I ../MyException/include/ \

CPPFLAGS= -g -pipe -fPIC -W -Wall -Wshadow -Wreturn-type \
		  -Wtrigraphs -Wformat -Wparentheses -Wpointer-arith \
		  -Wwrite-strings -Wswitch -Wconversion -Wsign-promo -Wredundant-decls -Wreorder \
		  #			-Weffc++ \-Werror \

LDFLAGS= -L ./lib/ -lfileblock \
		 -L ../mylog/lib/ -lmylog \
		 -L ../MyException/lib/ -l MyException \

# 用来控制是否执行代码覆盖率检查
# 若想执行，在命令行中执行 export COVER=xxoo即可
# 若不想执行，则在命令行中执行 unset COVER即可
ifneq ($(origin COVER), undefined)
LDFLAGS+= -L ../unittest/lib/ -lgcover
CPPFLAGS+= -fprofile-arcs -ftest-coverage
endif

all: clean $(LIB) test
	rm *.o -f

$(LIB): $(LIBOBJ)
	ar cr $(LIB) $(LIBOBJ)
	$(RANLIB) $(LIB)
	mkdir lib/
	mkdir include/
	mv $(LIB) lib/
	cp fileblock.h include/

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) $(INCS) $< -c -o $@

clean:
	rm $(LIB) -f
	rm *.o -f
	rm test -f
	rm lib/ -rf
	rm include/ -rf
test:
	g++ -g test.cpp -o test $(INCS) $(CPPFLAGS) $(LDFLAGS)
