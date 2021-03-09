all: build

build :
	g++ main.cpp -I/usr/include/python3.8 -lpython3.8 -lboost_program_options -o main.out -std=c++11 `pkg-config --cflags --libs opencv`
testrun : main.out
	./main.out -f empty
test:
	g++ test.cpp -I/usr/include/python3.8 -lpython3.8 -lboost_program_options -o test.out -std=c++11 `pkg-config --cflags --libs opencv`
clean :
	rm main.out


# CC		= g++
# C_FLAGS = -g -Wall

# BIN		= bin
# SRCS	= src/*.cpp
# PROG = bin/main
# SRC = src
# INCLUDE	:= include
# LIB		:= lib


# OPENCV = `pkg-config opencv --cflags --libs`
# LIBS = $(OPENCV)

# # EXECUTABLE	:= main

# # $(PROG):$(SRCS)
# # 	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)
	
# EXECUTABLE = main
# all: $(BIN)/$(EXECUTABLE)

# clean:
# 	$(RM) $(BIN)/$(EXECUTABLE)

# run: all
# 	./$(BIN)/$(EXECUTABLE)

# $(BIN)/$(EXECUTABLE): $(SRC)/*.cpp
# 	$(CC) $(C_FLAGS) -I$(INCLUDE) -L$(LIB) $^ -o $@ $(LIBS)