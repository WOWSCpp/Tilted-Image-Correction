#include "../include/evaluator.h"
#include "../include/builtin.h"
#include "../include/parser.h"
#include "../include/tokenizer.h"

namespace INTERPRETER {

	// read-eval-print loop
	void Evaluator::main_loop() {
		string code;
		string line;
		while (getline(cin, line)) {
			code += '\n' + line;
			Tokenizer t(code);
			t.tokenize();
			if (t.unclosed <= 0) { // no unmatched parenthesis nor quotation
				print_code_eval_result(t.tokens);
				code = "";
			}
		}
	}

	vector<string> Evaluator::tokenize(const string& text) {
		return Tokenizer(text).tokenize();
	}

	vector<shared_ptr<ASTNode>> Evaluator::parse(const vector<string>& tokens) {
		return Parser(tokens).parse();
	}

	vector<shared_ptr<ASTNode>> Evaluator::parse(const string& text) {
		return Parser(Evaluator::tokenize(text)).parse();
	}

	shared_ptr<ASTNode> Evaluator::create_func_node(shared_ptr<ASTNode> n, shared_ptr<Environment> outer_env) {
		auto node(n);
		node->type = ASTNode::USERFUNC;
		node->outer_env = outer_env;
		return node;
	}

	shared_ptr<ASTNode> Evaluator::eval_input_tokens(const string& text) {
		vector<shared_ptr<ASTNode>> parsed = parse(text);
		vector<shared_ptr<ASTNode>> compiled = compile_all_trees(parsed);
		return eval_all_trees(compiled);
	}
	shared_ptr<ASTNode> Evaluator::eval_input_tokens(string&& text) {
		return eval_input_tokens(text);
	}


	shared_ptr<ASTNode> Evaluator::eval_input_tokens(const vector<string>& tokens) {
		vector<shared_ptr<ASTNode>> parsed = parse(tokens);
		vector<shared_ptr<ASTNode>> compiled = compile_all_trees(parsed);
		shared_ptr<ASTNode> final_node = eval_all_trees(compiled);
		return final_node;
	}

	void Evaluator::print_code_eval_result(const vector<string>& tokens) {
		shared_ptr<ASTNode> final_node = eval_input_tokens(tokens);
		auto str_with_type = final_node->convert_to_string() + " : " + type_to_str(final_node->type);
		cout << str_with_type << endl;
	}


	shared_ptr<ASTNode> Evaluator::apply_macro(shared_ptr<ASTNode>& macro_def_actual_arg_node, map<string, shared_ptr<ASTNode>>& macro_def_input_actual_args_map) {
		if (macro_def_actual_arg_node->type == ASTNode::LIST) {
			vector<shared_ptr<ASTNode>> macro_def_actual_args = macro_def_actual_arg_node->child_nodes;
			vector<shared_ptr<ASTNode>> res;
			for (int i = 0; i < macro_def_actual_args.size(); ++i) {
				string parameter_name = macro_def_actual_args[i]->string_value;
				if (parameter_name == "...") {
					shared_ptr<ASTNode> vargs = macro_def_input_actual_args_map[parameter_name];
					res.insert(res.end(), vargs->child_nodes.begin(), vargs->child_nodes.end());
				}
				else res.emplace_back(apply_macro(macro_def_actual_args[i], macro_def_input_actual_args_map));
			}
			return ASTNode::create_normal_node(res);
		}
		else {
			string argument_name = macro_def_actual_arg_node->string_value;
			if (macro_def_input_actual_args_map.find(argument_name) != macro_def_input_actual_args_map.end()) {
				return macro_def_input_actual_args_map[argument_name];
			}
			else {
				return macro_def_actual_arg_node;
			}
		}
	}

