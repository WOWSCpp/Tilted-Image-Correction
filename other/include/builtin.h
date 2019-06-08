#pragma once
#ifndef  BUILTIN_H
#define  BUILTIN_H
#include "astnode.h"
#include "environment.h"

namespace INTERPRETER {
	using namespace TMP::OPERATIONS;
	shared_ptr<ASTNode> keyword_def(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (def SYMBOL VALUE) ; set in the current Environment

	shared_ptr<ASTNode> keyword_if(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (if COND THEN ELSE)

	shared_ptr<ASTNode> keyword_set(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (set EXPR VALUE)

	shared_ptr<ASTNode> keyword_func(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (func (ARGUMENT ..) BODY) => lexical closure

	shared_ptr<ASTNode> keyword_and(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (and X ..) short-circuit

	shared_ptr<ASTNode> keyword_or(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (or X ..) short-circuit

	shared_ptr<ASTNode> keyword_while(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (while COND EXPR ..)

	shared_ptr<ASTNode> builtin_plus(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (+ X ..)

	shared_ptr<ASTNode> builtin_minus(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (- X ..)

	shared_ptr<ASTNode> builtin_mul(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (* X ..)

	shared_ptr<ASTNode> builtin_div(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (/ X ..)

	shared_ptr<ASTNode> builtin_lt(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (< X Y)

	shared_ptr<ASTNode> builtin_leq(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (<= X Y)

	shared_ptr<ASTNode> builtin_gt(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (> X Y)

	shared_ptr<ASTNode> builtin_geq(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (>= X Y)

	shared_ptr<ASTNode> builtin_mod(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (% DIVIDEND DIVISOR)

	shared_ptr<ASTNode> builtin_eq(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (== X ..) short-circuit                    

	shared_ptr<ASTNode> builtin_not(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (! X)

	shared_ptr<ASTNode> builtin_list(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (list X ..)

	shared_ptr<ASTNode> function_apply(vector<shared_ptr<ASTNode>>& args, shared_ptr<Environment>& env); // (apply FUNC LIST)
	
	shared_ptr<ASTNode> function_push_back(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (push_back LIST ITEM)

	shared_ptr<ASTNode> function_head(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (head LIST)

	shared_ptr<ASTNode> function_tail(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (tail LIST)

	shared_ptr<ASTNode> function_rm_head(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (rm_head LIST)

	shared_ptr<ASTNode> function_rm_tail(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (rm_tail LIST)

	shared_ptr<ASTNode> function_nth_element(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (nth_element INDEX LIST)

	shared_ptr<ASTNode> function_size(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (size LIST)

	shared_ptr<ASTNode> function_cons(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (cons X LST): Returns a new list where x is the first element and list is the rest.

	shared_ptr<ASTNode> function_max(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (max LIST)

	shared_ptr<ASTNode> function_min(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (min LIST)

	shared_ptr<ASTNode> function_foldl(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (foldl FUNC LIST)

	shared_ptr<ASTNode> function_map(vector<shared_ptr<ASTNode>>& input_tokens, shared_ptr<Environment>& env); // (map FUNC LIST)

}

#endif // ! BUILTIN_H

