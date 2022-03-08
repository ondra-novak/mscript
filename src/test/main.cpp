/*
 * main.cpp
 *
 *  Created on: 13. 2. 2022
 *      Author: ondra
 */

#include <string_view>
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
	run,
	debug
};

json::NamedEnum<Action> strAction({
	{Action::parse,"parse"},
	{Action::showcode,"showcode"},
	{Action::run,"run"},
	{Action::debug,"debug"}
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

static void printValue(const mscript::Value &v) {
	if (v.type() == json::array && (v.flags() & mscript::paramPackValue)) {
		if (v.empty()) std::cout << "()";
		else {
			char c = '(';
			for (mscript::Value x :v) {
				std::cout << c;
				printValue(x);
				c = ',';
			}
			std::cout<<')';
		}
	} else {
		auto r = v.stringify();
		if (r.length() > 50) std::cout << r.str().substr(0,50) << "...";
		else std::cout << r.c_str();
	}
}

static void show_instruction(mscript::Block::DisEvent ev, std::intptr_t number, std::string_view text) {
	using namespace mscript;
	switch(ev) {
	case Block::DisEvent::code: std::cout << number << "\t" << text << std::endl;break;
	case Block::DisEvent::begin_block: std::cout<<std::endl<<"Block: " << number << std::endl<<std::endl;break;
	case Block::DisEvent::end_block: std::cout<<std::endl<<"End of block: " << number << std::endl<<std::endl;break;
	case Block::DisEvent::begin_fn: std::cout<<std::endl<<"Function: " << number << "  " << text << "=>" << std::endl<<std::endl;break;
	case Block::DisEvent::end_fn: std::cout<<std::endl<<"End function: " << number << std::endl<<std::endl;break;
	case Block::DisEvent::location: std::cout<<"Code location: " << text << ":" << number << std::endl<< std::endl;break;
	default: break;
	};
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



	auto block = mscript::compile(elements, getVirtualMachineRuntime(), {"input",1});
	block.disassemble(show_instruction);

	return 0;

}

static int run(CmdArgIter &iter, bool debug) {

	using namespace mscript;

	std::ifstream fin;
	int e = openFile(iter, fin);
	if (e) return e;

	std::vector<mscript::Element> elements;
	parseScript([&](){
		return fin.get();
	}, elements);


	Value global = getVirtualMachineRuntime();

	auto block = packToValue(mscript::compile(elements, global, {"input",1}));

	global.setItems({
		{"print",defineSimpleFn([](const ValueList &ppack)->Value{
			for (Value v:ppack) {
				std::cout << v.toString();
			}
			return nullptr;
		})},
		{"printnl",defineSimpleFn([](const ValueList &ppack)->Value{
			for (Value v:ppack) {
				std::cout << v.toString();
			}
			std::cout << std::endl;
			return nullptr;
		})}
	});

	VirtualMachine vm;
	vm.setGlobalScope(global);
	Value v;
	if (debug) {
		vm.push_scope(Value());
		vm.push_scope(Value());
		vm.push_task(std::make_unique<BlockExecution>(block));
		do {
			const auto &ts = vm.getTaskStack();
			const BlockExecution *bt = nullptr;
			if (!ts.empty()) bt = dynamic_cast<const BlockExecution *>(ts.back().get());
			if (bt) {


				std::cout << "Tasks:" << std::endl;
				for (const auto &x: ts) {
					auto cl = x->getCodeLocation();
					std::cout << "\t" << reinterpret_cast<std::uintptr_t>(x.get()) << "\t";
					if (cl.has_value()) std::cout << cl->file << ":" << cl->line << std::endl;
					else std::cout << "<unknown location>" << std::endl;
				}
				std::cout << std::endl;
				std::cout << "Scopes:" << std::endl;
				int i = 0;
				for (const auto &x: vm.getScopeStack()) {
					if (i) { //skip global scope
						std::cout << "\t" << i << ":\t";
						Value v = x.getBase();
						for (Value vr : v) {
							auto key = vr.getKey();
							if (x.find(key) == x.end()) {
								std::cout << vr.getKey() << "=";
								printValue(vr);
								std::cout << " ";
							}
						}
						for (const auto &vr: x) {
							if (v[vr.first].defined()) std::cout << "*";
							std::cout << vr.first << "=";
							printValue(vr.second);
							std::cout << " ";
						}
						std::cout << std::endl;
					}
					i++;
				}
				std::cout << std::endl;
				std::cout << "Calc stack: ";
				for (const auto &x: vm.getCalcStack()) {
					printValue(x);
					std::cout <<  "|";
				}
				std::cout << std::endl;

				std::cout << std::endl;
				std::cout << "Instruction:\t";
				const Block &bk = bt->getBlock();
				auto ip = bt->getIP();
				bk.disassemble_ip(ip, show_instruction);

				std::cout << "----------------" << std::endl;
			}

		} while (vm.run());
		v = vm.pop_value();
		vm.pop_scope();
		vm.pop_scope();
	} else {
		vm.setMaxExecutionTime(std::chrono::seconds(30));
		v = vm.exec(std::make_unique<BlockExecution>(block));
	}
	std::cout << std::endl << std::endl << "Result: " << v.stringify() << std::endl;


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
			case Action::run: return run(argiter, false);
			case Action::debug: return run(argiter, true);
		}

	} catch(const std::exception &e) {
		std::cerr << "Operation failed:" << std::endl;
		std::cerr << e.what() << std::endl;
		return 100;
	}

	return 0;
}



