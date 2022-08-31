all:
	g++ -std=c++17 -O0 -Wall -pedantic -Werror -Iinclude main.cpp -o run

clean:
	rm run