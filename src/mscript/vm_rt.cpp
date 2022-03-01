/*
 * vm_rt.cpp
 *
 *  Created on: 20. 2. 2022
 *      Author: ondra
 */

#include <imtjson/array.h>
#include <cmath>
#include <random>
#include <imtjson/object.h>
#include "block.h"
#include "function.h"
#include "vm_rt.h"

namespace mscript {

class AbstractMapTask: public AbstractTask {
public:
	bool post_init(mscript::VirtualMachine &vm) {
		if (!container.isContainer()) {
			vm.raise(std::make_exception_ptr(std::runtime_error("The first argument of the function must be a container")));
			return false;
		}
		if (!isFunction(fn)) {
			vm.raise(std::make_exception_ptr(std::runtime_error("The second argument of the function must be a function")));
			return false;
		}
		if (container.empty()) {
			vm.pop_value();
			vm.push_value(container);
			return false;
		}
		if (init_block.defined()) {
			if (!isBlock(init_block)) {
				vm.raise(std::make_exception_ptr(std::runtime_error("The last argument of the function must be a block")));
				return false;
			}
		} else {
			init_block = nullptr;
		}
		max = container.size();
		return true;
	}
	virtual bool init(mscript::VirtualMachine &vm) override {
		auto args = vm.top_params();
		container = args[0];
		fn = args[1];
		init_block = args[2];
		vm.del_value();
		return post_init(vm);
	}
	virtual void add_result(Value result) = 0;
	virtual Value get_result() const = 0;
	virtual void call_fn(mscript::VirtualMachine &vm) = 0;

