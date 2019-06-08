#include "../include/builtin.h"
#include "../include/evaluator.h"

namespace INTERPRETER {
	shared_ptr<ASTNode> keyword_def(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env) { // (def a (list 1 2 3 4))
		// value may need to eval firstly
		shared_ptr<ASTNode> symbol = input_tokens[1];
		shared_ptr<ASTNode> value  = ASTNode::create_normal_node(*ASTNode::eval_one_tree(input_tokens[2], env));
		return env->bind_symbol_to_value(symbol, value);
	}

	shared_ptr<ASTNode> keyword_if(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env) { // (if COND THEN ELSE)
		shared_ptr<ASTNode> COND = input_tokens[1], THEN = input_tokens[2], ELSE = input_tokens[3];
		if (ASTNode::eval_one_tree(COND, env)->bool_value) {
			return ASTNode::eval_one_tree(THEN, env);
		}
		else {
			return ASTNode::eval_one_tree(ELSE, env);
		}
	}

	shared_ptr<ASTNode> keyword_set(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env) { // (set EXPR VALUE)
		shared_ptr<ASTNode>& symbol = input_tokens[1]; // may need function overloading, so use l-value reference
		shared_ptr<ASTNode> expr   = ASTNode::eval_one_tree(symbol, env);
		shared_ptr<ASTNode> value  = ASTNode::create_normal_node(*ASTNode::eval_one_tree(input_tokens[2], env));
		if (symbol->type == ASTNode::SYMBOL && expr == ASTNode::nil) {// new variable 
			// only user-defined functions can overload
			if (value->type != ASTNode::USERFUNC) { 
				return env->bind_symbol_to_value(symbol, value);
			}
			else {
				auto res = env->bind_symbol_to_value(symbol, value);
				int raw_code = symbol->code;
				symbol->code = EnviromentHelper::get_global_codes_size();
				if (value->child_nodes.size() > 0) {
					int params_size = value->child_nodes[1]->child_nodes.size();
					symbol->string_value += to_string(params_size);
					EnviromentHelper::update_global_codes(symbol->string_value, symbol->code);
					EnviromentHelper::update_overload_map(raw_code, symbol->code);
					env->bind_symbol_to_value_no_return(symbol, value);
				}
				return res;
			}
		}
		else {
			if (value->type != ASTNode::USERFUNC) {
				swap(expr, value); // swaps the managed objects 
				return expr;
			}

			else if (env->env.find(symbol->code) != env->env.end()) {
				if (value->type == ASTNode::USERFUNC){ // may need function overloading
					auto res = env->bind_symbol_to_value(symbol, value);
					int raw_code = symbol->code;
					symbol->code = EnviromentHelper::get_global_codes_size();
					if (value->child_nodes.size() > 0) {
						int params_size = value->child_nodes[1]->child_nodes.size();
						symbol->string_value += to_string(params_size);
						EnviromentHelper::update_global_codes(symbol->string_value, symbol->code);
						EnviromentHelper::update_overload_map(raw_code, symbol->code);
						env->bind_symbol_to_value_no_return(symbol, value);
					}
					return res;
				}
			}
			
		}
	}

	shared_ptr<ASTNode> keyword_func(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env) { // (fn (ARGUMENT ..) BODY) => lexical closure
		shared_ptr<ASTNode> func = Evaluator::create_func_node(ASTNode::create_normal_node(input_tokens), make_shared<Environment>(env));
		return func;
	}

	shared_ptr<ASTNode> keyword_and(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env) { // (and X ..) short-circuit
		for (auto i = input_tokens.begin() + 1; i != input_tokens.end(); ++i) {
			if (!(ASTNode::eval_one_tree(*i, env))->bool_value) { 
				return ASTNode::node_false; 
			}
		}
		return ASTNode::node_true;
	}

	shared_ptr<ASTNode> keyword_or(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env) { // (or X ..) short-circuit
		for (auto i = input_tokens.begin() + 1; i != input_tokens.end(); ++i) {
			if ((ASTNode::eval_one_tree(*i, env))->bool_value) { 
				return ASTNode::node_true; 
			}
		}
		return ASTNode::node_false;
	}

	shared_ptr<ASTNode> keyword_while(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env) { // (while COND EXPR ..)
		shared_ptr<ASTNode>& cond = input_tokens[1];
		while (ASTNode::eval_one_tree(cond, env)->bool_value) {
			for (int i = 2; i < input_tokens.size(); ++i) {
				ASTNode::eval_one_tree(input_tokens[i], env);
			}
		}
		return ASTNode::nil;
	}

	shared_ptr<ASTNode> builtin_plus(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env) { // (+ X ..)
		int len = input_tokens.size();
		if (len <= 0) return ASTNode::node_0;
		shared_ptr<ASTNode> first = input_tokens[0];
		if (first->type == ASTNode::INT) {
			int res = first->convert_to_int();
			for (auto i = input_tokens.begin() + 1; i != input_tokens.end(); ++i) {
				res += (*i)->convert_to_int();
			}
			return ASTNode::create_normal_node(res);
		}
	}

