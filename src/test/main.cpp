/*
 * main.cpp
 *
 *  Created on: 13. 2. 2022
 *      Author: ondra
 */

#include <fstream>
#include <mscript/vm.h>
#include <mscript/function.h>
#include <mscript/block.h>
#include <mscript/compiler.h>
#include <mscript/parser.h>
#include <mscript/disasm.h>
#include <mscript/vm_rt.h>

#include <shared/cmdline.h>

enum class Action {
	parse,
	showcode,
};

json::NamedEnum<Action> strAction({
	{Action::parse,"parse"},
	{Action::showcode,"showcode"}
});

using mscript::getVirtualMachineRuntime;
using ondra_shared::CmdArgIter;

static int openFile(CmdArgIter &iter, std::ifstream &fin) {
	auto fname = iter.getNext();
	if (!fname) {
		std::cerr << "Need argument <file>" << std::endl;
		return 3;
	}
	fin.open(fname);
	if (!fin) {
		std::cerr << "Can't open file: " << fname << std::endl;
		return 4;
	}
	return 0;
}

static int testParse(CmdArgIter &iter) {
	std::ifstream fin;
	int e = openFile(iter, fin);
	if (e) return e;
	std::vector<mscript::Element> result;
	mscript::parseScript([&](){
		return fin.get();
	}, result);

	char sep = ':';
	std::cout << "Elements";
	for (const mscript::Element &x: result) {
		std::cout << sep << " " << mscript::strKeywords[x.symbol];
		if (x.data.defined()) {
			std::cout << "(";
			x.data.toStream(std::cout);
			std::cout << ")";
		}
		sep = ',';
	}
	return 0;
}

static int showcode(CmdArgIter &iter) {

	using namespace mscript;

	std::ifstream fin;
	int e = openFile(iter, fin);
	if (e) return e;

	std::vector<mscript::Element> elements;
	parseScript([&](){
		return fin.get();
	}, elements);



	auto block = mscript::compile(elements, getVirtualMachineRuntime(), {"input",0});
	block.disassemble([](Block::DisEvent ev, std::intptr_t number, std::string_view text){
		switch(ev) {
		case Block::DisEvent::code: std::cout << number << "\t" << text << std::endl;break;
		case Block::DisEvent::begin_block: std::cout<<std::endl<<"Block: " << number << std::endl<<std::endl;break;
		case Block::DisEvent::end_block: std::cout<<std::endl<<"End of block: " << number << std::endl<<std::endl;break;
		case Block::DisEvent::begin_fn: std::cout<<std::endl<<"Function: " << number << "  " << text << "=>" << std::endl<<std::endl;break;
		case Block::DisEvent::end_fn: std::cout<<std::endl<<"End function: " << number << std::endl<<std::endl;break;
		case Block::DisEvent::location: std::cout<<std::endl<<"Code location: " << text << ":" << number << std::endl;break;
		};
	});

	return 0;

}

int main(int argc, char **argv) {


	CmdArgIter argiter(argv[0], argc-1, argv+1);

	auto astr = argiter.getNext();
	if (astr == nullptr) {
		std::cerr << "Need argument action:" << std::endl;
		std::cerr << strAction.allEnums() << std::endl;
		return 1;
	}

	auto action = strAction.find(astr);
	if (!action) {
		std::cerr << "Unknown actions" << std::endl;
		std::cerr << strAction.allEnums() << std::endl;
		return 2;
	}

	try {

		switch (*action) {
			case Action::parse: return testParse(argiter);
			case Action::showcode: return showcode(argiter);
		}

	} catch(const std::exception &e) {
		std::cerr << "Operation failed:" << std::endl;
		std::cerr << e.what() << std::endl;
		return 100;
	}

	return 0;
}



