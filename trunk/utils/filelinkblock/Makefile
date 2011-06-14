CC= g++
SHELL= /bin/sh
RANLIB=ranlib

LIB= libfilelinkblock.a
LIBOBJ= FileLinkBlock.o creat_sign_BKDR_AP.o

READBLOCK= readBlock
READBLOCKOBJ= readBlock.o

#QUEUE2LINE= queue2line
#QUEUE2LINEOBJ= queue2line.o FileOutputer.o

VERINFO= -DPROJNAME="\"$(READBLOCK)\"" \
		 -DVERSION="\"$(READBLOCK) 1.0.0.0\"" \
		 -DCVSTAG="\"$(READBLOCK)_1-0-0-0_PD_BL\"" \

READBLOCKLIBS= -L ./output/lib/ -lfilelinkblock \

INCS= -I ../include/
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
	rm -rf output
	mkdir output/lib/ -p
	mkdir output/include/
	cp $(LIB) output/lib/
	cp FileLinkBlock.h output/include

%.o: %.cpp %.h
	$(CC) $(CPPFLAGS) $(INCS) $< -c -o $@

clean:
	rm $(LIB) -f
	rm *.o -f
	rm output -rf

LDFLAGS=  -g $(READBLOCKLIBS) 

$(READBLOCK): $(READBLOCKOBJ)
	$(CC) -o $@ $^ $(LDFLAGS)
	mv $@ output/

#$(QUEUE2LINE): $(QUEUE2LINEOBJ)
#	$(CC) -o $@ $^ $(LDFLAGS)
#	mv $@ output/
