/*
 * parser.cpp
 *
 *  Created on: 16. 2. 2022
 *      Author: ondra
 */

#include "parser.h"
namespace mscript {

json::NamedEnum<Symbol> strKeywords({

	{Symbol::identifier,"`identifier`"},
	{Symbol::number,"`number`"},
	{Symbol::string,"`string`"},
	{Symbol::s_plus,"+"},  // +
	{Symbol::s_minus,"-"},  // -
	{Symbol::s_star,"*"},   // *
	{Symbol::s_slash,"/"},  // /
	{Symbol::s_percent,"%"}, // %
	{Symbol::s_equal,"="},  // =
	{Symbol::s_not_equal,"!="}, // !=, <>
	{Symbol::s_not_equal,"<>"}, // !=, <>
	{Symbol::s_less,"<"},   // <
	{Symbol::s_greater,">"},  // >
	{Symbol::s_less_equal,"<="},  // <=
	{Symbol::s_greater_equal,">="},  // >=
	{Symbol::s_left_bracket,"("},  // (
	{Symbol::s_right_bracket,")"},  // )
	{Symbol::s_left_square_bracket,"["},  // [
	{Symbol::s_right_square_bracket,"]"}, // ]
	{Symbol::s_left_brace,"{"},  // {
	{Symbol::s_right_brace,"}"},  // }
	{Symbol::s_arrow,"=>"}, //=>
	{Symbol::s_comma,","}, // ,
	{Symbol::s_dot,"."},	 // .
	{Symbol::s_twodots, ".."},
	{Symbol::s_threedots, "..."},
	{Symbol::s_doublecolon,":"}, // ;
	{Symbol::s_semicolon,";"}, // ;
	{Symbol::s_questionmark,"?"}, // ?
	{Symbol::s_exclamation,"!"}, // !
	{Symbol::s_power,"^"}, // ^
	{Symbol::s_underscore,"_"}, // _
	{Symbol::kw_exec,"exec"},		//exec {... block ...}
	{Symbol::kw_with,"with"},        //with A {.... block ....}
	{Symbol::kw_object,"object"},		//object {.... block ....}, object A {.... block ....}
	{Symbol::kw_return,"return"},      //return A - exit block
	{Symbol::kw_if,"if"},			//if A {....}
	{Symbol::kw_else,"else"},        //else {....}, else if {....}
	{Symbol::kw_true,"true"},
	{Symbol::kw_false,"false"},
	{Symbol::kw_null,"null"},
	{Symbol::kw_undefined,"undefined"},
	{Symbol::kw_and,"and"},
	{Symbol::kw_or,"or"},
	{Symbol::kw_not,"not"},
	{Symbol::kw_while,"while"},
	{Symbol::kw_for,"for"},
	{Symbol::kw_this,"this"},
	{Symbol::eof,"`end of file`"},
	{Symbol::separator,"`new line`"},   // command separator - new line
	{Symbol::unknown_symbol,"`unknown symbol`"}


});

}


