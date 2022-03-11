/*
 * block.cpp
 *
 *  Created on: 15. 2. 2022
 *      Author: ondra
 */

#include <cmath>
#include <imtjson/serializer.h>
#include <imtjson/string.h>
#include "range.h"
#include "block.h"
#include "function.h"
#include "dynmap.h"

namespace mscript {


json::NamedEnum<Cmd> strCmd({
	{Cmd::noop,"NOP"},
	{Cmd::push_int_1,"PUSHI $1"},
	{Cmd::push_int_2,"PUSHI $2"},
	{Cmd::push_int_4,"PUSHI $4"},
	{Cmd::push_int_8,"PUSHI $8"},
	{Cmd::push_double,"PUSHF $F"},
	{Cmd::push_const_1,"PUSHC @1"},
	{Cmd::push_const_2,"PUSHC @2"},
	{Cmd::begin_list,"LSTART"},
	{Cmd::close_list,"LFINISH"},
	{Cmd::collapse_list_1,"COLPSLST $1"},
	{Cmd::expand_array, "EXPDARR"},
	{Cmd::dup,"DUP"},
	{Cmd::dup_1,"DUP $1"},
	{Cmd::del,"DEL"},
	{Cmd::swap,"SWAP"},
	{Cmd::swap_1,"SWAP $1"},
	{Cmd::get_var_1,"PUSH @1"},
	{Cmd::get_var_2,"PUSH @2"},
	{Cmd::deref,"DEREF"},
	{Cmd::deref_1,"DEREF @1"},
	{Cmd::deref_2,"DEREF @2"},
	{Cmd::call,"CALL"},
	{Cmd::call_1,"CALL @1"},
	{Cmd::call_2,"CALL @2"},
	{Cmd::mcall_1,"MCALL @1"},
	{Cmd::mcall_2,"MCALL @2"},
	{Cmd::vlist_pop, "VLIST_POP"},
	{Cmd::combine, "COMBINE"},
	{Cmd::exec_block,"EXEC"},
	{Cmd::push_scope,"PUSH_SCOPE"},
	{Cmd::pop_scope,"POP_SCOPE"},
	{Cmd::push_scope_object,"OBJ2SCOPE"},
	{Cmd::scope_to_object,"SCOPE2OBJ"},
	{Cmd::raise,"RAISE"},
	{Cmd::push_true,"PUSHC true"},
	{Cmd::push_false,"PUSHC false"},
	{Cmd::push_null,"PUSHC null"},
	{Cmd::push_undefined,"PUSHC undefined"},
	{Cmd::push_zero_int,"PUSHC 0"},
	{Cmd::push_array_1,"MKARRAY $1"},
	{Cmd::push_array_2,"MKARRAY $2"},
	{Cmd::push_array_4,"MKARRAY $4"},
	{Cmd::set_var_1,"STORE @1"},
	{Cmd::set_var_2,"STORE @2"},
	{Cmd::pop_var_1,"POP @1"},
	{Cmd::pop_var_2,"POP @2"},
	{Cmd::op_add,"ADD"},
	{Cmd::op_sub,"SUB"},
	{Cmd::op_mult,"MULT"},
	{Cmd::op_div,"DIV"},
	{Cmd::op_cmp_eq,"EQ"},
	{Cmd::op_cmp_less,"LESS"},
	{Cmd::op_cmp_greater,"GREATER"},
	{Cmd::op_cmp_less_eq,"LESS_EQ"},
	{Cmd::op_cmp_greater_eq,"GREATER_EQ"},
	{Cmd::op_cmp_not_eq,"NOT_EQ"},
	{Cmd::op_bool_and,"AND"},
	{Cmd::op_bool_or,"OR"},
	{Cmd::op_bool_not,"NOT"},
	{Cmd::op_power,"POWER"},
	{Cmd::op_unary_minus,"NEG"},
	{Cmd::op_mod,"MOD"},
	{Cmd::jump_1,"JMP ^1"},
	{Cmd::jump_2,"JMP ^2"},
	{Cmd::jump_true_1,"JTRUE ^1"},
	{Cmd::jump_true_2,"JTRUE ^2"},
	{Cmd::jump_false_1,"JFALSE ^1"},
	{Cmd::jump_false_2,"JFALSE ^2"},
	{Cmd::exit_block,"EXIT"},
	{Cmd::is_def_1,"ISDEF @1"},
	{Cmd::is_def_2,"ISDEF @2"},
	{Cmd::op_add_const_1,"ADD $1"},
	{Cmd::op_add_const_2,"ADD $2"},
	{Cmd::op_add_const_4,"ADD $4"},
	{Cmd::op_add_const_8,"ADD $8"},
	{Cmd::op_negadd_const_1,"NEGADD $1"},
	{Cmd::op_negadd_const_2,"NEGADD $2"},
	{Cmd::op_negadd_const_4,"NEGADD $4"},
	{Cmd::op_negadd_const_8,"NEGADD $8"},
	{Cmd::op_mult_const_1,"MULT $1"},
	{Cmd::op_mult_const_2,"MULT $2"},
	{Cmd::op_mult_const_4,"MULT $4"},
	{Cmd::op_mult_const_8,"MULT $8"},
	{Cmd::op_cmp_eq_1,"EQ @1"},
	{Cmd::op_cmp_eq_2,"EQ @2"},
	{Cmd::op_mkrange,"MKRANGE"},
	{Cmd::op_checkbound,"CHKBOUND"},


});

BlockExecution::BlockExecution(Value block):block_value(block),block(getBlockFromValue(block)) {

}

BlockExecution::BlockExecution(const BlockExecution &other)
:block_value(other.block_value),block(getBlockFromValue(other.block_value)) {

}

bool BlockExecution::init(VirtualMachine &vm) {
	return true;
}

bool BlockExecution::run(VirtualMachine &vm) {
	if (ip >= block.code.size()) {
		return false;
	}
	try {
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
			case Cmd::begin_list: vm.begin_list();break;
			case Cmd::close_list: vm.finish_list();break;
			case Cmd::expand_array: vm.push_values(vm.pop_value());break;
			case Cmd::collapse_list_1: vm.collapse_param_pack();vm.push_value(vm.pop_value().slice(load_int1()));break;
			case Cmd::dup: vm.dup_value();break;
			case Cmd::dup_1: vm.dup_value(load_int1());break;
			case Cmd::vlist_pop: vlist_pop(vm);break;
			case Cmd::combine: combine_results(vm);break;
			case Cmd::del: vm.del_value();break;
			case Cmd::swap: vm.swap_value();break;
			case Cmd::swap_1: vm.swap_value(load_int1());break;
			case Cmd::get_var_1: getVar(vm,load_int1());break;
			case Cmd::get_var_2: getVar(vm,load_int2());break;
			case Cmd::deref: deref(vm,vm.pop_value());break;
			case Cmd::deref_1: deref(vm,block.consts[load_int1()]);break;
			case Cmd::deref_2: deref(vm,block.consts[load_int2()]);break;
			case Cmd::call: vm.call_function_raw(vm.pop_value(),Value());break;
			case Cmd::call_1: vm.call_function_raw(pickVar(vm, load_int1()),Value());break;
			case Cmd::call_2: vm.call_function_raw(pickVar(vm, load_int2()),Value());break;
			case Cmd::mcall: mcall_fn(vm,vm.pop_value());break;
			case Cmd::mcall_1: mcall_fn(vm,block.consts[load_int1()]);break;
			case Cmd::mcall_2: mcall_fn(vm,block.consts[load_int2()]);break;
			case Cmd::exec_block: exec_block(vm);break;
			case Cmd::push_scope: vm.push_scope(Value());break;
			case Cmd::pop_scope: vm.pop_scope();break;
			case Cmd::push_scope_object: vm.push_scope(vm.pop_value());break;
			case Cmd::scope_to_object: vm.push_value(vm.scope_to_object());break;
			case Cmd::raise:do_raise(vm);break;
			case Cmd::set_var_1: set_var(vm,load_int1());break;
			case Cmd::set_var_2: set_var(vm,load_int2());break;
			case Cmd::pop_var_1: set_var(vm,load_int1());vm.del_value();break;
			case Cmd::pop_var_2: set_var(vm,load_int2());vm.del_value();break;
			case Cmd::op_add: bin_op(vm,op_add);break;
			case Cmd::op_sub: bin_op(vm,op_sub);break;
			case Cmd::op_mult: bin_op(vm,op_mult);break;
			case Cmd::op_div: bin_op(vm,op_div);break;
			case Cmd::op_mod: bin_op(vm,op_mod);break;
			case Cmd::op_cmp_eq: op_cmp(vm,[](int x){return x == 0;});break;
			case Cmd::op_cmp_less: op_cmp(vm,[](int x){return x < 0;});break;
			case Cmd::op_cmp_greater: op_cmp(vm,[](int x){return x > 0;});break;
			case Cmd::op_cmp_less_eq: op_cmp(vm,[](int x){return x <= 0;});break;
			case Cmd::op_cmp_greater_eq: op_cmp(vm,[](int x){return x >= 0;});break;
			case Cmd::op_cmp_not_eq: op_cmp(vm,[](int x){return x != 0;});break;
			case Cmd::op_cmp_eq_1: op_cmp_const(vm,load_int1());break;
			case Cmd::op_cmp_eq_2: op_cmp_const(vm,load_int2());break;
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
			case Cmd::push_false: vm.push_value(false);break;
			case Cmd::push_true: vm.push_value(true);break;
			case Cmd::push_null: vm.push_value(nullptr);break;
			case Cmd::push_zero_int: vm.push_value(0);break;
			case Cmd::push_undefined: vm.push_value(json::undefined);break;
			case Cmd::op_unary_minus: unar_op(vm, op_unar_minus);break;
			case Cmd::op_mkrange: bin_op(vm, op_mkrange);break;
			case Cmd::push_array_1: do_push_array(vm, load_int1());break;
			case Cmd::push_array_2: do_push_array(vm, load_int2());break;
			case Cmd::push_array_4: do_push_array(vm, load_int4());break;
			case Cmd::is_def_1: do_isdef(vm, load_int1());break;
			case Cmd::is_def_2: do_isdef(vm, load_int2());break;
			case Cmd::op_add_const_1: bin_op_const(vm, load_int1(), op_add);break;
			case Cmd::op_add_const_2: bin_op_const(vm, load_int2(), op_add);break;
			case Cmd::op_add_const_4: bin_op_const(vm, load_int4(), op_add);break;
			case Cmd::op_add_const_8: bin_op_const(vm, load_int8(), op_add);break;
			case Cmd::op_negadd_const_1: unar_op(vm,op_unar_minus);bin_op_const(vm, load_int1(), op_add);break;
			case Cmd::op_negadd_const_2: unar_op(vm,op_unar_minus);bin_op_const(vm, load_int2(), op_add);break;
			case Cmd::op_negadd_const_4: unar_op(vm,op_unar_minus);bin_op_const(vm, load_int4(), op_add);break;
			case Cmd::op_negadd_const_8: unar_op(vm,op_unar_minus);bin_op_const(vm, load_int8(), op_add);break;
			case Cmd::op_mult_const_1: bin_op_const(vm, load_int1(), op_mult);break;
			case Cmd::op_mult_const_2: bin_op_const(vm, load_int2(), op_mult);break;
			case Cmd::op_mult_const_4: bin_op_const(vm, load_int4(), op_mult);break;
			case Cmd::op_mult_const_8: bin_op_const(vm, load_int8(), op_mult);break;
			case Cmd::op_checkbound: bin_op(vm, op_checkbound);break;

			default: invalid_instruction(vm,cmd);
		}
		return true;
	} catch (...) {
		vm.raise(std::current_exception());
		return true;
	}
}


