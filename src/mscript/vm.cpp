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

Value Scope::convertToObject() const {
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
	exp = nullptr;

}

bool VirtualMachine::run() {
	std::size_t task_sz = taskStack.size();
	if (!task_sz) return false;
	const auto &task = taskStack.back();
	if (timeStop.has_value()) {
		auto now = std::chrono::system_clock::now();
		if (now > *timeStop) {
			throw MaxExecutionTimeReached();
		}
	}
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
	auto sz = scopeStack.size();
	if (!sz) {
		//if base is not defined, we can bind globalScope to current scope without need to create link
		if (!base.defined()) scopeStack.emplace_back(globalScope);
		else scopeStack.emplace_back(base, Value({globalScope,nullptr}));
	} else if (sz >= cfg.maxScopeStack) {
		throw ExecutionLimitReached(LimitType::scopeStack);
	} else {
		Value parentLink = scopeStack.back().create_parent_link();
		scopeStack.emplace_back(base,parentLink);
	}
}

void VirtualMachine::push_task(std::unique_ptr<AbstractTask>&&task) {
	auto sz = taskStack.size();
	if (sz >= cfg.maxTaskStack) {
		throw ExecutionLimitReached(LimitType::taskStack);
	}
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
	auto paramPack = std::min(calcStack.size(),arguments);
	if (paramPack != 1) {
		auto newsz = calcStack.size()-paramPack;
		auto pv = ValueListValue::create(paramPack);
		for (auto i = newsz; i < calcStack.size(); i++) {
			pv->push_back(calcStack[i].getHandle());
		}
		calcStack.resize(newsz);
		push_value(json::PValue::staticCast(pv));
	}
}

ValueList VirtualMachine::top_params() const {
	return ValueList(calcStack.empty()?Value():calcStack.back());
}

void VirtualMachine::raise(std::exception_ptr e) {
	exp_location.clear();
	std::size_t p = taskStack.size();
	while (p) {
		--p;
		if (taskStack[p]->exception(*this, e)) {
			if (taskStack.size() > p+1) taskStack.resize(p+1);
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
	if (iter == items.end()) return base[name];
	else return iter->second;
}

bool Scope::set(const std::string_view &name, const Value &v) {
	auto res = items.emplace(std::string(name), v);
	return res.second;
}

Value VirtualMachine::top_value() const {
	return calcStack.empty()?Value():ValueList(calcStack.back()).toValue();
}

Value VirtualMachine::pop_value() {
	Value out = top_value();
	del_value();
	return out;
}

void VirtualMachine::del_value() {
	if (!calcStack.empty()) calcStack.pop_back();
}

void VirtualMachine::push_value(const Value &val) {
	if (calcStack.size()>=cfg.maxCalcStack) throw ExecutionLimitReached(LimitType::calcStack);
	calcStack.push_back(val);
}

Value VirtualMachine::get_this() {
	if (scopeStack.empty()) return json::undefined;
	const auto &scope = scopeStack.back();
	return scope.get_parent_link()[0];
}

bool VirtualMachine::restore_state(const VMState &st) {
	if (st.scopes > scopeStack.size() || st.values > calcStack.size()) {
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
	res.values = calcStack.size();
	return res;
}

void VirtualMachine::dup_value() {
	push_value(top_value());
}

void VirtualMachine::dup_value(std::size_t idx) {
	if (idx >= calcStack.size()) {
		throw std::runtime_error("dup_value argument out of range");
	}
	push_value(calcStack[calcStack.size()-idx-1]);
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

void VirtualMachine::swap_value(std::size_t idx) {
	if (idx >= calcStack.size()) {
		throw std::runtime_error("swap_value argument out of range");
	}
	std::swap(calcStack.back(), calcStack[calcStack.size()-idx-1]);
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
	ValueList z( pop_value());
	push_value(z.toValue());
}

bool VirtualMachine::call_function_raw(Value fnval) {
	if (!isFunction(fnval)) {
		throw ArgumentIsNotFunction(fnval);
	} else {
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

std::exception_ptr VirtualMachine::get_exception() const {
	return exp;
}

void VirtualMachine::setTimeStop(std::chrono::system_clock::time_point timeStop) {
	this->timeStop = timeStop;
}

void VirtualMachine::clearTimeStop() {
	timeStop.reset();
}

Value VirtualMachine::exec() {
	do {} while (run());
	auto e = get_exception();
	if (e != nullptr) std::rethrow_exception(e);
	else return pop_value();
}

Value VirtualMachine::exec(std::unique_ptr<AbstractTask>&& t) {
	push_scope(Value());
	push_task(std::move(t));
	Value v = exec();
	pop_scope();
	return v;
}

VirtualMachine::VirtualMachine(const Config &cfg):cfg(cfg) {
}

VirtualMachine::VirtualMachine() {
}


}

