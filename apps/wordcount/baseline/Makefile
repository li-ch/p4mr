.PHONY: clean

CFLAGS  := -Wall -Werror -g -pg -std=c++11 -O0
LD      := g++
LDFLAGS := ${LDFLAGS} 

wordcount: wordcount.cpp
	        ${LD} -o $@ $^ ${CFLAGS} ${LDFLAGS}

clean:
	        rm -rf *.o *.dSYM/ test all wordcount
