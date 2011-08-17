CC= g++
SHELL= /bin/sh

OBJ= query_thread.o config.o flexse.o ontime_thread.o update_thread.o merger_thread.o

MAIN= flexse
TEST= client

VERINFO= -DPROJNAME="\"$(MAIN)\"" -DVERSION="\"$(MAIN) 0.0.0.1\""

INCS= -I ../utils/mylog/include/ \
	  -I ../utils/nlp_processor/include/ \
	  -I ../utils/xhead/include/ \
	  -I ../utils/equeue/include/ \
	  -I ../utils/myutils/include/ \
	  -I ../utils/MyException/include/ \
	  -I ../utils/postinglist/include/ \
	  -I ../utils/memblocks/include/ \
	  -I ../utils/fileblock/include/ \
	  -I ../utils/diskv/include/ \
	  -I ../utils/bitmap/include/ \
	  -I ../utils/bitlist/include/ \
	  -I ../utils/idmap/include/ \
	  -I ../utils/secore/include/ \
	  -I ../utils/indexer/index_group/include/ \
	  -I ../utils/indexer/base_indexer/include/ \
	  -I ../utils/indexer/disk_indexer/include/ \
	  -I ../utils/indexer/mem_indexer/include/ \
	  -I ../opensrc/jsoncpp/include/ \
	  -I ../plugin/flexse/include/ \

LDFLAGS= -L ../opensrc/jsoncpp/lib/            -ljson \
		 -L ../utils/indexer/index_group/lib/  -lindex_group \
		 -L ../utils/indexer/mem_indexer/lib/  -lmem_indexer \
		 -L ../utils/indexer/disk_indexer/lib/ -ldisk_indexer \
		 -L ../utils/indexer/base_indexer/lib/ -lbase_indexer \
		 -L ../utils/postinglist/lib/          -lpostinglist \
		 -L ../utils/fileblock/lib/            -lfileblock \
		 -L ../utils/diskv/lib/                -ldiskv \
		 -L ../utils/memblocks/lib/            -lmemblocks \
		 -L ../utils/creat_sign/lib/           -lcreat_sign \
		 -L ../utils/equeue/lib/               -lequeue \
		 -L ../utils/secore/lib/               -lsecore \
		 -L ../plugin/flexse/lib/              -lflexse_plugin \
		 -L ../utils/nlp_processor/lib/        -lnlp_processor \
		 -L ../utils/bitlist/lib/              -lbitlist \
		 -L ../utils/idmap/lib/                -lidmap \
		 -L ../utils/bitmap/lib/               -lbitmap \
		 -L ../utils/xhead/lib/                -lxhead \
		 -L ../utils/myutils/lib/              -lmyutils \
		 -L ../utils/mylog/lib/                -lmylog \
		 -L ../utils/MyException/lib/          -lMyException \

CPPFLAGS= $(DEBUG) $(INCS) $(VERINFO) -g  -pipe  -W -Wall -Wshadow -Wreturn-type \
		  -Wtrigraphs -Wformat -Wparentheses -Wpointer-arith  -Werror

all: client $(MAIN) 
	rm *.o -rf

$(MAIN): $(OBJ) 
	$(CC) -o $@ $^ $(LDFLAGS) -lpthread
	rm -rf bin/ log/
	mkdir log bin
	mv $(MAIN) bin/ -f
	mv $(TEST) bin/ -f

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) $< -c -o $@

clean:
	rm *.o -rf
	rm $(MAIN) -rf
	rm $(TEST) -rf
	rm -rf bin/ log/

client:
	g++ client.cpp -o $(TEST) $(CPPFLAGS) $(LDFLAGS)
