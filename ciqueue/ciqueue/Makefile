CC= g++
SHELL= /bin/sh

OBJ= service_app.o config.o ciqueue.o

MAIN= ciqueue

VERINFO= -DPROJNAME="\"$(MAIN)\"" -DVERSION="\"$(MAIN) 0.0.0.1\""

INCS= -I ../../utils/mylog/include/ \
	  -I ../../utils/filelinkblock/include/ \
	  -I ../../utils/MyException/include/ \
	  -I ../../utils/xhead/include/ \
	  -I ../../utils/myutils/include/ \
	  -I ../../utils/equeue/include/ \
	  -I ../../opensrc/jsoncpp/include/ \
	  -I ../sender/ \

LDFLAGS= -L ../../opensrc/jsoncpp/lib/     -ljson \
		 -L ../../utils/filelinkblock/lib/ -lfilelinkblock \
		 -L ../../utils/creat_sign/lib/    -lcreat_sign \
		 -L ../../utils/xhead/lib/         -lxhead \
		 -L ../../utils/equeue/lib/        -lequeue \
		 -L ../../utils/myutils/lib/       -lmyutils \
		 -L ../../utils/mylog/lib/         -lmylog \
		 -L ../../utils/MyException/lib/   -lMyException \

CPPFLAGS= $(DEBUG) $(INCS) $(VERINFO) -g  -pipe  -W -Wall -Wshadow -Wreturn-type \
		  -Wtrigraphs -Wformat -Wparentheses -Wpointer-arith  -Werror

all: $(MAIN) 
	rm *.o -rf

$(MAIN): $(OBJ) 
	$(CC) -o $@ $^ $(LDFLAGS) -lpthread
	rm -rf bin/ log/
	mkdir log bin
	mv $(MAIN) bin/ -f

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) $< -c -o $@

clean:
	rm *.o -rf
	rm $(MAIN) -rf
	rm -rf bin/ log/

release:
	cp bin/sender  /home/xurui/test_ciqueue/bin/ -f
	cp bin/ciqueue /home/xurui/test_ciqueue/bin/ -f
