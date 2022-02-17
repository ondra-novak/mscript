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

Block compile(const std::vector<Element> &code, VirtualMachine &vm, const std::string &fname, std::size_t line);


class Compiler {
public:

	Compiler(const std::vector<Element> &code, VirtualMachine &vm, BlockBld &bld)
			:code(code), vm(vm), bld(bld) {}








protected:
	const std::vector<Element> &code;
	VirtualMachine &vm;
	BlockBld &bld;
	int curLine = 0;
	int lastLine = 0;
	std::size_t curSymbol = 0;
	Element eof{Symbol::eof};


	const Element &next();
	void commit();
	void sync(const Element &elem);
	void sync(const Symbol &symbol);

	PNode parseValue();
	PNode handleCallable(PNode expr);
	PParamPackNode parseParamPack();



};




}

#endif /* SRC_MSCRIPT_COMPILER_H_ */
