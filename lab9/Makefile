CC=clang
CXX=clang++

CFLAGS=-std=c17 -g -O2
CXXFLAGS=-std=c++20 -g -O2

LDFLAGS=

BUILD_DIR=build
EXECUTABLE_NAME=safe_read_test
EXECUTABLE=$(BUILD_DIR)/$(EXECUTABLE_NAME)

SOURCES=$(shell find src -type f -name "*.cpp")
OBJECTS=$(SOURCES:.cpp=.o)

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) -o $@ $^

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
