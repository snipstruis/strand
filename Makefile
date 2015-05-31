all: main.cc connector.h
	clang++ main.cc -o strand -std=c++14 -g -Wall -Wextra

connector.h:
	git clone https://github.com/snipstruis/strats-connector
	cp strats-connector/connector.h .
	rm strats-connector -Rf
