#CXX = g++
CXX = clang++

CXXFLAGS = -g -I../include -I../agg/include $(shell mapnik-config --cflags) -DMAPNIKDIR=\"$(shell mapnik-config --input-plugins)\" -ftemplate-depth-400

LIBS = $(shell mapnik-config --ldflags --libs --dep-libs) -lboost_program_options

SRC = expression_test.cpp

OBJ = $(SRC:.cpp=.o)

BIN = test

all : $(SRC) $(BIN)

$(BIN) : $(OBJ)
	$(CXX) $(OBJ) $(LIBS) -o $@  

.cpp.o :
	$(CXX) -c $(CXXFLAGS) $< -o $@

.PHONY : clean

clean: 
	rm -f $(OBJ)
	rm -f $(BIN)
