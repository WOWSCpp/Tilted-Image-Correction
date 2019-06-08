#pragma once
#ifndef  ENVIRONMENT_H
#define  ENVIRONMENT_H
#include "utils.h"

namespace INTERPRETER {
	class ASTNode;

	class Environment {
	public:
		map<int, shared_ptr<ASTNode>> env;
		shared_ptr<Environment> env_avatar;
		Environment() : env_avatar(nullptr) {}
		Environment(shared_ptr<Environment> env_avatar) : env_avatar(env_avatar) {}

		shared_ptr<ASTNode> get_node_by_code(int code);
		shared_ptr<ASTNode> bind_symbol_to_value(shared_ptr<ASTNode>& k, shared_ptr<ASTNode>& v);
		void bind_symbol_to_value_no_return(shared_ptr<ASTNode>& k, shared_ptr<ASTNode>& v);
		void exchange_nodes_by_codes(int raw_code, int overload_code);

	};

	class EnviromentHelper {
	public:
		static int to_code(const string& name);
		static int get_global_codes_size();
		static void update_global_codes(const string& name, int code);
		static void update_overload_map(int code, int overload_code);
		static int get_code_by_name(const string& name);
		static bool is_overload(int code);
		static int find_first_overload_func(int code);
	};
}



#endif // ! ENVIRONMENT_H

