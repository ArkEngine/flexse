CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libsecore.a
LIBOBJ= secore.o

INCS= -I ../../utils/mylog/include/ \
	  -I ../../utils/MyException/include/ \
	  -I ../../utils/postinglist/include/ \
	  -I ../../utils/memblocks/include/ \
	  -I ../../utils/fileblock/include/ \
	  -I ../../utils/diskv/include/ \
	  -I ../../utils/detaildb/include/ \
	  -I ../../utils/nlp_processor/include/ \
	  -I ../../utils/idmap/include/ \
	  -I ../../utils/bitmap/include/ \
	  -I ../../utils/bitlist/include/ \
	  -I ../../utils/myutils/include/ \
	  -I ../../utils/structmask/include/ \
	  -I ../../utils/algo/include/ \
	  -I ../../utils/indexer/mem_indexer/include/ \
	  -I ../../utils/indexer/disk_indexer/include/ \
	  -I ../../utils/indexer/base_indexer/include/ \
	  -I ../../utils/indexer/index_group/include/ \
	  -I ../../opensrc/jsoncpp/include/ \
	  -I ../../../code/nlp/wordseg2/trunk/release/ \

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


all: clean $(LIB)
	rm *.o -f

$(LIB): $(LIBOBJ)
	ar cr $(LIB) $(LIBOBJ)
	$(RANLIB) $(LIB)
	rm -rf lib include
	mkdir lib/
	mkdir include/
	mv $(LIB) lib/
	cp secore.h include

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) $< -c -o $@

clean:
	rm $(LIB) -f
	rm *.o -f
	rm -rf lib/ include/
