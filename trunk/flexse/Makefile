CC= g++
SHELL= /bin/sh

OBJ= ServiceApp.o Config.o xHead.o equeue.o flexse.o ontime_thread.o update_thread.o merge_thread.o

MAIN= flexse
TEST= client

VERINFO= -DPROJNAME="\"$(MAIN)\"" -DVERSION="\"$(MAIN) 0.0.0.1\""

INCS= -I ../utils/mylog/include/ \
	  -I ../utils/MyException/include/ \
	  -I ../opensrc/jsoncpp/include/ \

LDFLAGS= -L ../opensrc/jsoncpp/lib/ -ljson \
		 -L ../utils/MyException/lib/ -lMyException \
		 -L ../utils/mylog/lib/ -lmylog \

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
	g++ client.cpp xHead.cpp -o $(TEST) $(CPPFLAGS)
