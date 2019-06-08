#include "../include/utils.h"
namespace INTERPRETER {
	namespace UTILS {
		string type_to_str(int type){
			vector<string> ast_type_for_debug{
			"NIL",
			"INT",
			"BOOL",
			"STRING",
			"SYMBOL",
			"LIST",
			"SPECIAL",
			"BUILTIN",
			"USERFUNC"
			};
			return ast_type_for_debug[type];
		};
	}
}