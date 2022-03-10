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




///Compile code
/**
 * @param code array of symbols, extracted from the text file, using parseScript()
 * @param vm virtual machine. It is used to optimize code when some parts can be evaluated during compilation.
 * You need to initialize the virtual machine with a global scope contains symbols which are available
 * during evaluation
 *
 * @note state of virtual machine is modified during compilation
 */

Block compile(const std::vector<Element> &code, Value globalScope, const CodeLocation &loc);


class Compiler {
public:

	Compiler(const std::vector<Element> &code, Value globalScope, const CodeLocation &loc)
			:code(code), globalScope(globalScope), loc(loc) {}

	PNode compile();


protected:
	const std::vector<Element> &code;
	Value globalScope;
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

};




}

#endif /* SRC_MSCRIPT_COMPILER_H_ */
