CC		= g++
C_FLAGS = -g 

BIN		= bin
SRCS	= src/*.cpp
PROG = bin/main
SRC = src
INCLUDE	:= /usr/include/python3.8
LIB		:= -lpython3.8 -lboost_program_options


OPENCV = `pkg-config opencv --cflags --libs`
LIBS = $(OPENCV)

# EXECUTABLE	:= main

# $(PROG):$(SRCS)
# 	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)
	
EXECUTABLE = main
all: $(BIN)/$(EXECUTABLE)

clean:
	$(RM) $(BIN)/$(EXECUTABLE)

run: all
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp
	$(CC) $^ -o $@ $(C_FLAGS) -I$(INCLUDE) $(LIB) $(LIBS)