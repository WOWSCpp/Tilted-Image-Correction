#include "../include/evaluator.h"

using namespace std;
using namespace INTERPRETER;
using namespace INTERPRETER::UTILS;
int main() {
	Evaluator n;
	n.init_env();

	n.eval_input_tokens("(#define function (name ...) (set name (func ...)))");
	n.eval_input_tokens("(function add (a b) (+ a b))");
	n.eval_input_tokens("(function add (a b c) (+ a b c))");
	n.eval_input_tokens("(function add (a b c d) (+ a b c d))");
	n.eval_input_tokens("(add 1 2)")->int_value ;
	n.eval_input_tokens("(add 1 2 3)")->int_value ;
	n.eval_input_tokens("(add 1 2 3 4)")->int_value ;
	n.eval_input_tokens("(+ 1 2)")->int_value ;
	n.eval_input_tokens("(^ 1 2)")->int_value ;
	n.eval_input_tokens("((func (x) (+ x 2)) 3)")->int_value ;
	n.eval_input_tokens("(function sum1 (x) (+ x 1)) (map sum1 (list 1 2 3 4))")->int_value ;
	n.eval_input_tokens("(set a (if (< 2 1) 3 4))")->int_value ;
	n.eval_input_tokens("(set a (if (and (< 2 1) (< 1 2)) 3 4))")->int_value ;
	n.eval_input_tokens("(+ (+ 1, 2), (+ 3, 4))")->int_value ;
	n.eval_input_tokens("(- (- 10, 0), (- 9, 1))")->int_value ;
	n.eval_input_tokens("(* 1, 2, 3, 4)")->int_value ;
	n.eval_input_tokens("(def a (list 1 2 3 4)) (function sum (x y) (+ x y)) (foldl sum 10 a)")->int_value ;
	n.eval_input_tokens("(function fib (n) (if (< n 2) n (+ (fib (- n 1)) (fib (- n 2)))))") ;
	n.eval_input_tokens("(fib 15)")->int_value ;
	n.eval_input_tokens("(def a (list 1 2 3)) (nth_element 0 a)")->int_value ;
	Matrix3D<double> m1(140, 140, 140);
	Matrix3D<double> m2(140, 140, 140);
	auto m3 = m1 + m2;
    return 0;
}
