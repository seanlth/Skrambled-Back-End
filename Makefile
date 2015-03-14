C=clang++                                                                                
FLAGS= -std=c++11 -L/usr/local/lib -lsqlite3 -lgmp -o Skrambled
all: Skrambled
Skrambled: main.cpp
	$(C) $(FLAGS) main.cpp
