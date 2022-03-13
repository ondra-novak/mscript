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
#include <imtjson/operations.h>
#include <mscript/arrbld.h>
#include <mscript/procarr.h>
#include "vm.h"
#include "block.h"
#include "function.h"
#include "vm_rt.h"
#include "generator.h"
#include "mathex.h"

using mscript::VirtualMachine;

namespace mscript {


template<typename Fn>
static void asyncDeref(VirtualMachine &vm, Value obj, std::size_t idx, Fn &&cb) {
	if (isProcArray(obj)) {
		vm.call_function(getProcArray(obj).fn, Value(), idx)
			>> [fn = std::forward<Fn>(cb)](VirtualMachine &vm) mutable {
				fn(vm, vm.pop_value());
			};
	} else {
		cb(vm, obj[idx]);
	}
}

template<bool findIndex>
class ArrayFindTask: public AbstractTask {
public:
	ArrayFindTask(Value obj, Value fn)
			:obj(obj),fn(fn),lastRes(nullptr),idx(0),sz(obj.size()) {}

	virtual bool init(VirtualMachine &vm)override  {return true;}
	virtual bool run(VirtualMachine &vm) override {
		if (idx) {
			Value r = vm.pop_value();
			if  (r.getBool()) {
				if (findIndex) vm.push_value(idx-1);else vm.push_value(lastRes);
				return false;
			}
		}
		if (idx >= sz) {
			if (findIndex) vm.push_value(-1);else vm.push_value(nullptr);
			return false;
		}
		asyncDeref(vm, obj, idx, [this](VirtualMachine &vm, Value v){
			lastRes = v;
			vm.call_function(fn, Value(), v, idx, obj);
			++idx;
		});
		return true;
	}

protected:
	Value obj;
	Value fn;
	Value lastRes;
	std::size_t idx;
	std::size_t sz;
};

class ArrayMap: public AbstractTask {
public:
	ArrayMap(Value obj, Value fn)
			:obj(obj),fn(fn),result(json::array),idx(0) {}

	virtual bool init(VirtualMachine &vm)override  {
		asyncDeref(vm, obj, idx, [this](VirtualMachine &vm, Value v){
			vm.call_function(fn, Value(), v, idx, obj);
		});
		return true;

	}
	virtual bool run(VirtualMachine &vm) override {
		auto r = vm.top_params();
		vm.pop_value();
		for (Value x: r) result = arrayPushBack(result, x);
		idx++;
		if (idx == obj.size()) {
			vm.push_value(result.map([](Value x){return x;}));
			return false;
		}
		return init(vm);
	}

protected:
	Value obj;
	Value fn;
	Value result;
	std::size_t idx;
};


class ArraySort: public AbstractTask, public Generator {
public:
	ArraySort(Value arr, Value fn):arr(arr),fn(fn) {
	}

	~ArraySort() {
	}

	void generatingFunction() override {
		result = arr.sort([&](Value a, Value b){
			left = a;
			right = b;
			side = 0;
			suspend_generator();
			return res;
		});
	}

