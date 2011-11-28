CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libmemblocks.a
LIBOBJ= memblocks.o

INCS= -I ../mylog/include \
	  -I ../MyException/include \

CPPFLAGS= -g -pipe -fPIC -W -Wall -Wshadow -Wreturn-type \
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


all: clean $(LIB)
	rm *.o -f

$(LIB): $(LIBOBJ)
	ar cr $(LIB) $(LIBOBJ)
	$(RANLIB) $(LIB)
	rm -rf lib/ include/
	mkdir lib/
	mkdir include/
	mv $(LIB) lib/
	cp memblocks.h include

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) $(INCS) $< -c -o $@

clean:
	rm *.o -f
	rm -rf lib include

