# ifndef __TOKEN_H
# define __TOKEN_H

#include "utils.h"

namespace INTERPRETER {
	enum TokenType {
		TOKEN_TRUE,			// true
		TOKEN_FALSE,		// false
		TOKEN_OP_ADD,     	// +
		TOKEN_OP_MINUS,   	// -
		TOKEN_OP_MUL,     	// *
		TOKEN_OP_DIV,	  	// /
		TOKEN_OP_MOD,	  	// %
		TOKEN_OP_AND,	  	// and
		TOKEN_OP_OR,	  	// or
		TOKEN_OP_NOT,	  	// not
		TOKEN_OP_EQ,	  	// ==
		TOKEN_OP_LT,	  	// <
		TOKEN_OP_LEQ,     	// <=
		TOKEN_OP_GT,	  	// >
		TOKEN_OP_GEQ,	  	// >=
		TOKEN_KW_DEF,		// def
		TOKEN_KW_SET,		// set
		TOKEN_KW_IF,		// if
		TOKEN_KW_FUNC,		// func
		TOKEN_KW_WHILE,		// while
		TOKEN_KW_EVAL,    	// eval
		TOKEN_KW_LIST,		// list
		TOKEN_KW_APPLY,		// apply
		TOKEN_KW_PUSHB,		// push_back
		TOKEN_KW_HEAD,		// head
		TOKEN_KW_TAIL,		// tail
		TOKEN_KW_RMHEAD,	// rm_head
		TOKEN_KW_RMTAIL,	// rm_tail
		TOKEN_KW_NTHELE,    // nth_element
		TOKEN_KW_LEN,		// length
		TOKEN_KW_CONS,		// cons
		TOKEN_KW_FOLDL,		// foldl
		TOKEN_KW_MAX,		// max
		TOKEN_KW_MIN,		// min
		TOKEN_KW_MAPS,    	// map
	};

	vector<string> token_string = {
		"true",
		"false",
		"+",
		"-",
		"*",
		"/",
		"%",
		"and",
		"or",
		"!",
		"==",
		"<",
		"<=",
		">",
		">=",
		"def",
		"set",
		"if",
		"func",
		"while",
		"eval",
		"list",
		"apply",
		"push_back",
		"head",
		"tail",
		"rm_head",
		"rm_tail",
		"nth_element",
		"size",
		"cons",
		"foldl",
		"map",
	};


	class Token {
	public:
		TokenType   token_type;
		double      value;
		char      symbol;
		Token() :token_type(), value(0), symbol(0) {}
	};
}

#endif#
