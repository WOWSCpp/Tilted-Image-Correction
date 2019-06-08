#include "../include/astnode.h"
#include "../include/environment.h"

namespace INTERPRETER {
    using namespace std;
	/*variables begin*/
	
	vector<int> ASTNode::v_list = vector<int>();
	shared_ptr<ASTNode> ASTNode::node_true = ASTNode::create_normal_node(ASTNode(true));
	shared_ptr<ASTNode> ASTNode::node_false = ASTNode::create_normal_node(ASTNode(false));
	shared_ptr<ASTNode> ASTNode::node_0 = ASTNode::create_normal_node(ASTNode(Int_0::value));
	shared_ptr<ASTNode> ASTNode::node_1 = ASTNode::create_normal_node(ASTNode(Int_1::value));
	shared_ptr<ASTNode> ASTNode::nil = ASTNode::create_normal_node(ASTNode());
	
	
	/*variables end*/

	/*constructor begin*/
	ASTNode::ASTNode() : type(NIL) {}
	ASTNode::ASTNode(int i) : type(INT), int_value(i) {}
	ASTNode::ASTNode(bool b) : type(BOOL), bool_value(b) {}
	ASTNode::ASTNode(builtin b) : type(BUILTIN), builtin_value(b) {}
	ASTNode::ASTNode(const string& s) : type(STRING), string_value(s) {}
	ASTNode::ASTNode(const vector<shared_ptr<ASTNode>>& nodes) : type(LIST), child_nodes(nodes) {}
	ASTNode::ASTNode(const ASTNodeType type, const string& token) : type(type), string_value(token), code(EnviromentHelper::to_code(token)) {}
	ASTNode::ASTNode(const ASTNodeType type, shared_ptr<Environment> outer_env) : type(type), outer_env(outer_env) {}
	/*constructors end*/

	/*member functions begin*/
	shared_ptr<ASTNode> ASTNode::create_normal_node(const ASTNode& n) {
		return make_shared<ASTNode>(n);
	}

	shared_ptr<ASTNode> ASTNode::create_builtin_node(const builtin& b)   {
		ASTNode n;
		n.type = ASTNode::SPECIAL;
		n.builtin_value = b;
		return create_normal_node(n);
	}

	shared_ptr<ASTNode> ASTNode::eval_one_tree(shared_ptr<ASTNode>& n, shared_ptr<Environment>& env) {
		switch(n->type) {
			case ASTNode::SYMBOL: {
				return env->get_node_by_code(n->code);
			}
			case ASTNode::LIST: { // function (FUNCTION ARGUMENT ..)
				
				shared_ptr<ASTNode> func = ASTNode::eval_one_tree(n->child_nodes[0], env);
				switch (func->type) {
					case ASTNode::SPECIAL: {
						return func->builtin_value(n->child_nodes, env);
					}
					case ASTNode::BUILTIN:
					case ASTNode::USERFUNC: {
						// evaluate tokens
						
						// handle function overloading
						
						int raw_code = n->child_nodes[0]->code;
						if (EnviromentHelper::is_overload(raw_code)) {
							// cout << "Call overloaded function." << endl;
							int params_size = n->child_nodes.size() - 1;
							string func_name = n->child_nodes[0]->string_value;
							string overload_func_name = func_name + to_string(params_size);
							int overload_code = EnviromentHelper::get_code_by_name(overload_func_name);
							auto new_node = env->get_node_by_code(overload_code);
							if (new_node->type == ASTNode::NIL) { // is it's the first defined func of the whole overload funcs
								int code = EnviromentHelper::find_first_overload_func(new_node->code);
								new_node = env->get_node_by_code(code);
							}
							func.swap(new_node);
						}
						
						vector<shared_ptr<ASTNode>> tokens;
						for (auto i = n->child_nodes.begin() + 1; i != n->child_nodes.end(); ++i) {
							tokens.emplace_back(ASTNode::eval_one_tree(*i, env));
						}
						shared_ptr<Environment> new_env(make_shared<Environment>(func->outer_env));
						return apply(func, tokens, new_env);
					}
					default:
						return ASTNode::nil;
				} // end switch (func->type)
			}
			default:
				return n;
		}
	}

	shared_ptr<ASTNode> ASTNode::apply(shared_ptr<ASTNode>& func, vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env) {
		switch (func->type) {
			case ASTNode::BUILTIN: { // apply to builtin funcitons directly
				return func->builtin_value(input_tokens, env);
			}
			case ASTNode::USERFUNC: { // for user defined functions
				vector<shared_ptr<ASTNode>> func_ast = func->child_nodes;
				// (func (ARGUMENT ..) BODY ..)
				vector<shared_ptr<ASTNode>>& new_tokens = func_ast[1]->child_nodes;

				shared_ptr<Environment> local_env(make_shared<Environment>(Environment(func->outer_env)));

				for (int i = 0; i < new_tokens.size(); ++i) { // map new tokens to environment
					local_env->env[new_tokens[i]->code] = input_tokens[i];
				}

				int last = func_ast.size() - 1;
				if (last < 2) return ASTNode::nil;
				for (int i = 2; i < last; ++i) { // body
					ASTNode::eval_one_tree(func_ast[i], local_env);
				}
				return ASTNode::eval_one_tree(func_ast[last], local_env);
			}
			default:
				return ASTNode::nil;
		}
	}
	
	int ASTNode::convert_to_int() {
        switch (type) {
			case ASTNode::INT:
				return int_value;
			case ASTNode::BOOL:
				return static_cast<int>(bool_value);
			case ASTNode::STRING:
				return atoi(string_value.c_str());
			default:
				return 0;
        }
    }

    string ASTNode::convert_to_string() {
		char buf[32];
		string res;
        switch (type) {
			case ASTNode::NIL:
				break;
			case ASTNode::INT:
				res = to_string(int_value); 
				break;
			case ASTNode::BUILTIN:
				sprintf(buf, "#<builtin:%p>", builtin_value);
				res = buf; 
				break;
			case ASTNode::SPECIAL:
				sprintf(buf, "#<builtin:%p>", builtin_value);
				res = buf; 
				break;
			case ASTNode::BOOL:
				return (bool_value ? "true" : "false");
			case ASTNode::STRING:
			case ASTNode::SYMBOL:
				return string_value;
			case ASTNode::USERFUNC:
			case ASTNode::LIST:
				{
					res = '(';
					for (auto iter = child_nodes.begin(); iter != child_nodes.end(); iter++) {
						if (iter != child_nodes.begin()) res += ' ';
						res += (*iter)->convert_to_string();
					}
					res += ')';
					break;
				}
			default:;
        }
		return res;
    }
} // namespace INTERPRETER