	shared_ptr<ASTNode> Evaluator::expand_macro(shared_ptr<ASTNode>& n) {
		vector<shared_ptr<ASTNode>> child_nodes = n->child_nodes;
		// if macro defination already exists in global_macros
		if (global_macros.find(child_nodes[0]->string_value) != global_macros.end()) {
			vector<shared_ptr<ASTNode>> macro_def_body = global_macros[child_nodes[0]->string_value];

			// get the macro Formal Parameter and Actual Argument
			vector<shared_ptr<ASTNode>> macro_def_formal_params = macro_def_body[0]->child_nodes;
			shared_ptr<ASTNode> macro_def_actual_arg_node  = macro_def_body[1];
			// map macro actual arguments in defination to actual arguments in input
			// macro defination: (#define function (name ...) (set name (func ...)))
			// macro input:      (function sum1 (x) (+ x 1))
			// {"name" : sum1, "..." : [(x), (+ x 1)]
			map<string, shared_ptr<ASTNode>> macro_def_input_actual_args_map; 

			for (int i = 0; i < macro_def_formal_params.size(); ++i) {
				string parameter_name = macro_def_formal_params[i]->string_value;
				if (parameter_name == "...") {
					// if any parameter with name "...", then find all parameters represent by ... 
					vector<shared_ptr<ASTNode>> act_args_represent_by_ellipsis;
					for (int j = i + 1; j < child_nodes.size(); ++j)
						act_args_represent_by_ellipsis.emplace_back(child_nodes[j]);
					// use Formal Parameter name to map act_args_represent_by_ellipsis
					macro_def_input_actual_args_map[parameter_name] = shared_ptr<ASTNode>(ASTNode::create_normal_node(act_args_represent_by_ellipsis));
					break;
				}
				else { // if there is no "..." in macro, like (#define add (a b) (+ a b)) 
					macro_def_input_actual_args_map[macro_def_formal_params[i]->string_value] = child_nodes[i + 1];
				}
			}
			return apply_macro(macro_def_actual_arg_node, macro_def_input_actual_args_map);
		}
		else
			return n;
	}


	shared_ptr<ASTNode> Evaluator::compile_macro(shared_ptr<ASTNode>& node) {
		switch (node->type) {
			// it must be a list if it's a macro
			case ASTNode::LIST: { // function (FUNCTION ARGUMENT ..)
				if (node->child_nodes.empty()) return node; // if no any children, it must not be a macro
				shared_ptr<ASTNode> macro_name = compile_macro(node->child_nodes[0]);
				
				// create new macro in global_macros
				if (macro_name->type == ASTNode::SYMBOL && macro_name->string_value == "#define") { // (#define add (a b) (+ a b)) ; define macro
					vector<shared_ptr<ASTNode>> macro_relations{ node->child_nodes[2], node->child_nodes[3] };
					global_macros[node->child_nodes[1]->string_value] = macro_relations;
					return ASTNode::nil;
				}
				// use already exist macro or not
				else {
					// if there already exists defined macro
					if (global_macros.find(macro_name->string_value) != global_macros.end()) {
						shared_ptr<ASTNode> expanded = expand_macro(node);
						return compile_macro(expanded);
					}
					else { // if there doesn't exist defined macro
						vector<shared_ptr<ASTNode>> new_to_compile;
						for (auto i : node->child_nodes) {
							new_to_compile.emplace_back(compile_macro(i));
						}
						return ASTNode::create_normal_node(new_to_compile);
					}
				}
			}
			default: return node;
		}
	}

	vector<shared_ptr<ASTNode>> Evaluator::compile_all_trees(vector<shared_ptr<ASTNode>>& trees) {
		vector<shared_ptr<ASTNode>> compiled;
		int last = trees.size() - 1;
		for (int i = 0; i <= last; ++i) {
			compiled.emplace_back(compile_macro(trees[i]));
		}
		return compiled;
	}

	shared_ptr<ASTNode> Evaluator::eval_all_trees(vector<shared_ptr<ASTNode>>& trees) {

		int last = trees.size() - 1;
		if (last < 0) return ASTNode::nil;
		for (int i = 0; i < last; ++i) {
			ASTNode::eval_one_tree(trees[i], env_avatar);
		}
		auto final_node = ASTNode::eval_one_tree(trees[last], env_avatar);
		for (auto c : final_node->child_nodes) {
			if (c->type == ASTNode::INT) {
				ASTNode::v_list.emplace_back(c->int_value);
			}
		}
		return final_node;

	}

