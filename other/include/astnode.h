#pragma once
#ifndef ASTNODE_H
#define ASTNODE_H
#include "utils.h"

namespace INTERPRETER {
	using namespace std;
	using namespace UTILS;
	using namespace TMP::BASICTYPE;

	class Environment;
	
    class ASTNode {
		using builtin = shared_ptr<ASTNode>(*)(vector<shared_ptr<ASTNode>>& args, shared_ptr<Environment>& env);
	public:
		enum ASTNodeType {
			NIL,
			INT,
			BOOL,
			STRING,
			SYMBOL,
			LIST,
			SPECIAL,
			BUILTIN,
			USERFUNC
		};
		ASTNodeType type;
		// constructors
		ASTNode();
		ASTNode(int i);
		ASTNode(bool b);
		ASTNode(builtin b);
		ASTNode(const string& s);
		ASTNode(const vector<shared_ptr<ASTNode>>& nodes);
		ASTNode(const ASTNodeType type, const string& token);
		ASTNode(const ASTNodeType type, shared_ptr<Environment> outer_env);

		// member variables
		
		int code; // code for SYMBOL

		Int<int>::value_type int_value;
        bool bool_value;		
		builtin builtin_value;
        string string_value;
		
		vector<shared_ptr<ASTNode>> child_nodes; // for contents contained in each "( )"
		shared_ptr<Environment> outer_env; // if USERFUNC

		// member functions
        int convert_to_int(); // convert to int
        string convert_to_string(); // convert to string
		

		// static members
		static vector<int> v_list;
		static shared_ptr<ASTNode> node_true;
		static shared_ptr<ASTNode> node_false;
		static shared_ptr<ASTNode> node_0;
		static shared_ptr<ASTNode> node_1;
		static shared_ptr<ASTNode> nil;
		
		static shared_ptr<ASTNode> create_normal_node(const ASTNode& n);
		static shared_ptr<ASTNode> create_builtin_node(const builtin& b);
		static shared_ptr<ASTNode> eval_one_tree(shared_ptr<ASTNode>& n, shared_ptr<Environment>& env);
		static shared_ptr<ASTNode> apply(shared_ptr<ASTNode>& func, vector<shared_ptr<ASTNode>>& args, shared_ptr<Environment>& env);
    };

    
} // namespace INTERPRETER
#endif 
