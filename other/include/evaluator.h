#pragma once
#ifndef  EVALUATOR_H
#define  EVLUATOR_H
#include "astnode.h"
#include "environment.h"

namespace INTERPRETER {
	// class EvaluatorException : public exception{
	// public:
	// 	EvaluatorException(const string& message) :
	// 		exception(message.c_str()) {
	// 		cerr << message.c_str() << endl;
	// 	}
	// };

	class Evaluator {
	private:
		shared_ptr<Environment> env_avatar;
		map<string, vector<shared_ptr<ASTNode>>> global_macros;

	public:
		shared_ptr<ASTNode> eval_all_trees(vector<shared_ptr<ASTNode>>& nodes);
		shared_ptr<ASTNode> compile_macro(shared_ptr<ASTNode>& node);
		vector<shared_ptr<ASTNode>> compile_all_trees(vector<shared_ptr<ASTNode>>& nodes);
		shared_ptr<ASTNode> apply_macro(shared_ptr<ASTNode>& body, map<string, shared_ptr<ASTNode>>& vars);
		shared_ptr<ASTNode> expand_macro(shared_ptr<ASTNode>& node);

		shared_ptr<ASTNode> eval_input_tokens(const string& text);
		shared_ptr<ASTNode> eval_input_tokens(string&& text);
		shared_ptr<ASTNode> eval_input_tokens(const vector<string>& tokens);

		void print_code_eval_result(const vector<string>& tokens);
		void main_loop(); // read-eval-print loop
		void init_env();

		static vector<string> tokenize(const string& text);
		static vector<shared_ptr<ASTNode>> parse(const string& text);
		static vector<shared_ptr<ASTNode>> parse(const vector<string>& tokens);
		static shared_ptr<ASTNode> create_func_node(shared_ptr<ASTNode> n, shared_ptr<Environment> outer_env);
	};
}





#endif // ! EVALUATOR_H


