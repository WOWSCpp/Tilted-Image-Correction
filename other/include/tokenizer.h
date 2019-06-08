#pragma once
#ifndef  TOKENIZER_H
#define  TOKENIZER_H
#include "utils.h"

namespace INTERPRETER {
	class Tokenizer {
	private:
		int pos;
		string input_code, token;
		void add_token();
		void skip_white_space();
	public:
		int unclosed; // number of unclosed parenthesis ( or quotation "
		vector<string> tokens;
		Tokenizer(const string& input_code) : pos(0), input_code(input_code), unclosed(0) {}
		vector<string> tokenize();
	};

	void Tokenizer::add_token() { // add accumulated string to token list
		if (token.size() > 0) { 
			tokens.emplace_back(token); 
			token = ""; 
		}
	}

	void Tokenizer::skip_white_space() {
		while (isspace(input_code[pos])) ++pos;
	}

	vector<string> Tokenizer:: tokenize() {
		skip_white_space();
		int last = input_code.size() - 1;
		for (; pos <= last; pos++) {
			char c = input_code[pos];
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
				add_token();
			}

			else if (c == '"') { // string
				unclosed++;
				add_token();
				token += '"';
				pos++;
				while (pos <= last) {
					if (input_code[pos] == '"') {
						unclosed--; 
						break; 
					}
					token += input_code[pos];
					pos++;
				}
				add_token();
			}
			else if (c == '(') { // open parathesis
				unclosed++;
				add_token();
				token += c;
				add_token();
			}
			else if (c == ')') { // open parathesis
				unclosed--;
				add_token();
				token += c;
				add_token();
			}
			else {
				token += c;
			}
		}
		add_token();
		// cout << "tokens are :  " ;
		// for (auto t : tokens)
		// 	cout << t << " ";
		// cout << endl;
		return tokens;
	}
}

#endif // ! TOKENIZER_H

