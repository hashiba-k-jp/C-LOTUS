# install dependenies (for mac):
# brew install yaml-cpp
# brew install libomp

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 $(INCLUDES) -Wno-unused-parameter -Xpreprocessor -fopenmp -Wno-macro-redefined
LDFLAGS = -L$(shell brew --prefix yaml-cpp)/lib -lyaml-cpp -L$(shell brew --prefix libomp)/lib -lomp
INCLUDES = -I$(shell brew --prefix yaml-cpp)/include -I$(shell brew --prefix libomp)/include
SRCS = main.cpp
HEADERS = lotus.h util.h as_class.h routing_table.h
OBJS = $(SRCS:.cpp=.o)
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

help:
	@echo "Usage:"
	@echo "  make        - Build the program"
	@echo "  make test   - Build and execute the program"
	@echo "  make clean  - Remove compiled files"
	@echo "  make help   - Show this help message"

.PHONY: all test clean help
