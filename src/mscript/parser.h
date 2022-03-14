/*
 * parser.h
 *
 *  Created on: 16. 2. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_PARSER_H_
#define SRC_MSCRIPT_PARSER_H_

#include "value.h"
#include <imtjson/parser.h>
#include <imtjson/namedEnum.h>

namespace mscript {

enum class Symbol {
	identifier,
	number,
	string,
	s_plus,  // +
	s_minus,  // -
	s_star,   // *
	s_slash,  // /
	s_percent, // %
	s_equal,  // =
	s_dequal,  // ==
	s_not_equal, // !=, <>
	s_less,   // <
	s_greater,  // >
	s_less_equal,  // <=
	s_greater_equal,  // >=
	s_left_bracket,  // (
	s_right_bracket,  // )
	s_left_square_bracket,  // [
	s_right_square_bracket, // ]
	s_left_brace,  // {
	s_right_brace,  // }
	s_arrow, //=>
	s_comma, // ,
	s_dot,	 // .
	s_twodots, //...
	s_threedots, //...
	s_doublecolon,	 // :
	s_semicolon, // ;
	s_questionmark, // ?
	s_exclamation, // !
	s_power, // ^
	s_dollar, // $
	s_amp, //@
	s_qequal, // ?=
	s_dblq, // ??
	s_cast, // ->    x->Object.method
  	kw_exec,		//exec {... block ...}
	kw_with,        //with A {.... block ....}
	kw_object,		//object {.... block ....}, object A {.... block ....}
	kw_return,      //return A - exit block
	kw_break,		//break cycle
	kw_if,			//if A {....}
	kw_else,        //else {....}, else if {....}
	kw_true,
	kw_false,
	kw_null,
	kw_undefined,
	kw_or,
	kw_and,
	kw_not,
	kw_while,
	kw_for,
	kw_switch,
	kw_constexpr,
	eof,
	separator,   // command separator - new line
	unknown_symbol
};


extern json::NamedEnum<Symbol> strKeywords;

struct Element {
	///Read symbol class
	Symbol symbol;
	///Data attached to symbol
	Value data;

	bool operator==(const Element &x) const {return symbol == x.symbol && data == x.data;}
	bool operator!=(const Element &x) const {return !operator==(x);}
};


template<typename Fn>
class Parser: public json::Parser<Fn> {
public:
	using json::Parser<Fn>::Parser;

	using Super = json::Parser<Fn>;

	Element readNext() {

		while (true) {
			int c = this->rd.nextCommit();
			while (c == '\t' || c == ' ' ) c = this->rd.nextCommit();
			switch (c) {
				case -1: return {Symbol::eof};
				case '\n': return {Symbol::separator};
				case '\r': if (this->rd.next() == '\n') this->rd.commit();
						   return {Symbol::separator};
				case '!':switch (this->rd.next()) {
					case '=': this->rd.commit(); return {Symbol::s_not_equal};;
					default: return {Symbol::s_exclamation};;
					};break;
				case '?': switch (this->rd.next()) {
					default: return {Symbol::s_questionmark};
					case '=': this->rd.commit(); return {Symbol::s_qequal};
					case '?': this->rd.commit(); return {Symbol::s_dblq};
					}; break;

				case '%': return {Symbol::s_percent};
				case '^': return {Symbol::s_power};
				case '(': return {Symbol::s_left_bracket};
				case ')': return {Symbol::s_right_bracket};
				case '*': return {Symbol::s_star};
				case '/': return {Symbol::s_slash};
				case '+': return {Symbol::s_plus};
				case '-': switch(this->rd.next()) {
					case '>': this->rd.commit(); return {Symbol::s_cast};
					default: return {Symbol::s_minus};
				};
				case '@': return {Symbol::s_amp};
				case '$': return {Symbol::s_dollar};
				case '>': switch (this->rd.next()) {
					case '=': this->rd.commit(); return {Symbol::s_greater_equal};
					default: return {Symbol::s_greater};
				};
				case '<': switch (this->rd.next()) {
					case '=': this->rd.commit(); return {Symbol::s_less_equal};
					case '>': this->rd.commit(); return {Symbol::s_not_equal};
					default: return {Symbol::s_less};
				};
				case '=': switch (this->rd.next()) {
					case '=': this->rd.commit(); return {Symbol::s_dequal};
					case '>': this->rd.commit(); return {Symbol::s_arrow};
					default: return {Symbol::s_equal};
				};
				case '.': switch (this->rd.next()) {
					case '.': this->rd.commit();
							switch (this->rd.next()) {
								case '.': this->rd.commit(); return {Symbol::s_threedots};
								default: return {Symbol::s_twodots};
							}
					default: return {Symbol::s_dot};
				};break;
				case ';': return {Symbol::s_semicolon};
				case ':': return {Symbol::s_doublecolon};
				case ',': return {Symbol::s_comma};
				case '[': return {Symbol::s_left_square_bracket};
				case ']': return {Symbol::s_right_square_bracket};
				case '{': return {Symbol::s_left_brace};
				case '}': return {Symbol::s_right_brace};
				case '"':return {Symbol::string,this->parseString()};
				case '#':  //remove comments
					c = this->rd.next();
					while (c != '\r' && c != '\n' && c != -1) {
						this->rd.commit();
						c = this->rd.next();
					};
					break;
				default:
					if (isalpha(c) || c == '_') {
						buffer.clear();
						buffer.push_back(c);
						c = this->rd.next();
						while (isalnum(c) || c == '_') {
							buffer.push_back(c);
							this->rd.commit();
							c = this->rd.next();
						}
						const Symbol *kw = strKeywords.find(buffer);
						if (kw) return {*kw};
						return {Symbol::identifier, buffer};
					} else if (isdigit(c)) {
						buffer.clear();
						buffer.push_back(c);
						c = this->rd.next();
						while (isdigit(c)) {
							buffer.push_back(c);
							this->rd.commit();
							c = this->rd.next();
						}
						return {Symbol::number, buffer};
					} else {
						throw json::ParseError("Unknown symbol", c);
					}
			}
		}

	}

protected:
	std::string buffer;

};


template<typename Fn>
void parseScript(Fn &&fn, std::vector<Element> &result) {
	Parser<Fn> p(std::forward<Fn>(fn));
	Element item = p.readNext();
	while (item.symbol != Symbol::eof) {
		result.push_back(item);
		item = p.readNext();
	}
}

}




#endif /* SRC_MSCRIPT_PARSER_H_ */
