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
#include <mscript/arrbld.h>
#include "vm.h"
#include "block.h"
#include "function.h"
#include "vm_rt.h"

using mscript::VirtualMachine;

namespace mscript {


class MapTask: public AbstractTask {
public:
		MapTask(Value object):src(object),idx(0) {}
		virtual bool init(VirtualMachine &vm) override;
		virtual bool run(VirtualMachine &vm) override;
protected:
		Value src;
		Value fn;
		std::size_t idx;


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
	}},
	{"Array",json::Object {
		{"push_back",defineSimpleMethod([](Value obj, ValueList params){return arrayPushBack(obj, params[0]);})},
		{"pop_back",defineSimpleMethod([](Value obj, ValueList params){return arrayPopBack(obj);})},
		{"back",defineSimpleMethod([](Value obj, ValueList params){return obj.empty()?Value():obj[obj.size()-1];})},
		{"size",defineSimpleMethod([](Value obj, ValueList params){
			return obj.size();
		})},
		{"reverse",defineSimpleMethod([](Value obj, ValueList params){return obj.reverse();})},
	}}

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
