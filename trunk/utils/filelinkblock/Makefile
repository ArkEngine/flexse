CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libfilelinkblock.a
LIBOBJ= FileLinkBlock.o

READBLOCK= readBlock
READBLOCKOBJ= readBlock.o

#QUEUE2LINE= queue2line
#QUEUE2LINEOBJ= queue2line.o FileOutputer.o

VERINFO= -DPROJNAME="\"$(READBLOCK)\"" \
		 -DVERSION="\"$(READBLOCK) 1.0.0.0\"" \
		 -DCVSTAG="\"$(READBLOCK)_1-0-0-0_PD_BL\"" \

READBLOCKLIBS= -L ./lib/ -lfilelinkblock \
			   -L ../creat_sign/lib/ -lcreat_sign\
			   -L ../mylog/lib/ -lmylog

INCS= -I ../mylog/include/\
	  -I ../creat_sign/include \
	  -I ../MyException/include/

CPPFLAGS= $(VERINFO)-g -pipe -fPIC -W -Wall -Wshadow -Wreturn-type \
		  -Wtrigraphs -Wformat -Wparentheses -Wpointer-arith  -Werror \
		  -Wwrite-strings -Wswitch -Wconversion -Wsign-promo -Wredundant-decls -Wreorder \
		  #			-Weffc++ \

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
	cp FileLinkBlock.h include

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) $(INCS) $< -c -o $@

clean:
	rm *.o -f
	rm lib include -rf

LDFLAGS=  -g $(READBLOCKLIBS) 

$(READBLOCK): $(READBLOCKOBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

#$(QUEUE2LINE): $(QUEUE2LINEOBJ)
#	$(CC) -o $@ $^ $(LDFLAGS)
#	mv $@ output/
