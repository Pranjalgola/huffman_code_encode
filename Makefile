APPDECODE =./decode
APPENCODE =./encode
all:
	g++  -o $(APPENCODE)  ./encode.cpp -std=c++14 -lpthread -O3 -Wall -Wpedantic
	g++  -o $(APPDECODE)  ./decode.cpp -std=c++14 -lpthread -O3 -Wall -Wpedantic
clean:
	rm -f *.o ; rm $(APPENCODE);rm $(APPDECODE);