	void Evaluator::init_env() {
		env_avatar = make_shared<Environment>(Environment());

		env_avatar->env[EnviromentHelper::to_code("true")] = ASTNode::create_normal_node(true);
		env_avatar->env[EnviromentHelper::to_code("false")] = ASTNode::create_normal_node(false);
			
		env_avatar->env[EnviromentHelper::to_code("+")] = ASTNode::create_normal_node(builtin_plus);
		env_avatar->env[EnviromentHelper::to_code("-")] = ASTNode::create_normal_node(builtin_minus);
		env_avatar->env[EnviromentHelper::to_code("*")] = ASTNode::create_normal_node(builtin_mul);
		env_avatar->env[EnviromentHelper::to_code("/")] = ASTNode::create_normal_node(builtin_div);
		env_avatar->env[EnviromentHelper::to_code("%")] = ASTNode::create_normal_node(builtin_mod);
		env_avatar->env[EnviromentHelper::to_code("!")] = ASTNode::create_normal_node(builtin_not);
		env_avatar->env[EnviromentHelper::to_code("==")] = ASTNode::create_normal_node(builtin_eq);
		env_avatar->env[EnviromentHelper::to_code("<")] = ASTNode::create_normal_node(builtin_lt);
		env_avatar->env[EnviromentHelper::to_code("<=")] = ASTNode::create_normal_node(builtin_leq);
		env_avatar->env[EnviromentHelper::to_code(">")] = ASTNode::create_normal_node(builtin_gt);
		env_avatar->env[EnviromentHelper::to_code(">=")] = ASTNode::create_normal_node(builtin_geq);
	
		env_avatar->env[EnviromentHelper::to_code("def")] = ASTNode::create_builtin_node(keyword_def);
		env_avatar->env[EnviromentHelper::to_code("set")] = ASTNode::create_builtin_node(keyword_set);
		env_avatar->env[EnviromentHelper::to_code("and")] = ASTNode::create_builtin_node(keyword_and);
		env_avatar->env[EnviromentHelper::to_code("or")] = ASTNode::create_builtin_node(keyword_or);
		env_avatar->env[EnviromentHelper::to_code("if")] = ASTNode::create_builtin_node(keyword_if);
		env_avatar->env[EnviromentHelper::to_code("func")] = ASTNode::create_builtin_node(keyword_func);
		env_avatar->env[EnviromentHelper::to_code("while")] = ASTNode::create_builtin_node(keyword_while);

		env_avatar->env[EnviromentHelper::to_code("list")] = ASTNode::create_normal_node(builtin_list);
		env_avatar->env[EnviromentHelper::to_code("apply")] = ASTNode::create_normal_node(function_apply);
		env_avatar->env[EnviromentHelper::to_code("push_back")] = ASTNode::create_normal_node(function_push_back);
		env_avatar->env[EnviromentHelper::to_code("head")] = ASTNode::create_normal_node(function_head);
		env_avatar->env[EnviromentHelper::to_code("tail")] = ASTNode::create_normal_node(function_tail);
		env_avatar->env[EnviromentHelper::to_code("rm_head")] = ASTNode::create_normal_node(function_rm_head);
		env_avatar->env[EnviromentHelper::to_code("rm_tail")] = ASTNode::create_normal_node(function_rm_tail);
		env_avatar->env[EnviromentHelper::to_code("nth_element")] = ASTNode::create_normal_node(function_nth_element);
		env_avatar->env[EnviromentHelper::to_code("size")] = ASTNode::create_normal_node(function_size);
		env_avatar->env[EnviromentHelper::to_code("cons")] = ASTNode::create_normal_node(function_cons);
		env_avatar->env[EnviromentHelper::to_code("max")] = ASTNode::create_normal_node(function_max);
		env_avatar->env[EnviromentHelper::to_code("min")] = ASTNode::create_normal_node(function_min);
		env_avatar->env[EnviromentHelper::to_code("foldl")] = ASTNode::create_normal_node(function_foldl);
		env_avatar->env[EnviromentHelper::to_code("map")] = ASTNode::create_normal_node(function_map);
	}
}