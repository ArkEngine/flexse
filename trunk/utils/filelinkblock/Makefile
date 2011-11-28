CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libfilelinkblock.a
LIBOBJ= filelinkblock.o

READBLOCK= readBlock

#QUEUE2LINE= queue2line
#QUEUE2LINEOBJ= queue2line.o FileOutputer.o

VERINFO= -DPROJNAME="\"$(READBLOCK)\"" \
		 -DVERSION="\"$(READBLOCK) 1.0.0.0\"" \
		 -DCVSTAG="\"$(READBLOCK)_1-0-0-0_PD_BL\"" \

LDFLAGS= -L ./lib/ -lfilelinkblock \
		 -L ../creat_sign/lib/ -lcreat_sign\
		 -L ../mylog/lib/ -lmylog\
		 -L ../MyException/lib/ -lMyException

INCS= -I ../mylog/include/\
	  -I ../creat_sign/include \
	  -I ../MyException/include/

CPPFLAGS= $(VERINFO)-g -pipe -fPIC -W -Wall -Wshadow -Wreturn-type \
		  -Wtrigraphs -Wformat -Wparentheses -Wpointer-arith  -Werror \
		  -Wwrite-strings -Wswitch -Wconversion -Wsign-promo -Wredundant-decls -Wreorder \
		  #			-Weffc++ \

# 用来控制是否执行代码覆盖率检查
# 若想执行，在命令行中执行 export COVER=xxoo即可
# 若不想执行，则在命令行中执行 unset COVER即可
ifneq ($(origin COVER), undefined)
LDFLAGS+= -L ../unittest/lib/ -lgcover
CPPFLAGS+= -fprofile-arcs -ftest-coverage
endif


#all: clean $(LIB) $(READBLOCK) $(QUEUE2LINE)
all: clean $(LIB) $(READBLOCK)
	rm *.o -f

$(LIB): $(LIBOBJ)
	ar cr $(LIB) $(LIBOBJ)
	$(RANLIB) $(LIB)
	rm -rf lib include
	mkdir lib/
	mkdir include/
	mv $(LIB) lib/
	cp filelinkblock.h include

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) $(INCS) $< -c -o $@

clean:
	rm *.o -f
	rm lib include -rf

$(READBLOCK):
	$(CC) readBlock.cpp -o readBlock $(CPPFLAGS) $(LDFLAGS)

#$(QUEUE2LINE): $(QUEUE2LINEOBJ)
#	$(CC) -o $@ $^ $(LDFLAGS)
#	mv $@ output/
