COMPILER = clang++
FLAGS    = -std=c++14 -g -Wall -Wextra

all: r1 r2

r1: r1.cc r.h connector.h
	$(COMPILER) r1.cc -o r1 $(FLAGS)

r2: r2.cc r.h connector.h
	$(COMPILER) r2.cc -o r2 $(FLAGS)

connector.h:
	git clone https://github.com/snipstruis/strats-connector
	cp strats-connector/connector.h .
	rm strats-connector -Rf
