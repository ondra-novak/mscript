/*
 * vm.cpp
 *
 *  Created on: 15. 2. 2022
 *      Author: ondra
 */

#include <imtjson/object.h>
#include <mscript/exceptions.h>
#include <mscript/function.h>
#include "vm.h"

namespace mscript {

Scope::Scope(Value base, Value parentLink):base(base),parentLink(parentLink) {}

Scope::Scope(Value base):base(base) { }

Value Scope::create_parent_link() {
	return {convertToObject(), parentLink};
}

Value Scope::convertToObject() {
	json::Object obj(base);
	for (const auto &x: items) {
		obj.set(x.first, x.second);
	}
	return obj;
}

void VirtualMachine::setGlobalScope(Value globalScope) {
	this->globalScope = globalScope;
}

void VirtualMachine::reset() {
	taskStack.clear();
	calcStack.clear();
	scopeStack.clear();
	paramPack = 0;
	exp = nullptr;

}

bool VirtualMachine::run() {
	std::size_t task_sz = taskStack.size();
	if (!task_sz) return false;
	const auto &task = taskStack.back();
	if (!task->run(*this)) {
		//so task returned false, but added task or more tasks - we need to delete it from middle
		if (task_sz >= taskStack.size()) {
			taskStack.erase(taskStack.begin()+task_sz-1);
			//if no task added, remove last task
		} else if (task_sz == taskStack.size()) {
			taskStack.pop_back();
		}
		//if tasks are reduced, then no action is performed, last task was probably terminated by exception
	}
	return true;
}

void VirtualMachine::push_scope(const Value base) {
	if (scopeStack.empty()) {
		//if base is not defined, we can bind globalScope to current scope without need to create link
		if (!base.defined()) scopeStack.emplace_back(globalScope);
		else scopeStack.emplace_back(base, Value({globalScope,nullptr}));
	} else {
		Value parentLink = scopeStack.back().create_parent_link();
		scopeStack.emplace_back(base,parentLink);
	}
}

void VirtualMachine::push_task(std::unique_ptr<AbstractTask>&&task) {
	if (task->init(*this)) {
		taskStack.push_back(std::move(task));
	}
}

bool VirtualMachine::get_var(const std::string_view &name, Value &value) {
	if (scopeStack.empty()) {
		value = globalScope[name];
		return value.defined();
	}
	const auto &scope = scopeStack.back();
	value = scope[name];
	if (value.defined()) return true;
	Value link = scope.get_parent_link();
	while (link.hasValue()) {
		value = link[0][name];
		if (value.defined()) return true;
		link = link[1];
	}
	return false;
}

bool VirtualMachine::set_var(const std::string_view &name, const Value &value) {
	if (scopeStack.empty()) return false;
	return scopeStack.back().set(name, value);
}

void VirtualMachine::define_param_pack(std::size_t arguments) {
	collapse_param_pack();
	paramPack = std::min(calcStack.size(),arguments);
}

ParamPack VirtualMachine::top_params() const {
	return ParamPack(calcStack.data()+calcStack.size()-paramPack, paramPack);
}

void VirtualMachine::raise(std::exception_ptr e) {
	exp_location.clear();
	std::size_t p = taskStack.size();
	while (p) {
		--p;
		if (taskStack[p]->exception(*this, e)) {
			taskStack.resize(p+1);
			return;
		}
		auto loc = taskStack[p]->getCodeLocation();
		if (loc.has_value()) {
			exp_location.push_back(*loc);
		}
	}
	reset();
	exp = e;
}

bool VirtualMachine::pop_scope() {
	if (scopeStack.empty()) return false;
	scopeStack.pop_back();
	return true;
}

Value Scope::operator [](const std::string_view &name) const {
	auto iter = items.find(name);
	if (iter == items.end()) return json::undefined;
	else return iter->second;
}

bool Scope::set(const std::string_view &name, const Value &v) {
	auto res = items.emplace(std::string(name), v);
	return res.second;
}

Value VirtualMachine::top_value() const {
	switch (paramPack) {
		case 0: return json::array;
		case 1: return calcStack.empty()?Value():calcStack.back();
		default: {
			ParamPack pk = top_params();
			return Value(json::array, pk.begin(), pk.end(), [](const Value &v){return v;});
		}
	}
}

Value VirtualMachine::pop_value() {
	Value out = top_value();
	del_value();
	return out;
}

void VirtualMachine::del_value() {
	switch(paramPack) {
		case 0: break;
		case 1: if (!calcStack.empty()) calcStack.pop_back();
		default: for (std::size_t i = 0; i < paramPack; i++) {
			calcStack.pop_back();
		}
	}
	define_param_pack(1);
}

void VirtualMachine::push_value(const Value &val) {
	collapse_param_pack();
	calcStack.push_back(val);
	paramPack = 1;
}

Value VirtualMachine::get_this() {
	if (scopeStack.empty()) return json::undefined;
	const auto &scope = scopeStack.back();
	return scope.get_parent_link()[0];
}

bool VirtualMachine::restore_state(const VMState &st) {
	if (st.scopes > scopeStack.size() || st.values > scopeStack.size()) {
		return false;
	}
	scopeStack.resize(st.scopes, Scope(Value()));
	calcStack.resize(st.values);
	if (st.paramPack.has_value()) {
		for (Value x: *st.paramPack) push_value(x);
		define_param_pack(st.paramPack->size());
	} else {
		define_param_pack(1);
	}
	return true;
}

VirtualMachine::VMState VirtualMachine::save_state() const {
	VMState res;
	res.scopes = scopeStack.size();
	if (paramPack != 1) {
		auto top = top_params();
		res.paramPack.emplace(top.begin(), top.end());
		res.values = calcStack.size()-paramPack;
	} else {
		res.values = calcStack.size();
	}
	return res;
}

void VirtualMachine::dup_value() {
	if (paramPack != 1) {
		Value x = pop_value();
		push_value(x);
		push_value(x);
	} else {
		push_value(top_value());
	}
}

std::optional<CodeLocation> VirtualMachine::getCodeLocation() const {
	std::optional<CodeLocation> loc;
	auto iter = taskStack.rbegin();
	while (!loc.has_value() && iter != taskStack.rend()) {
		loc = (*iter)->getCodeLocation();
	}
	return loc;
}

void VirtualMachine::swap_value() {
	Value x = pop_value();
	Value y = pop_value();
	push_value(y);
	push_value(x);
}

Value VirtualMachine::scope_to_object() {
	if (scopeStack.empty()) return json::Value();
	return scopeStack.back().convertToObject();
}

Value VirtualMachine::get_value(std::size_t idx) const {
	auto sz = calcStack.size();
	if (idx>=sz) return Value();
	else return calcStack[sz - idx];
}

std::vector<CodeLocation> VirtualMachine::getExceptionCodeLocation() const {
	return exp_location;
}

void VirtualMachine::collapse_param_pack() {
	if (paramPack != 1) {
		push_value(pop_value());
	}
}

bool VirtualMachine::call_function_raw(Value fnval, std::size_t argCnt) {
	if (!isFunction(fnval)) {
		throw ArgumentIsNotFunction(fnval);
	} else {
		define_param_pack(argCnt);
		const AbstractFunction &fnobj = getFunction(fnval);
		auto task = fnobj.call(*this,fnval);
		if (task != nullptr) {
			push_task(std::move(task));
			return true;
		} else {
			return false;
		}
	}
}

}

