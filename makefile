all:
	g++ -std=c++11 -O3 -g -Wall -fmessage-length=0 -o ThreesT Threes.cpp
clean:
	rm Threes
