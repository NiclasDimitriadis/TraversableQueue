CXX = g++-14
COMPILER_FLAGS = -std=c++23 -ggdb
INCLUDE_DIRS = -I ../src/production -I ../src/unittests -I ../dependencies/TMP_lib

all: build/Unittests.o build/Unittests

build/Unittests.o: ../src/unittests/Unittests.cpp
	$(CXX) $(COMPILER_FLAGS) $(INCLUDE_DIRS) -c $< -o $@

build/Unittests: build/Unittests.o
	$(CXX) $(COMPILER_FLAGS) $(INCLUDE_DIRS) $< -o $@

.PHONY: clean all

clean:
	rm -rf  build/*
