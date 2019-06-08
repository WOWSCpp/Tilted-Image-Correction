#include "../include/environment.h"
#include "../include/astnode.h"

namespace INTERPRETER {

	static map<string, int> symbol_code;
	static vector<string> symbol_name;
	static unordered_map<int, unordered_set<int>> overload_map;

	shared_ptr<ASTNode> Environment::get_node_by_code(int code) {
		
		auto found = env.find(code);
		if (found != env.end()) { // if code in inner env
			return found->second;
		}
		else {
			if (env_avatar != nullptr) { // if code in outer env
				return env_avatar->get_node_by_code(code);
			}
			else {
				return ASTNode::nil;
			}
		}
	}


	shared_ptr<ASTNode> Environment::bind_symbol_to_value(shared_ptr<ASTNode>& k, shared_ptr<ASTNode>& v) {
		env[k->code] = v;
		return v;
	}

	void Environment::bind_symbol_to_value_no_return(shared_ptr<ASTNode>& k, shared_ptr<ASTNode>& v) {
		for (auto p : env) {
			if (p.second == v) {
				if (!EnviromentHelper::is_overload(p.first) && k->code == p.first) {
					return;
				}	
			}	
		}
		env[k->code] = v;
	}

	void Environment::exchange_nodes_by_codes(int raw_code, int overload_code) {
		auto overload_node = get_node_by_code(overload_code);
		env[raw_code] = overload_node;
	}


	int EnviromentHelper::find_first_overload_func(int code) {
		for (auto p : overload_map) {
			if (p.second.find(code) != p.second.end()) {
				return p.first;
			}
		}
	}

	int EnviromentHelper::get_global_codes_size() {
		return symbol_name.size();
	}

	void EnviromentHelper::update_overload_map(int code, int overload_code) {
		overload_map[code].emplace(overload_code);
		int idx = overload_map[code].size() - 1;
		// if (idx > 0) {
		// 	cout << "Overloaded function + " << to_string(idx) << "." << endl;
		// }
	}

	bool EnviromentHelper::is_overload(int code) {
		return overload_map[code].size() > 1;
	}

	void EnviromentHelper::update_global_codes(const string& name, int code) {
		symbol_name.emplace_back(name);
		symbol_code[name] = code;
	}

	int EnviromentHelper::get_code_by_name(const string& name) {
		return symbol_code[name];
	}

	int EnviromentHelper::to_code(const string& name) {

		int res;
		auto found = symbol_code.find(name);
		if (found != symbol_code.end()) {
			return found->second;
		}
		else {
			res = symbol_code.size();
			symbol_code[name] = res;
			symbol_name.emplace_back(name);
			return res;
		}
	}


}