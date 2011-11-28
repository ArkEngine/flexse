CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libdetaildb.a
LIBOBJ= detaildb.o

LDFLAGS= -L ./lib/ -l detaildb \
		  -L ../diskv/lib/ -ldiskv\
		  -L ../fileblock/lib/ -lfileblock \
		  -L ../MyException/lib/ -lMyException \
		  -L ../mylog/lib/ -lmylog\

INCS= -I ../mylog/include/ \
	  -I ../MyException/include/\
	  -I ../fileblock/include/\
	  -I ../diskv/include/\
	  -I ../detaildb/include/\

CPPFLAGS= $(INCS) -g -pipe -fPIC -W -Wall -Wshadow -Wreturn-type \
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

test:
	$(CC) test.cpp -o test $(CPPFLAGS) $(LDFLAGS)

