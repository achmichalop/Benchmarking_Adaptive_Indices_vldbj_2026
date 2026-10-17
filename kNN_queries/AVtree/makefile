OS := $(shell uname)
ifeq ($(OS),Darwin)
		CC      = g++-13
        CFLAGS  = -O3 -std=c++14 -w -D $(distFunc) -D DIM$(dims) -D datacount$(datacount) 
        LDFLAGS = 
else
        CC      = g++
        CFLAGS  = -O3 -mavx -std=c++14 -w -march=native -D $(distFunc) -D DIM$(dims) -D datacount$(datacount) 
        LDFLAGS =
endif

ifneq ($(datacount),)
        CFLAGS += -D DATACOUNT=$(datacount)
endif


ifeq ($(distFunc), ED)
	SOURCES = tree/AVtreeString.cpp
else
	SOURCES = tree/AVtree.cpp
endif
OBJECTS = $(SOURCES:.cpp=.o)
	

all: main_knn

main_knn: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) main_knn.cpp -o knn $(LDADD)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

.cc.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf tree/*.o
	@rm -rf knn
	@rm -rf range
