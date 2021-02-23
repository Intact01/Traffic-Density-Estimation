all: build

build :
	g++ main.cpp -o main.out -std=c++11 `pkg-config --cflags --libs opencv`
testrun : main.out
	./main.out empty
clean :
	rm main.out
