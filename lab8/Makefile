CC=clang
CXX=clang++

CFLAGS=-std=c17 -g -O2
CXXFLAGS=-std=c++20 -g -O2

LDFLAGS=

EXECUTABLE=string_ptr_test

SOURCES=$(shell find src -type f -name "*.cpp")
OBJECTS=$(SOURCES:.cpp=.o)

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) -o $@ $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
