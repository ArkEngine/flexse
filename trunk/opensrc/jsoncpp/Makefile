WORKROOT = ../..
INCLUDE = -I./jsoncpp-src-0.5.0/include

LIB     = 

COMMON_DEFINES = -DLINUX -D_REENTERANT

ifeq "$(MAKECMDGOALS)" "release"
	DEFINES=$(COMMON_DEFINES) -DNDEBUG
	CPPFLAGS=
	CFLAGS= -O2 -Wall $(DEFINES) $(INCLUDE)  
	CXXFLAGS= -O2 -Wall $(DEFINES) $(INCLUDE) 
else
	ifeq "$(MAKECMDGOALS)" "withpg"
		DEFINES=$(COMMON_DEFINES)
		CPPFLAGS=
		CFLAGS= -g -pg -Wall $(DEFINES) $(INCLUDE)  
		CXXFLAGS= -g -pg -Wall $(DEFINES) $(INCLUDE) 
	else
		DEFINES=$(COMMON_DEFINES)
		CPPFLAGS= 
		CFLAGS= -g -Wall $(DEFINES) $(INCLUDE)  
		CXXFLAGS= -g -Wall $(DEFINES) $(INCLUDE) 
	endif
endif

CC  = g++
CPP = g++
CXX = g++ 
AR  = ar

vpath %.cpp ./jsoncpp-src-0.5.0/src/lib_json

LIBS         = libjson.a
RELEASE_LIBS =
EXECUTABLE   =
TEST_EXEC    =  
INSTALL_PATH = .
ULLIB_OBJS=json_reader.o json_value.o json_writer.o

all		: release
alls	: $(LIBS) $(EXECUTABLE) $(TEST_EXEC)

clean   :
	/bin/rm -f *.o
	/bin/rm -f $(EXECUTABLE) $(TEST_EXEC) $(LIBS)

rebuild : clean alls
	/bin/rm -rf $(INSTALL_PATH)/include $(INSTALL_PATH)/lib
	mkdir -p $(INSTALL_PATH)/lib
	cp -f $(LIBS) $(INSTALL_PATH)/lib
	mkdir -p $(INSTALL_PATH)/include
	cp -r jsoncpp-src-0.5.0/include/json $(INSTALL_PATH)/include

release : rebuild

withpg : rebuild


deps :
	g++ -MM -MG *.cpp >depends
cleanall : clean 
	rm -f *~

$(LIBS) : $(ULLIB_OBJS)
	$(AR) rcv $@ $^

-include depends



