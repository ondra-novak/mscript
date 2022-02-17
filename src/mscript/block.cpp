/*
 * block.cpp
 *
 *  Created on: 15. 2. 2022
 *      Author: ondra
 */

#include <cmath>
#include <imtjson/serializer.h>
#include <imtjson/string.h>
#include "block.h"
#include "function.h"

namespace mscript {

BlockExecution::BlockExecution(Value block):block_value(block),block(getBlockFromValue(block)) {

}

bool BlockExecution::init(VirtualMachine &vm) {
	return true;
}

bool BlockExecution::run(VirtualMachine &vm) {
	if (ip >= block.code.size()) {
		return false;
	}
	Cmd cmd = static_cast<Cmd>(block.code[ip]);
	++ip;
	switch (cmd) {
		case Cmd::noop:	break;
		case Cmd::push_int_1: vm.push_value(load_int1());break;
		case Cmd::push_int_2: vm.push_value(load_int2());break;
		case Cmd::push_int_4: vm.push_value(load_int4());break;
		case Cmd::push_int_8: vm.push_value(load_int8());break;
		case Cmd::push_double: vm.push_value(load_double());break;
		case Cmd::push_const_1: vm.push_value(block.consts[load_int1()]);break;
		case Cmd::push_const_2: vm.push_value(block.consts[load_int2()]);break;
		case Cmd::def_param_pack_1: vm.define_param_pack(load_int1());break;
		case Cmd::def_param_pack_2: vm.define_param_pack(load_int2());break;
		case Cmd::dup: vm.dup_value();break;
		case Cmd::del: vm.del_value();break;
		case Cmd::swap: vm.swap_value();break;
		case Cmd::get_var_1: getVar(vm,load_int1());break;
		case Cmd::get_var_2: getVar(vm,load_int2());break;
		case Cmd::deref: deref(vm,vm.pop_value());break;
		case Cmd::deref_1: deref(vm,block.consts[load_int1()]);break;
		case Cmd::deref_2: deref(vm,block.consts[load_int2()]);break;
		case Cmd::call_fn_1: call_fn(vm,load_int1());break;
		case Cmd::call_fn_2: call_fn(vm,load_int2());break;
		case Cmd::call_method_1: call_method(vm,load_int1());break;
		case Cmd::call_method_2: call_method(vm,load_int2());break;
		case Cmd::exec_block: exec_block(vm);break;
		case Cmd::push_scope: vm.push_scope(Value());break;
		case Cmd::pop_scope: vm.pop_scope();break;
		case Cmd::push_scope_object: vm.push_scope(vm.pop_value());break;
		case Cmd::scope_to_object: vm.push_value(vm.scope_to_object());break;
		case Cmd::raise:do_raise(vm);break;
		case Cmd::reset_ir:ir = 0;break;
		case Cmd::inc_ir: ++ir;break;
		case Cmd::set_var_ir_1: set_var_parampack(vm,load_int1());break;
		case Cmd::set_var_ir_2: set_var_parampack(vm,load_int2());break;
		case Cmd::set_var_1: set_var(vm,load_int1());break;
		case Cmd::set_var_2: set_var(vm,load_int2());break;
		case Cmd::op_add: bin_op(vm,op_add);break;
		case Cmd::op_sub: bin_op(vm,op_sub);break;
		case Cmd::op_mult: bin_op(vm,op_mult);break;
		case Cmd::op_div: bin_op(vm,op_div);break;
		case Cmd::op_cmp_eq: op_cmp(vm,[](int x){return x == 0;});break;
		case Cmd::op_cmp_less: op_cmp(vm,[](int x){return x < 0;});break;
		case Cmd::op_cmp_greater: op_cmp(vm,[](int x){return x > 0;});break;
		case Cmd::op_cmp_less_eq: op_cmp(vm,[](int x){return x <= 0;});break;
		case Cmd::op_cmp_greater_eq: op_cmp(vm,[](int x){return x >= 0;});break;
		case Cmd::op_cmp_not_eq: op_cmp(vm,[](int x){return x != 0;});break;
		case Cmd::op_bool_and: bin_op(vm,op_and);break;
		case Cmd::op_bool_or: bin_op(vm,op_or);break;
		case Cmd::op_bool_not: unar_op(vm,op_not);break;
		case Cmd::op_power: bin_op(vm,op_power);break;
		case Cmd::jump_1: ip+=load_int1();break;
		case Cmd::jump_2: ip+=load_int2();break;
		case Cmd::jump_true_1: ip+=load_int1() * (vm.pop_value().getBool()?1:0);break;
		case Cmd::jump_true_2: ip+=load_int2() * (vm.pop_value().getBool()?1:0);break;
		case Cmd::jump_false_1: ip+=load_int1() * (vm.pop_value().getBool()?0:1);break;
		case Cmd::jump_false_2: ip+=load_int2() * (vm.pop_value().getBool()?0:1);break;
		case Cmd::exit_block: ip = block.code.size();break;
		case Cmd::dbg_inc_line_1: line+=load_int1();break;
		case Cmd::dbg_inc_line_2: line+=load_int2();break;
		case Cmd::push_false: vm.push_value(false);break;
		case Cmd::push_true: vm.push_value(true);break;
		case Cmd::push_null: vm.push_value(nullptr);break;
		case Cmd::push_undefined: vm.push_value(json::undefined);break;
		case Cmd::op_unary_minus: unar_op(vm, op_unar_minus);break;
		case Cmd::push_array_1: do_push_array(vm, load_int1());break;
		case Cmd::push_array_2: do_push_array(vm, load_int2());break;
		case Cmd::push_array_4: do_push_array(vm, load_int4());break;
		default: invalid_instruction(vm,cmd);
	}
	return true;
}


bool BlockExecution::exception(VirtualMachine &vm, std::exception_ptr e) {
	return false;
}

std::optional<CodeLocation> BlockExecution::getCodeLocation() const {
	return CodeLocation{block.file, line};
}

std::intptr_t BlockExecution::load_int1() {
	return static_cast<std::int8_t>(block.code[ip++]);
}

std::intptr_t BlockExecution::load_int2() {
	std::intptr_t ret = static_cast<std::int8_t>(block.code[ip++]);
	ret = ret * 256+static_cast<std::uint8_t>(block.code[ip++]);
	return ret;

}

std::intptr_t BlockExecution::load_int4() {
	std::intptr_t ret = static_cast<std::int8_t>(block.code[ip++]);
	ret = ret * 256+static_cast<std::uint8_t>(block.code[ip++]);
	ret = ret * 256+static_cast<std::uint8_t>(block.code[ip++]);
	ret = ret * 256+static_cast<std::uint8_t>(block.code[ip++]);
	return ret;
}

std::int64_t BlockExecution::load_int8() {
	std::intptr_t ret = static_cast<std::int8_t>(block.code[ip++]);
	ret = ret * 256+static_cast<std::uint8_t>(block.code[ip++]);
	ret = ret * 256+static_cast<std::uint8_t>(block.code[ip++]);
	ret = ret * 256+static_cast<std::uint8_t>(block.code[ip++]);
	ret = ret * 256+static_cast<std::uint8_t>(block.code[ip++]);
	ret = ret * 256+static_cast<std::uint8_t>(block.code[ip++]);
	ret = ret * 256+static_cast<std::uint8_t>(block.code[ip++]);
	ret = ret * 256+static_cast<std::uint8_t>(block.code[ip++]);
	return ret;
}

double BlockExecution::load_double() {
	double x = *reinterpret_cast<const double *>(block.code.data()+ip);
	ip+=8;
	return x;
}

void BlockExecution::getVar(VirtualMachine &vm, std::intptr_t idx) {
	Value name = block.consts[idx];
	Value out;
	if (!vm.get_var(name.getString(), out)) {
		variable_not_found(vm, name.getString());
	} else {
		vm.push_value(out);
	}
}

void BlockExecution::deref(VirtualMachine &vm, Value idx) {
	Value z = vm.pop_value();
	switch (idx.type()) {
	case json::number: vm.push_value(z[idx.getUInt()]);break;
	case json::string: vm.push_value(z[idx.getString()]);break;
	default: invalid_dereference(vm, idx);break;
	}
}


void BlockExecution::call_fn(VirtualMachine &vm, std::intptr_t param_pack_size) {
	Value fn = vm.pop_value();
	if (!isFunction(fn)) {
		argument_is_not_function(vm, fn);
	} else {
		vm.define_param_pack(param_pack_size);
		const AbstractFunction &fnobj = getFunction(fn);
		auto task = fnobj.call(vm,Value(),fn);
		if (task != nullptr) {
			vm.push_task(std::move(task));
		}
	}
}


void BlockExecution::call_method(VirtualMachine &vm, std::intptr_t param_pack_size) {
	Value fn = vm.pop_value();
	if (!isFunction(fn)) {
		argument_is_not_function(vm, fn);
	} else {
		Value object = vm.pop_value();
		vm.define_param_pack(param_pack_size);
		const AbstractFunction &fnobj = getFunction(fn);
		auto task = fnobj.call(vm,object,fn);
		if (task != nullptr) {
			vm.push_task(std::move(task));
		}
	}
}

void BlockExecution::exec_block(VirtualMachine &vm) {
	Value bk = vm.pop_value();
	if (!isBlock(bk)) {
		argument_is_not_block(vm, bk);
	} else {
		auto task = std::make_unique<BlockExecution>(bk);
		vm.push_task(std::move(task));
	}
}

void BlockExecution::do_raise(VirtualMachine &vm) {
	Value e = vm.pop_value();
	//TODO
	try {
		throw e;
	} catch (...) {
		vm.raise(std::current_exception());
	}
}

void BlockExecution::set_var_parampack(VirtualMachine &vm, std::intptr_t cindex) {
	auto args = vm.top_params();
	Value v = args[ir];
	ir++;
	auto name = block.consts[cindex].getString();
	if (!vm.set_var(name, v)) {
		variable_already_assigned(vm, name);
	}
}

void BlockExecution::set_var(VirtualMachine &vm, std::intptr_t cindex) {
	Value v = vm.top_value();
	auto name = block.consts[cindex].getString();
	if (!vm.set_var(name, v)) {
		variable_already_assigned(vm, name);
	}
}

void BlockExecution::bin_op(VirtualMachine &vm, Value (*fn)(const Value &a, const Value &b)) {
	Value b = vm.pop_value();
	Value a = vm.pop_value();
	vm.push_value(fn(a,b));
}

void BlockExecution::unar_op(VirtualMachine &vm, Value (*fn)(const Value &a)) {
	Value a = vm.pop_value();
	vm.push_value(fn(a));
}

void BlockExecution::op_cmp(VirtualMachine &vm, bool (*fn)(int z)) {
	Value b = vm.pop_value();
	Value a = vm.pop_value();
	vm.push_value(fn(Value::compare(a, b)));
}

Value BlockExecution::op_add(const Value &a, const Value &b) {
	if (!b.hasValue()) return b;
	switch (a.type()) {
		case json::undefined: return json::undefined;
		case json::boolean: return a.getBool() || b.getBool();
		case json::number: return a.getNumber()+b.getNumber();
		case json::string: return json::String({a.toString(),b.toString()});
		case json::array: return a.merge(b);
		case json::object: return a.merge(b);
		default: return nullptr;
	}
	return a;
}

Value BlockExecution::op_sub(const Value &a, const Value &b) {
	if (!b.hasValue()) return b;
	switch (a.type()) {
		case json::undefined: return json::undefined;
		case json::boolean: return a.getBool() || (!b.getBool());
		case json::number: return a.getNumber()-b.getNumber();
		case json::string: {
			auto pos = a.getString().find(b.getString());
			if (pos == a.getString().npos) return a;
			else return json::String({a.getString().substr(0,pos), a.getString().substr(pos+b.getString().length())});
		}
		default: return nullptr;
	}
	return a;
}

Value BlockExecution::op_mult(const Value &a, const Value &b) {
	if (!b.hasValue()) return b;
	switch (a.type()) {
		case json::undefined: return json::undefined;
		case json::boolean: return a.getBool() && b.getBool();
		case json::number: return a.getNumber() * b.getNumber();
		default: return nullptr;
	}
	return a;
}

Value BlockExecution::op_div(const Value &a, const Value &b) {
	if (!b.hasValue()) return b;
	switch (a.type()) {
		case json::undefined: return json::undefined;
		case json::boolean: return a.getBool() && !b.getBool();
		case json::number: return a.getNumber() * b.getNumber();
		default: return nullptr;
	}
	return a;
}

Value BlockExecution::op_and(const Value &a, const Value &b) {
	return a.getBool()?b:a;
}

Value BlockExecution::op_or(const Value &a, const Value &b) {
	return a.getBool()?a:b;
}

Value BlockExecution::op_power(const Value &a, const Value &b) {
	if (!b.hasValue()) return b;
		switch (a.type()) {
			case json::undefined: return json::undefined;
			case json::boolean: return a.getBool() && !b.getBool();
			case json::number: return std::pow(a.getNumber() , b.getNumber());
			default: return nullptr;
		}
		return a;
}




void BlockExecution::variable_not_found(VirtualMachine &vm, std::string_view name) {
	vm.raise(std::make_exception_ptr(VariableNotFound(name)));
}

void BlockExecution::invalid_dereference(VirtualMachine &vm, Value idx) {
	vm.raise(std::make_exception_ptr(InvalidDereference(idx)));
}

void BlockExecution::argument_is_not_function(VirtualMachine &vm, Value v) {
	vm.raise(std::make_exception_ptr(ArgumentIsNotFunction(v)));
}

void BlockExecution::argument_is_not_block(VirtualMachine &vm, Value v) {
	vm.raise(std::make_exception_ptr(ArgumentIsNotBlock(v)));
}

void BlockExecution::invalid_instruction(VirtualMachine &vm, Cmd cmd) {
	vm.raise(std::make_exception_ptr(InvalidInstruction(cmd)));
}

void BlockExecution::variable_already_assigned(VirtualMachine &vm,std::string_view name) {
	vm.raise(std::make_exception_ptr(VariableAlreadyAssigned(name)));
}

Value BlockExecution::op_not(const Value &a) {
	return !a.getBool();
}

void InvalidInstruction::what(std::string &out) const {
	out.append("Invalid instruction: ");
	out.append(std::to_string(static_cast<unsigned int>(cmd)));
}

void VariableNotFound::what(std::string &out) const {
	out.append("Variable not found: ");
	out.append(name);
}

void VariableAlreadyAssigned::what(std::string &out) const {
	out.append("Variable already assigned: ");
	out.append(name);
}

void InvalidDereference::what(std::string &out) const {
	out.append("Invalid index: '");
	idx.serialize([&](char c){out.push_back(c);});
	out.append("'");
}

void ArgumentIsNotFunction::what(std::string &out) const {
	out.append("Argument is not a function: '");
	idx.serialize([&](char c){out.push_back(c);});
	out.append("'");
}

void ArgumentIsNotBlock::what(std::string &out) const {
	out.append("Argument is not a block: '");
	idx.serialize([&](char c){out.push_back(c);});
	out.append("'");
}

void BlockExecution::do_push_array(VirtualMachine &vm, std::intptr_t count) {
	vm.define_param_pack(count);
	auto pp = vm.top_params();
	Value arr (json::array, pp.begin(), pp.end(), [](const Value &a){return a;});
	vm.del_value();
	vm.push_value(arr);
}

Value BlockExecution::op_unar_minus(const Value &a) {
	switch(a.type()) {
	case json::number: return -a.getNumber();
	case json::string: return a.reverse();
	case json::array: return a.reverse();
	default: return a;
	}
}

}
