/*
 * compiler.cpp
 *
 *  Created on: 17. 2. 2022
 *      Author: ondra
 */

#include "compiler.h"

namespace mscript {




Block compile(const std::vector<Element> &code, VirtualMachine &vm, const std::string &fname, std::size_t line) {

	BlockBld bld;


//	compile2(code, vm, bld);

	std::vector<Value> consts;
	consts.resize(bld.constMap.size());
	for (const auto &itm: bld.constMap) {
		consts[itm.second] = itm.first;
	}

	return {
		consts,
		bld.code,
		fname,
		line
	};



}



const Element& Compiler::next() {
	if (curSymbol > code.size()) return eof;
	else return code[curSymbol];
}

void Compiler::commit() {
	curSymbol++;
	if (next().symbol == Symbol::separator) curLine++;
}

void Compiler::sync(const Element &elem) {
	const Element &exp = next();
	if (exp == elem) {
		commit();
	} else {
		throw std::runtime_error(std::string("Expected element: ").append(strKeywords[exp.symbol]));
	}
}

void Compiler::sync(const Symbol &symbol) {
	const Element &exp = next();
	if (exp.symbol == symbol) {
		commit();
	} else {
		throw std::runtime_error(std::string("Expected symbol: ").append(strKeywords[exp.symbol]));
	}

}

PNode Compiler::handleCallable(PNode expr) {
	if (next().symbol == Symbol::s_left_bracket) { //it is function
		commit();
		return std::make_unique<FunctionCall>(std::move(expr), parseParamPack());
	} else {
		return expr;
	}

}

PNode Compiler::parseValue() {
	PNode out;
	auto s = next();
	switch(s.symbol) {
	case Symbol::identifier:			//can be value or function call
		out = std::make_unique<Identifier>(s.data);
		commit();
		out = handleCallable(std::move(out));
		break;
	case Symbol::number:
		out = std::make_unique<NumberNode>(s.data);
		break;
	default:
		break;
	}
	return out;
}

}