bool BlockExecution::exception(VirtualMachine &vm, std::exception_ptr e) {
	return false;
}

std::optional<CodeLocation> BlockExecution::getCodeLocation() const {
	//ip points to next instruction, which can be also next line, so decrease ip by one
	auto pos = ip;
	if (pos) pos--;
	auto iter = std::lower_bound(block.lines.begin(), block.lines.end(), std::pair{std::size_t(pos),std::size_t(-1)},std::greater());
	std::size_t l;
	if (iter == block.lines.end()) {
		l = 0;
	} else {
		l = iter->second;
	}

	return std::optional<CodeLocation>({block.location.file, block.location.line+l});
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

Value BlockExecution::pickVar(VirtualMachine &vm, std::intptr_t idx) {
	Value name = block.consts[idx];
	Value out;
	if (!vm.get_var(name.getString(), out)) {
		variable_not_found(vm, name.getString());
		return Value();
	} else {
		return out;
	}

}

json::Value BlockExecution::deref(VirtualMachine &vm, Value src, Value idx) {
	switch (idx.type()) {
	case json::number: return src[idx.getUInt()];
	case json::array: return newIndexMap(src, idx);
	case json::string: {
		if (src.type() == json::object) {
			Value r = src[idx.getString()];
			if (r.defined()) return r;
			Value clsItem = src[""];
			if (clsItem.type() != json::object) {
				if (!vm.get_var(strTypeClasses[src.type()], clsItem)) return json::undefined;
			}
			return clsItem[idx.getString()];
		} else {
			Value clsItem;
			if (!vm.get_var(strTypeClasses[src.type()], clsItem)) return json::undefined;
			return clsItem[idx.getString()];
		}
	}
	default: throw InvalidDereference(idx);
	}
}

void BlockExecution::deref(VirtualMachine &vm, Value idx) {
	try {
		Value z = vm.pop_value();
		vm.push_value(deref(vm, z, idx));
	} catch (...) {
		vm.raise(std::current_exception());
	}
}

void BlockExecution::bin_op_const(VirtualMachine &vm, std::int64_t val, Value (*fn)(const Value &a, const Value &b)) {
	Value z = vm.pop_value();
	vm.push_value(fn(z,val));
}

void BlockExecution::op_cmp_const(VirtualMachine &vm, int idx) {
	Value z = vm.top_value();
	Value v = block.consts[idx];
	if (v == z) {
		vm.del_value();
		vm.push_value(true);
	} else {
		vm.push_value(false);
	}
}

Value BlockExecution::op_mkrange(const Value &a, const Value &b) {
	auto begin = a.getInt();
	auto end = b.getInt();
	return newRange(begin, end);
}

Value BlockExecution::op_checkbound(const Value &a, const Value &b) {
	auto idx = b.getInt();
	return (idx >= 0 && idx < static_cast<std::intptr_t>(a.size()));
}

void BlockExecution::mcall_fn(VirtualMachine &vm, Value method) {
	Value obj = vm.pop_value();
	Value m = obj[method.getString()];
	vm.call_function_raw(m,obj);
}

void BlockExecution::vlist_pop(VirtualMachine &vm) {
	ValueList p = vm.top_params();
	vm.del_value();
	auto sz = p.size();
	if (sz==0) throw InvalidDereference("dereference of empty result");
	auto vl = ValueListValue::create(sz-1);
	for (std::size_t i = 1; i < sz;i++) {
		vl->push_back(p[i].getHandle());
	}
	vm.push_value(json::PValue::staticCast(vl));
	vm.push_value(p[0]);
}

void BlockExecution::combine_results(VirtualMachine &vm) {
	ValueList p2 = vm.top_params();
	vm.del_value();
	ValueList p1 = vm.top_params();
	vm.del_value();
	auto vl = ValueListValue::create(p2.size()+p1.size());
	for (Value x: p2) {
		vl->push_back(x.getHandle());
	}
	for (Value x: p1) {
		vl->push_back(x.getHandle());
	}
	vm.push_value(json::PValue::staticCast(vl));
}
/*
Value BlockExecution::do_deref(const Value where, const Value &what) {
	switch (what.type()) {
		case json::number: return  where[what.getUInt()];break;
		case json::string:
			if (where.type() != json::object) {
				auto tname = strTypeClasses[where.type()];
				if (vm.get)

			}

			return  where[what.getString()];break;
		default: throw InvalidDereference(what);
	}

}
*/

void BlockExecution::call_fn(VirtualMachine &vm) {
	Value fn = vm.pop_value();
	vm.call_function_raw(fn,Value());
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


void BlockExecution::set_var(VirtualMachine &vm, std::intptr_t cindex) {
	Value trg = block.consts[cindex];
	if (trg.type() == json::array) {
		auto args = vm.top_params();
		int idx = 0;
		for (Value x: trg) {
			if (x.hasValue()) {
				auto name = x.getString();
				if (!vm.set_var(name, args[idx])) {
					variable_already_assigned(vm, name);
					return;
				}
			}
			idx++;
		}
	} else {
		Value v = vm.top_value();
		auto name = block.consts[cindex].getString();
		if (!vm.set_var(name, v)) {
			variable_already_assigned(vm, name);
		}
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
		case json::number:
			if (b.isContainer())
				    return newDynMap(b, [a](const Value &x){return op_add(a,x);});
			if ((a.flags() & (json::numberInteger| json::numberUnsignedInteger))
					&& (b.flags() & (json::numberInteger| json::numberUnsignedInteger))) {
				return a.getIntLong()+b.getIntLong();
			} else {
				return a.getNumber()+b.getNumber();
			}
		case json::string: return json::String({a.toString(),b.toString()});
		case json::array:if (b.type() == json::number)
						    return newDynMap(a, [b](const Value &x){return op_add(x,b);});
							else return a.merge(b);
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
		case json::number:
			if (b.isContainer())
				    return newDynMap(b, [a](const Value &x){return op_sub(a,x);});
			if ((a.flags() & (json::numberInteger| json::numberUnsignedInteger))
					&& (b.flags() & (json::numberInteger| json::numberUnsignedInteger))) {
				return a.getIntLong()-b.getIntLong();
			} else {
				return a.getNumber()-b.getNumber();
			}
		case json::array: if (b.type() == json::number)
							return newDynMap(a, [b](const Value &x){return op_div(x,b);});
							else return nullptr;
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
		case json::object:
		case json::array:if (b.type() == json::number)
						    return newDynMap(a, [b](const Value &x){return op_mult(x,b);});
							else return nullptr;
		case json::number:
			if (b.isContainer())
			    return newDynMap(b, [a](const Value &x){return op_mult(x,a);});
			if ((a.flags() & (json::numberInteger| json::numberUnsignedInteger))
					&& (b.flags() & (json::numberInteger| json::numberUnsignedInteger))) {
				return a.getIntLong() * b.getIntLong();
			} else {
				return a.getNumber() * b.getNumber();
			}

		default: return nullptr;
	}
	return a;
}

Value BlockExecution::op_div(const Value &a, const Value &b) {
	if (!b.hasValue()) return b;
	switch (a.type()) {
		case json::undefined: return json::undefined;
		case json::boolean: return a.getBool() && !b.getBool();
		case json::object:
		case json::array: if (b.type() == json::number)
							return newDynMap(a, [b](const Value &x){return op_div(x,b);});
							else return nullptr;
		case json::number:
			if (b.isContainer())
				return newDynMap(b, [a,one = Value(1.0)](const Value &x){return op_mult(op_div(one,x), a);});
			return a.getNumber() / b.getNumber();
		default: return nullptr;
	}
	return a;
}

Value BlockExecution::op_mod(const Value &a, const Value &b) {
	if (!b.hasValue()) return b;
	switch (a.type()) {
		case json::undefined: return json::undefined;
		case json::boolean: return a.getBool() && !b.getBool();
		case json::number:
			if ((a.flags() & (json::numberInteger| json::numberUnsignedInteger))
					&& (b.flags() & (json::numberInteger| json::numberUnsignedInteger))) {
				return a.getIntLong() % b.getIntLong();
			} else {
				return std::fmod(a.getNumber(),b.getNumber());
			}

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


void BlockExecution::do_push_array(VirtualMachine &vm, std::intptr_t count) {
	vm.define_param_pack(count);
	auto pp = vm.top_params();
	Value arr (json::array, pp.begin(), pp.end(), [](const Value &a){return a;});
	vm.del_value();
	vm.push_value(arr);
}

void BlockExecution::expand_param_pack(VirtualMachine &vm, std::intptr_t amount) {
	if (amount > 0) {
		auto top = vm.top_value();
		if (top.isContainer()) {
			vm.del_value();
			for (Value x: top) {
				vm.push_value(x);
			}
			vm.define_param_pack(top.size()+amount-1);
			return;
		}
	}
	vm.define_param_pack(amount);
}


Value BlockExecution::op_unar_minus(const Value &a) {
	switch(a.type()) {
	case json::number: return -a.getNumber();
	case json::string: return a.reverse();
	case json::array: return a.reverse();
	default: return a;
	}
}

void BlockExecution::do_isdef(VirtualMachine &vm, std::intptr_t idx) {
	if (vm.isComileTime()) {
		vm.raise(std::make_exception_ptr(std::runtime_error("Compile time")));
	} else {
		Value dummy;
		vm.push_value(vm.get_var(block.consts[idx].getString(), dummy));
	}
}

}
