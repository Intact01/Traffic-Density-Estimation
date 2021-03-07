all: build

build :
	g++ main.cpp -I/usr/include/python3.8 -lpython3.8 -lboost_program_options -o main.out -std=c++11 `pkg-config --cflags --libs opencv`
testrun : main.out
	./main.out -f empty
test:
	g++ test.cpp -I/usr/include/python3.8 -lpython3.8 -lboost_program_options -o test.out -std=c++11 `pkg-config --cflags --libs opencv`
clean :
	rm main.out
