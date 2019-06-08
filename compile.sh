#!/bin/sh
g++ -std=c++11 main.cpp astnode.cpp builtin.cpp environment.cpp evaluator.cpp utils.cpp  -I ../include  -o main
