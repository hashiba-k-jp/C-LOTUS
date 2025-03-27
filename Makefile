# install dependenies (for mac):
# brew install yaml-cpp
# brew install libomp

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 $(INCLUDES) -Wno-unused-parameter -Xpreprocessor -fopenmp -Wno-macro-redefined
# LDFLAGS = -L$(shell brew --prefix yaml-cpp)/lib -lyaml-cpp -L$(shell brew --prefix libomp)/lib -lomp
# INCLUDES = -I$(shell brew --prefix yaml-cpp)/include -I$(shell brew --prefix libomp)/include
SRCS = main.cpp
HEADERS = lotus.h util.h as_class.h routing_table.h data_struct.h util_convert.h
OBJS = $(SRCS:.cpp=.o)
TARGET = main

ifeq ($(OS), Windows_NT)
    $(error "Windows is not supported by this Makefile.")
else
    OS_NAME := $(shell uname -s)
endif

ifeq ($(OS_NAME), Darwin) # MacOS flags
    LDFLAGS  = -L$(shell brew --prefix yaml-cpp)/lib -lyaml-cpp -L$(shell brew --prefix libomp)/lib -lomp
    INCLUDES = -I$(shell brew --prefix yaml-cpp)/include -I$(shell brew --prefix libomp)/include
else
    ifeq ($(OS_NAME), Linux) # Linux
        # to find the PATH_OPENMP
        # find /usr/lib /usr/local/lib -name "libgomp.so*"
        PATH_INCLUDES = $(HOME)/.local# the path download the YAML-CPP
        PATH_OPENMP   = /usr/lib/gcc/x86_64-linux-gnu/9
        LDFLAGS  = -L$(PATH_INCLUDES)/lib -lyaml-cpp -L$(PATH_OPENMP) -lgomp
        INCLUDES = -I$(PATH_INCLUDES)/include
        RPATH = -Wl,-rpath,$(PATH_INCLUDES)/lib
    endif
endif
#
# ifeq ($(OS_NAME), Linux)
#   # Linux-specific flags
#   CXXFLAGS += -DLINUX
#   LDFLAGS += -L/usr/local/lib -lyaml-cpp -lomp
# endif


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(RPATH)

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
