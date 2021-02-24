all: build

build :
	g++ main.cpp -lboost_program_options -o main.out -std=c++11 `pkg-config --cflags --libs opencv`
testrun : main.out
	./main.out -f empty
clean :
	rm main.out
