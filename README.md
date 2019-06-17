For the usage of OpenCV, just concern about these three folders: lib, src and test_sets.
If you want to use this code in Visual Studio or something others, please config your OpenCV environment and add theses files to your project.

对于能看懂中文的朋友， 可以去简书看看更详细的的描述。
https://www.jianshu.com/p/96df7c6fb24e

For the folder "other", it combined with several interesting things in C++, such as matrix manipulations, C++ Template Meta-programming, functional programming language interpreter, etc.
usages:
g++ main.cpp astnode.cpp builtin.cpp environment.cpp evaluator.cpp utils.cpp -I ../include -o main
