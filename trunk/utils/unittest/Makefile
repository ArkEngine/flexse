all:
	rm -f gcover.o
	g++ -shared -fPIC gcover.cpp -o libgcover.so
	rm -rf lib/
	rm -f gcover.o
	mkdir lib
	mv libgcover.so lib
clean:
	rm -rf lib
