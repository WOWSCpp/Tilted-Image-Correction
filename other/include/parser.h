#pragma once
#ifndef  PARSER_H
#define  PARSER_H
#include "astnode.h"
#include "token.h"

namespace INTERPRETER {
	// class ParserException : public exception {
	// private:
	// 	int pos;
	// public:
	// 	ParserException(const string& message, int pos) :
	// 		exception(message.c_str()),
	// 		pos(pos) {}
	// };

	class Parser {
	private:
		int pos;
		vector<string> tokens;
		unordered_map<string, TokenType> checker;
		unordered_map<string, int> error_candis;
		unordered_map<string, int> first_error_pos;
		void static_check(string token, int pos);
		void clear_check_maps();
		vector<shared_ptr<ASTNode>> parse_helper();

	public:
		Parser(const vector<string>& tokens);
		vector<shared_ptr<ASTNode>> parse();
		void print_check_result();
	};

	Parser::Parser(const vector<string>& tokens) : pos(0), tokens(tokens) {
		for (int i = 0; i < token_string.size(); ++i) {
			checker.insert(make_pair(token_string[i], static_cast<TokenType>(i)));
		}
	}

	void Parser::static_check(string token, int pos) {
		if (checker.find(token) == checker.end()) {
			++error_candis[token];
			first_error_pos[token] = pos;
		}
	}

	void Parser::print_check_result() {
		for (auto p : error_candis) {
			if (p.second % 2 == 1) {
				string _token = p.first;
				int _pos = first_error_pos[_token];
				//cerr << "Unexpected token '" << _token << "' at position " << _pos << endl;
			}
		}
		clear_check_maps();
	}

	void Parser::clear_check_maps() {
		first_error_pos.clear();
		error_candis.clear();
	}

	vector<shared_ptr<ASTNode>> Parser::parse_helper() {
		vector<shared_ptr<ASTNode>> child_nodes;
		for (; pos < tokens.size(); pos++) {
			string token = tokens[pos];
			if (token[0] <= 0) break;
			if (token == "(") { // list begins, create nodes
				pos++;
				child_nodes.emplace_back(ASTNode::create_normal_node(parse_helper()));
			}
			else if (token == ")") { // list ends,
				break;
			}
			else if (isdigit(token[0]) || (token[0] == '-' && token.size() >= 2 && isdigit(token[1]))) { // number
					child_nodes.emplace_back(ASTNode::create_normal_node(atoi(token.c_str())));
			}
			else { // symbol
				static_check(token, pos);
				ASTNode n = ASTNode(ASTNode::SYMBOL, token);
				child_nodes.emplace_back(ASTNode::create_normal_node(n));
			}
		}
		return child_nodes;
	}

	vector<shared_ptr<ASTNode>> Parser::parse() {
		auto parsed = parse_helper();
		print_check_result();
		return parsed;
	}
}

#endif // ! PARSER_H