	shared_ptr<ASTNode> builtin_minus(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env) { // (- X ..)
		int len = input_tokens.size();
		if (len <= 0) return ASTNode::node_0;
		shared_ptr<ASTNode> first = input_tokens[0];
		if (first->type == ASTNode::INT) {
			int res = first->convert_to_int();
			for (auto i = input_tokens.begin() + 1; i != input_tokens.end(); ++i) {
				res -= (*i)->convert_to_int();
			}
			return ASTNode::create_normal_node(res);
		}
	}

	shared_ptr<ASTNode> builtin_mul(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env) { // (* X ..)
		int len = input_tokens.size();
		if (len <= 0) return ASTNode::node_1;
		shared_ptr<ASTNode> first = input_tokens[0];
		if (first->type == ASTNode::INT) {
			int res = first->convert_to_int();
			for (auto i = input_tokens.begin() + 1; i != input_tokens.end(); ++i) {
				res *= (*i)->convert_to_int();
			}
			return ASTNode::create_normal_node(res);
		}
	}

	shared_ptr<ASTNode> builtin_div(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env) { // (/ X ..)
		int len = input_tokens.size();
		if (len <= 0) return ASTNode::node_1;
		shared_ptr<ASTNode> first = input_tokens[0];
		if (first->type == ASTNode::INT) {
			int res = first->convert_to_int();
			for (auto i = input_tokens.begin() + 1; i != input_tokens.end(); ++i) {
				res /= (*i)->convert_to_int();
			}
			return ASTNode::create_normal_node(res);
		}
	}

	shared_ptr<ASTNode> builtin_lt(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env) { // (< X Y)
		if (input_tokens[0]->type == ASTNode::INT) {
			return ASTNode::create_normal_node(input_tokens[0]->convert_to_int() < input_tokens[1]->convert_to_int());
		}
	}

	shared_ptr<ASTNode> builtin_leq(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env) { // (<= X Y)
		if (input_tokens[0]->type == ASTNode::INT) {
			return ASTNode::create_normal_node(input_tokens[0]->convert_to_int() <= input_tokens[1]->convert_to_int());
		}
	}

	shared_ptr<ASTNode> builtin_gt(vector<shared_ptr<ASTNode>> & input_tokens, shared_ptr<Environment> & env) { // (> X Y)
		if (input_tokens[0]->type == ASTNode::INT) {
			return ASTNode::create_normal_node(input_tokens[0]->convert_to_int() > input_tokens[1]->convert_to_int());
		}
	}

	shared_ptr<ASTNode> builtin_geq(vector<shared_ptr<ASTNode>> & input_tokens, shared_ptr<Environment> & env) { // (>= X Y)
		if (input_tokens[0]->type == ASTNode::INT) {
			return ASTNode::create_normal_node(input_tokens[0]->convert_to_int() >= input_tokens[1]->convert_to_int());
		}
	}

	shared_ptr<ASTNode> builtin_mod(vector<shared_ptr<ASTNode>> & input_tokens, shared_ptr<Environment> & env) { // (% DIVIDEND DIVISOR)
		return ASTNode::create_normal_node(input_tokens[0]->convert_to_int() % input_tokens[1]->convert_to_int());
	}

	shared_ptr<ASTNode> builtin_eq(vector<shared_ptr<ASTNode>> & input_tokens, shared_ptr<Environment> & env) { // (== X ..) short-circuit                    
		shared_ptr<ASTNode> first = input_tokens[0];
		if (first->type == ASTNode::INT) {
			int firstv = first->convert_to_int();
			for (auto i = input_tokens.begin() + 1; i != input_tokens.end(); ++i) {
				if ((*i)->convert_to_int() != firstv) { return ASTNode::node_false; }
			}
		}
		return ASTNode::node_true;
	}

	shared_ptr<ASTNode> builtin_not(vector<shared_ptr<ASTNode>> & input_tokens, shared_ptr<Environment> & env) { // (! X)
		return ASTNode::create_normal_node(!(input_tokens[0]->bool_value));
	}

	shared_ptr<ASTNode> builtin_list(vector<shared_ptr<ASTNode>> & input_tokens, shared_ptr<Environment> & env) { // (list X1, X2, ...)
		vector<shared_ptr<ASTNode>> res;
		for (auto& n : input_tokens) {
			res.emplace_back(n);
		}
		return ASTNode::create_normal_node(res);
	}

	shared_ptr<ASTNode> function_apply(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env) { // (apply FUNC LIST)
		shared_ptr<ASTNode> func = input_tokens[0];
		vector<shared_ptr<ASTNode>> list = input_tokens[1]->child_nodes;
		return ASTNode::apply(func, list, env);
	}

	shared_ptr<ASTNode> function_push_back(vector<shared_ptr<ASTNode>> & input_tokens, shared_ptr<Environment> & env) { // (push_back LIST ITEM)
		input_tokens[0]->child_nodes.emplace_back(ASTNode::create_normal_node(*input_tokens[1]));
		return input_tokens[0];
	}

