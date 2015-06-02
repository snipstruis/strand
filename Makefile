COMPILER = clang++
FLAGS    = -std=c++14 -g -Wall -Wextra

all: r1

r1: r1.cc r.h connector.h
	$(COMPILER) r1.cc -o r1 $(FLAGS)

connector.h:
	git clone https://github.com/snipstruis/strats-connector
	cp strats-connector/connector.h .
	rm strats-connector -Rf