	virtual bool init(VirtualMachine &vm) override {
		resume_generator();
		if (done) {
			vm.push_value(result);
			return false;
		} else {
			vm.call_function(fn, Value(), left,right);
			return true;
		}
	}
	virtual bool run(VirtualMachine &vm) override {
		Value cmpRes =vm.pop_value();
		res = cmpRes.getInt();
		return init(vm);
	}


protected:
	Value arr;
	Value fn;
	Value left;
	Value right;
	int res = 0;
	Value result;

};

template<typename Fn>
static void arrayCopyAsync(VirtualMachine &vm, json::RefCntPtr<json::ArrayValue> cont, Value arr, Fn &&out) {
	vm.call_function(getProcArray(arr).fn, Value(), cont->size())
			>> [arr, cont, out = std::forward<Fn>(out)](VirtualMachine &vm) mutable {
		Value v = vm.pop_value();
		cont->push_back(v.getHandle());
		if (cont->size() == arr.size()) out(vm,Value(json::PValue::staticCast(cont)));
		else arrayCopyAsync(vm,cont,arr,std::forward<Fn>(out));
	};

}
static void arrayCopy(VirtualMachine &vm, json::Value arr) {
	if (isProcArray(arr)) {
		auto cont = json::ArrayValue::create(arr.size());
		arrayCopyAsync(vm, cont, arr, [](VirtualMachine &vm, Value v){vm.push_value(v);});
	} else if (dynamic_cast<const json::ArrayValue *>(arr.getHandle()->unproxy())) {
		vm.push_value(arr);
	} else {
		Value cp(json::array, arr.begin(), arr.end(), [](Value x){return x;});
		vm.push_value(cp);
	}
}

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
		{"INF",std::numeric_limits<double>::infinity()},
		{"EPSILON",std::numeric_limits<double>::epsilon()},
		{"abs",defineSimpleFn([](ValueList params){return std::abs(params[0].getNumber());})},
		{"acos",defineSimpleFn([](ValueList params){return std::acos(params[0].getNumber());})},
		{"acosh",defineSimpleFn([](ValueList params){return std::acosh(params[0].getNumber());})},
		{"asin",defineSimpleFn([](ValueList params){return std::asin(params[0].getNumber());})},
		{"asinh",defineSimpleFn([](ValueList params){return std::asinh(params[0].getNumber());})},
		{"atan",defineSimpleFn([](ValueList params){return std::atan(params[0].getNumber());})},
		{"atanh",defineSimpleFn([](ValueList params){return std::atanh(params[0].getNumber());})},
		{"atan2",defineSimpleFn([](ValueList params){return std::atan2(params[0].getNumber(),params[1].getNumber());})},
		{"cbrt",defineSimpleFn([](ValueList params){return std::cbrt(params[0].getNumber());})},
		{"ceil",defineSimpleFn([](ValueList params){return std::ceil(params[0].getNumber());})},
		{"cos",defineSimpleFn([](ValueList params){return std::cos(params[0].getNumber());})},
		{"cosh",defineSimpleFn([](ValueList params){return std::cosh(params[0].getNumber());})},
		{"exp",defineSimpleFn([](ValueList params){return std::exp(params[0].getNumber());})},
		{"expm1",defineSimpleFn([](ValueList params){return std::expm1(params[0].getNumber());})},
		{"floor",defineSimpleFn([](ValueList params){return std::floor(params[0].getNumber());})},
		{"fround",defineSimpleFn([](ValueList params){return std::round(params[0].getNumber());})},
		{"hypot",defineSimpleFn([](ValueList params){
			double v = 0;
			for (Value a: params) {auto x = a.getNumber(); v+=x*x;}
			return std::sqrt(v);
		})},
		{"log",defineSimpleFn([](ValueList params){return std::log(params[0].getNumber());})},
		{"log1p",defineSimpleFn([](ValueList params){return std::log1p(params[0].getNumber());})},
		{"log10",defineSimpleFn([](ValueList params){return std::log10(params[0].getNumber());})},
		{"log2",defineSimpleFn([](ValueList params){return std::log2(params[0].getNumber());})},
		{"max",defineSimpleFn([](ValueList params){
			double v = params[0].getNumber();
			for (Value a: params) {auto x = a.getNumber(); v = x>v?x:v;}
			return v;})},
		{"min",defineSimpleFn([](ValueList params){
			double v = params[0].getNumber();
			for (Value a: params) {auto x = a.getNumber(); v = x<v?x:v;}
			return v;})},
		{"pow",defineSimpleFn([](ValueList params){return std::pow(params[0].getNumber(),params[1].getNumber());})},
		{"random",defineSimpleFn([](ValueList params){
				std::random_device rnd;
				std::uniform_real_distribution<double> urd(0,1);
				return urd(rnd);})},
		{"round",defineSimpleFn([](ValueList params){return std::round(params[0].getNumber());})},
		{"sign",defineSimpleFn([](ValueList params){
			double n = params[0].getNumber();
			return n>0?1:n<0?-1:0;})},
		{"sin",defineSimpleFn([](ValueList params){return std::sin(params[0].getNumber());})},
		{"sinh",defineSimpleFn([](ValueList params){return std::sinh(params[0].getNumber());})},
		{"sqrt",defineSimpleFn([](ValueList params){return std::sqrt(params[0].getNumber());})},
		{"tan",defineSimpleFn([](ValueList params){return std::tan(params[0].getNumber());})},
		{"trunc",defineSimpleFn([](ValueList params){return std::trunc(params[0].getNumber());})},
		{"isfinite",defineSimpleFn([](ValueList params){return std::isfinite(params[0].getNumber());})},
		{"isNaN",defineSimpleFn([](ValueList params){return std::isnan(params[0].getNumber());})},
		{"erf",defineSimpleFn([](ValueList params){return std::erf(params[0].getNumber());})},
		{"erfc",defineSimpleFn([](ValueList params){return std::erfc(params[0].getNumber());})},
		{"tgamma",defineSimpleFn([](ValueList params){return std::tgamma(params[0].getNumber());})},
		{"lgamma",defineSimpleFn([](ValueList params){return std::lgamma(params[0].getNumber());})},
		{"expint",defineSimpleFn([](ValueList params){return std::expint(params[0].getNumber());})},
		{"beta",defineSimpleFn([](ValueList params){return std::beta(params[0].getNumber(),params[1].getNumber());})},
		{"lcm",defineSimpleFn([](ValueList params){return std::lcm(params[0].getInt(),params[1].getInt());})},
		{"integral",defineAsyncFunction(mathIntegral)},

	}},
	{"Array",json::Object {
		{"push_back",defineSimpleMethod([](Value obj, ValueList params){for (Value z: params) {obj = arrayPushBack(obj, z);} return obj;})},
		{"pop_back",defineSimpleMethod([](Value obj, ValueList params){return arrayPopBack(obj);})},
		{"push_front",defineSimpleMethod([](Value obj, ValueList params){for (Value z: params) {obj = arrayPushFront(obj, z);} return obj;})},
		{"pop_front",defineSimpleMethod([](Value obj, ValueList params){return arrayPopFront(obj);})},
		{"back",defineSimpleMethod([](Value obj, ValueList params){return obj.empty()?Value():obj[obj.size()-1];})},
		{"front",defineSimpleMethod([](Value obj, ValueList params){return obj.empty()?Value():obj[0];})},
		{"size",defineSimpleMethod([](Value obj, ValueList params){
			return obj.size();
		})},
		{"reverse",defineSimpleMethod([](Value obj, ValueList params){return obj.reverse();})},
		{"indexOf",defineSimpleMethod([](Value obj, ValueList params){return obj.indexOf(params[0], params[1].getUInt());})},
		{"find", defineAsyncMethod([](VirtualMachine &vm, Value obj, ValueList params){vm.push_task(std::make_unique<ArrayFindTask<false> >(obj,params[0]));})},
		{"findIndex", defineAsyncMethod([](VirtualMachine &vm, Value obj, ValueList params){vm.push_task(std::make_unique<ArrayFindTask<true> >(obj,params[0]));})},
		{"sort", defineAsyncMethod([](VirtualMachine &vm, Value obj, ValueList params){
			if (obj.size()<2) vm.push_value(obj);
			else vm.push_task(std::make_unique<ArraySort>(obj,params[0]));})},
		{"copy", defineAsyncMethod([](VirtualMachine &vm, Value obj, ValueList ){
			if (obj.empty()) vm.push_value(obj);
			else arrayCopy(vm, obj);
		})},
		{"map", defineAsyncMethod([](VirtualMachine &vm, Value obj, ValueList params){
			if (obj.empty()) vm.push_value(obj);
			else vm.push_task(std::make_unique<ArrayMap>(obj,params[0]));
		})}
	}},
	{"String",json::Object {

	}},
	{"__operator",json::Object{
		{"in", defineSimpleFn([](ValueList lst){
			auto str = lst[0].getString();
			Value obj =lst[1];
			return obj[str].defined();
		})}
	}},

	{"vtarray", defineSimpleFn([](ValueList params){
		Value fn = params[1];
		Value size = params[0];
		if (size.type() != json::number) throw std::runtime_error("vtarray - the second argument must be a number");
		if (!isFunction(fn)) throw std::runtime_error("vtarray - the second argument must be a function");
		return packProcArray(fn, size.getUInt());
	})},
	{"typeof",defineSimpleFn([](ValueList params){return strTypeClasses[params[0].type()];})},
	{"keyof",defineSimpleFn([](ValueList params){return params[0].getKey();})},
	{"strip_key",defineSimpleFn([](ValueList params){return params[0].stripKey();})},




};
return rt;

}

/*

inline bool MapTask::init(VirtualMachine &vm) {
	fn = vm.pop_value();

}

inline bool MapTask::run(VirtualMachine &vm) {
}
*/

}
