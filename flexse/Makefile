CC= g++
SHELL= /bin/sh

OBJ= ServiceApp.o Config.o xHead.o equeue.o flexse.o

MAIN= flexse
TEST= client

VERINFO= -DPROJNAME="\"$(MAIN)\"" -DVERSION="\"$(MAIN) 0.0.0.1\""

INCS= -I ../utils/mylog/include/

LDFLAGS= -L ../utils/mylog/lib/ -lmylog

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