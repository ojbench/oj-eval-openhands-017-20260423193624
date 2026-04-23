

all: code

code: main.cpp system.hpp BPTree.hpp models.hpp utility.hpp
	g++ -O3 main.cpp -o code

clean:
	rm -f code *.db

