/*
 * compiler.h
 *
 *  Created on: 17. 2. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_COMPILER_H_
#define SRC_MSCRIPT_COMPILER_H_

#include "parser.h"
#include "node.h"
#include "vm.h"

namespace mscript {


class Compiler {
public:

	Compiler(Value globalScope, std::size_t compilerExecTm=10000):globalScope(globalScope),compilerExecTm(compilerExecTm) {}

	///Compile code
	/**
	 * @param loc code location
	 * @param fn function which return characters of text. It needs to return -1 for end of file
	 * @return Value which contains block which can be executed using VirtualMachine
	 */
	template<typename Fn> Value compileText(const CodeLocation &loc, Fn &&fn);
	///Compile code
	/**
	 * @param loc code location
	 * @param str string to compile
	 * @return Value which contains block which can be executed using VirtualMachine
	 */
	Value compileString(const CodeLocation &loc, const std::string_view &str);

	///Parses include file
	/**
	 * @param name name of include file
	 * @param loc location object, must be filled
	 * @param code parsed code, must be filled
	 * @retval true success, include resolved
	 * @retval false include cannot be resolved
	 */
	virtual bool onInclude(std::string_view name, CodeLocation &loc, std::vector<Element> &code) {return false;}


protected:
	PNode compile(std::vector<Element> &&code, const CodeLocation &loc);


	std::vector<Element> code;
	Value globalScope;
	std::size_t compilerExecTm;
	//BlockBld &bld;
	CodeLocation loc;
	int curLine = 0;
	int lastLine = 0;
	std::size_t curSymbol = 0;
	Element eof{Symbol::eof};


	const Element &next();
	void commit();
	void sync(const Element &elem);
	void sync(const Symbol &symbol);
	void checkBeginBlock();


	PNode compileValue();
	PValueListNode compileValueList();

	PNode parseIfElse();
	PNode handleValueSuffixes(PNode expr);
	PNode compileDefineFunction(PNode expr);
	PNode compileBlockContent();
	PNode compileBlock();
	PNode compileCommand();
	PNode compileExpression();
	PNode compileTernal();
	PNode compileRange();
	PNode compileCustomBinary();
	PNode compileAnd();
	PNode compileOr();
	PNode compileCompare();
	PNode compileAddSub();
	PNode compileMultDiv();
	PNode compilePower();
	PNode compileArray();
	PNode compileFor();
	PNode compileWhile();
	bool eatSeparators();

	PNode tryCompileAssgn();
	PNode compileBlockOrExpression();
	CompileError compileError(const std::string &text);
	void syncSeparator();
	PNode compileSwitch();
	Value expressionToConst();
	PNode compileNumber();
	PNode compileCast(PNode &&expr, PNode &&baseObj);
	PNode compileIfDefExpression();

};


template<typename Fn>
Value Compiler::compileText(const CodeLocation &loc, Fn &&fn) {
	std::vector<Element> el;
	parseScript(std::forward<Fn>(fn), el);
	return packToValue(buildCode(compile(std::move(el),loc), loc));
}


}


#endif /* SRC_MSCRIPT_COMPILER_H_ */