	shared_ptr<ASTNode> function_head(vector<shared_ptr<ASTNode>> & input_tokens, shared_ptr<Environment> & env) { // (head LIST)
		auto& v = input_tokens[0]->child_nodes;
		// if (v.empty()) {
		// 	new EvaluatorException("fatal error: try to access into empty list");
		// }
		shared_ptr<ASTNode> head = v.front();
		return head;
	}

	shared_ptr<ASTNode> function_tail(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env) { // (tail LIST)
		auto& v = input_tokens[0]->child_nodes;
		// if (v.empty()) {
		// 	new EvaluatorException("fatal error: try to access into empty list");
		// }
		shared_ptr<ASTNode> tail = v.back();
		return tail;
	}

	shared_ptr<ASTNode> function_rm_head(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env) { // (rm_head LIST)
		auto& v = input_tokens[0]->child_nodes;
		// if (v.empty()) {
		// 	new EvaluatorException("fatal error: try to access into empty list");
		// }
		shared_ptr<ASTNode> head = v.front();
		v.erase(v.begin());
		return head;
	}

	shared_ptr<ASTNode> function_rm_tail(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env) { // (rm_tail LIST)
		auto& v = input_tokens[0]->child_nodes;
		// if (v.empty()) {
		// 	new EvaluatorException("fatal error: try to access into empty list");
		// }
		shared_ptr<ASTNode> tail = v.back();
		v.pop_back();
		return tail;
	}

	shared_ptr<ASTNode> function_nth_element(vector<shared_ptr<ASTNode>> & input_tokens, shared_ptr<Environment> & env) { // (nth_element INDEX LIST)
		int index = input_tokens[0]->convert_to_int();
		auto elements = input_tokens[1]->child_nodes;
		if (index < elements.size()) {
			return elements[index];
		}
		//new EvaluatorException("fatal error: vector subscript out of range");
		return make_shared<ASTNode>(new ASTNode());
	}

	shared_ptr<ASTNode> function_size(vector<shared_ptr<ASTNode>> & input_tokens, shared_ptr<Environment> & env) { // (size LIST)
		return ASTNode::create_normal_node(static_cast<int>(input_tokens[0]->child_nodes.size()));
	}

	// (cons X list): Returns a new list where x is the first element and list is the rest.
	shared_ptr<ASTNode> function_cons(vector<shared_ptr<ASTNode>> & input_tokens, shared_ptr<Environment> & env) { 
		shared_ptr<ASTNode> X = input_tokens[0];
		shared_ptr<ASTNode> list = input_tokens[1];
		vector<shared_ptr<ASTNode>> res{X};
		res.insert(res.end(), list->child_nodes.begin(), list->child_nodes.end());
		return ASTNode::create_normal_node(res);
	}

	shared_ptr<ASTNode> function_max(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env) { // (max LIST)
		shared_ptr<ASTNode> list = input_tokens[0];
		shared_ptr<ASTNode> max_node = *max_element(list->child_nodes.begin(), list->child_nodes.end(), [](shared_ptr<ASTNode> & n1, shared_ptr<ASTNode> & n2) {
			return n1->int_value < n2->int_value; });
		return max_node;
	}

	shared_ptr<ASTNode> function_min(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env) { // (min LIST)
		shared_ptr<ASTNode> list = input_tokens[0];
		shared_ptr<ASTNode> min_node = *min_element(list->child_nodes.begin(), list->child_nodes.end(), [](shared_ptr<ASTNode> & n1, shared_ptr<ASTNode> & n2) {
			return n1->int_value < n2->int_value; });
		return min_node;
	}

	shared_ptr<ASTNode> function_foldl(vector<shared_ptr<ASTNode>> & input_tokens, shared_ptr<Environment> & env) { // (foldl FUNC INIT LIST)
		shared_ptr<ASTNode> func = input_tokens[0];
		shared_ptr<ASTNode> init = input_tokens[1];
		shared_ptr<ASTNode> list = input_tokens[2];
		shared_ptr<ASTNode> acc = init;
		vector<shared_ptr<ASTNode>> new_tokens(2);
		for (int i = 0; i < list->child_nodes.size(); ++i) {
			new_tokens[0] = acc;
			new_tokens[1] = list->child_nodes[i];
			acc = ASTNode::apply(func, new_tokens, env);
		}
		return acc;
	}

	shared_ptr<ASTNode> function_map(vector<shared_ptr<ASTNode>> & input_tokens, shared_ptr<Environment> & env) { // (map FUNC LIST)
		shared_ptr<ASTNode> func = input_tokens[0];
		shared_ptr<ASTNode> list = input_tokens[1];
		vector<shared_ptr<ASTNode>> acc;
		vector<shared_ptr<ASTNode>> new_tokens(1);
		for (int i = 0; i < list->child_nodes.size(); ++i) {
			new_tokens[0] = list->child_nodes[i];
			acc.emplace_back(ASTNode::apply(func, new_tokens, env));
		}
		return ASTNode::create_normal_node(acc);
	}
}