	virtual bool run(mscript::VirtualMachine &vm) override {
		if (init_block.defined()) {
			vm.push_scope(Value());
			if (!init_block.isNull()) {
				vm.push_task(std::make_unique<BlockExecution>(init_block));
			}
			init_block = Value();
			return true;
		}

		Value prevScope = vm.scope_to_object();
		vm.pop_scope();

		if (index) {
			add_result(vm.pop_value());
			if (index >= max) {
				vm.push_value(get_result());
				return false;
			}
		} else {
			vm.del_value();
		}
		vm.push_scope(prevScope);
		call_fn(vm);
		++index;
		return true;
	}
protected:
	Value fn;
	Value init_block;
	Value container;
	Value prevScope;
	std::size_t index;
	std::size_t max;
	bool has_scope;
};

class MapTask: public AbstractMapTask {
public:
	virtual void add_result(Value result) override {r.push_back(result);}
	virtual Value get_result() const override {return r;}
	virtual void call_fn(mscript::VirtualMachine &vm) override {
		vm.call_function(fn, container[index], index, container);
	}
	json::Array r;

};


class ReduceTask: public AbstractMapTask {
public:
	virtual bool init(mscript::VirtualMachine &vm) override {
		auto args = vm.top_params();
		container = args[0];
		fn = args[1];
		sum = args[2];
		init_block = args[3];
		vm.del_value();
		return post_init(vm);
	}
	virtual void add_result(Value result) override {sum = result;}
	virtual Value get_result() const override {return sum;}
	virtual void call_fn(mscript::VirtualMachine &vm) override {
		vm.call_function(fn, sum, container[index], index, container);
	}
protected:
	Value sum;
};


Value getVirtualMachineRuntime() {

static Value rt = json::Object {
	{"Math",json::Object{
		{"E",std::exp(1.0)},
		{"LN10",std::log(10.0)},
		{"LN2",std::log(2.0)},
		{"LOG2E",std::log2(std::exp(1))},
		{"LOG10E",std::log10(std::exp(1))},
		{"PI",std::acos(0.0)*2},
		{"SQRT1_2",std::sqrt(0.5)},
		{"SQRT2",std::sqrt(2)},
		{"abs",defineSimpleFn([](ParamPack params){return std::abs(params[0].getNumber());})},
		{"acos",defineSimpleFn([](ParamPack params){return std::acos(params[0].getNumber());})},
		{"acosh",defineSimpleFn([](ParamPack params){return std::acosh(params[0].getNumber());})},
		{"asin",defineSimpleFn([](ParamPack params){return std::asin(params[0].getNumber());})},
		{"asinh",defineSimpleFn([](ParamPack params){return std::asinh(params[0].getNumber());})},
		{"atan",defineSimpleFn([](ParamPack params){return std::atan(params[0].getNumber());})},
		{"atanh",defineSimpleFn([](ParamPack params){return std::atanh(params[0].getNumber());})},
		{"atan2",defineSimpleFn([](ParamPack params){return std::atan2(params[0].getNumber(),params[1].getNumber());})},
		{"cbrt",defineSimpleFn([](ParamPack params){return std::cbrt(params[0].getNumber());})},
		{"ceil",defineSimpleFn([](ParamPack params){return std::ceil(params[0].getNumber());})},
		{"cos",defineSimpleFn([](ParamPack params){return std::cos(params[0].getNumber());})},
		{"cosh",defineSimpleFn([](ParamPack params){return std::cosh(params[0].getNumber());})},
		{"exp",defineSimpleFn([](ParamPack params){return std::exp(params[0].getNumber());})},
		{"expm1",defineSimpleFn([](ParamPack params){return std::expm1(params[0].getNumber());})},
		{"floor",defineSimpleFn([](ParamPack params){return std::floor(params[0].getNumber());})},
		{"fround",defineSimpleFn([](ParamPack params){return std::round(params[0].getNumber());})},
		{"hypot",defineSimpleFn([](ParamPack params){
			double v = 0;
			for (Value a: params) {auto x = a.getNumber(); v+=x*x;}
			return std::sqrt(v);
		})},
		{"log",defineSimpleFn([](ParamPack params){return std::log(params[0].getNumber());})},
		{"log1p",defineSimpleFn([](ParamPack params){return std::log1p(params[0].getNumber());})},
		{"log10",defineSimpleFn([](ParamPack params){return std::log10(params[0].getNumber());})},
		{"log2",defineSimpleFn([](ParamPack params){return std::log2(params[0].getNumber());})},
		{"max",defineSimpleFn([](ParamPack params){
			double v = params[0].getNumber();
			for (Value a: params) {auto x = a.getNumber(); v = x>v?x:v;}
			return v;})},
		{"min",defineSimpleFn([](ParamPack params){
			double v = params[0].getNumber();
			for (Value a: params) {auto x = a.getNumber(); v = x<v?x:v;}
			return v;})},
		{"pow",defineSimpleFn([](ParamPack params){return std::pow(params[0].getNumber(),params[1].getNumber());})},
		{"random",defineSimpleFn([](ParamPack params){
				std::random_device rnd;
				std::uniform_real_distribution<double> urd(0,1);
				return urd(rnd);})},
		{"round",defineSimpleFn([](ParamPack params){return std::round(params[0].getNumber());})},
		{"sign",defineSimpleFn([](ParamPack params){
			double n = params[0].getNumber();
			return n>0?1:n<0?-1:0;})},
		{"sin",defineSimpleFn([](ParamPack params){return std::sin(params[0].getNumber());})},
		{"sinh",defineSimpleFn([](ParamPack params){return std::sinh(params[0].getNumber());})},
		{"sqrt",defineSimpleFn([](ParamPack params){return std::sqrt(params[0].getNumber());})},
		{"tan",defineSimpleFn([](ParamPack params){return std::tan(params[0].getNumber());})},
		{"trunc",defineSimpleFn([](ParamPack params){return std::trunc(params[0].getNumber());})},
	}},
	{"map",defineFunction([](VirtualMachine &vm, Value){
		return std::make_unique<MapTask>();
	})},
	{"reduce",defineFunction([](VirtualMachine &vm, Value){
		return std::make_unique<ReduceTask>();
	})},
	{"filter",defineFunction([](VirtualMachine &vm, Value){
		return std::make_unique<ReduceTask>();
	})},

};
return rt;

}

}

