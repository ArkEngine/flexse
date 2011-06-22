CC= g++
SHELL= /bin/sh

OBJ= ServiceApp.o Config.o xHead.o equeue.o flexse.o ontime_thread.o update_thread.o merge_thread.o utils.o

MAIN= flexse
TEST= client

VERINFO= -DPROJNAME="\"$(MAIN)\"" -DVERSION="\"$(MAIN) 0.0.0.1\""

INCS= -I ../utils/mylog/include/ \
	  -I ../utils/MyException/include/ \
	  -I ../utils/postinglist/include/ \
	  -I ../utils/memblocks/include/ \
	  -I ../utils/fileblock/include/ \
	  -I ../utils/diskv/include/ \
	  -I ../utils/indexer/index_group/include/ \
	  -I ../utils/indexer/base_indexer/include/ \
	  -I ../utils/indexer/disk_indexer/include/ \
	  -I ../utils/indexer/mem_indexer/include/ \
	  -I ../opensrc/jsoncpp/include/ \

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
		 -L ../utils/MyException/lib/          -lMyException \
		 -L ../utils/mylog/lib/                -lmylog \

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
	g++ client.cpp xHead.cpp -o $(TEST) $(CPPFLAGS) -L ../utils/mylog/ -lmylog -L ../utils/MyException/lib/ -lMyException
