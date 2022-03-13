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


void Scope::init(Value base){
	this->base = base;
	items.clear();
}


Value Scope::convertToObject() const {
	json::Object obj(base);
	for (const auto &x: items) {
		obj.set(x.name, x.value);
	}
	if (isFunction(base)) {
		return repackFunction(base, obj);
	} else {
		return obj;
	}

}

void VirtualMachine::setGlobalScope(Value globalScope) {
	this->globalScope = globalScope;
}

void VirtualMachine::reset() {
	scopeStack.clear();
	run_mode = RunMode::run_reset;
}

bool VirtualMachine::run() {
	switch (run_mode) {
	case RunMode::run_fast:return run_fast();
	case RunMode::run_exception: return run_exception();
	case RunMode::run_add_task: return run_add_task();
	case RunMode::run_fast_wtimer: return run_fast_wtimer();
	case RunMode::run_reset: return run_reset();
	}
	return false;
}

bool VirtualMachine::run_reset() {
	taskStack.clear();
	calcStack.clear();
	exp = nullptr;
	return run_add_task();
}
void VirtualMachine::prepare_all_tasks() {
	while (!newTasks.empty()) {
		std::swap(tmpTasks, newTasks);
		for (auto &t : tmpTasks) {
			if (t->init(*this)) taskStack.push_back(std::move(t));
		}
		tmpTasks.clear();
	}
}

bool VirtualMachine::run_add_task() {
	prepare_all_tasks();

	if (taskStack.empty()) {
		run_mode = RunMode::run_add_task;
		return false;
	}
	if (timeStop.has_value()) {
		run_mode = RunMode::run_fast_wtimer;
	} else {
		run_mode = RunMode::run_fast;
	}
	curTask = taskStack.back().get();
	return run();
}

bool VirtualMachine::run_fast_wtimer() {
	if (timeStop.has_value()) {
		auto now = std::chrono::system_clock::now();
		if (now > *timeStop) {
			throw MaxExecutionTimeReached();
		}
	} else {
		run_mode = RunMode::run_fast;
	}
	return run_fast();
}

bool VirtualMachine::run_fast() {
	if (!curTask->run(*this)) {
		taskStack.pop_back();
		if (taskStack.empty()) {
			run_mode = RunMode::run_reset; //code exited, next action would be reset
			return false;
		} else {
			curTask = taskStack.back().get();
		}
	}
	return true;
}

void VirtualMachine::push_scope(const Value base) {
	auto sz = scopeStack.size();
	if (!sz) {
		scopeStack.emplace_back();
		scopeStack.back().init(globalScope);
		//if base is not defined, we can bind globalScope to current scope without need to create link
		if (base.defined()) {
			scopeStack.emplace_back();
			scopeStack.back().init(base);
		}
	} else if (sz >= cfg.maxScopeStack) {
		throw ExecutionLimitReached(LimitType::scopeStack);
	} else {
		if (tmpScopes.empty()) {
			scopeStack.emplace_back();
		} else{
			scopeStack.push_back(std::move(tmpScopes.back()));
			tmpScopes.pop_back();
		}
		scopeStack.back().init(base);
	}
}

void VirtualMachine::push_task(std::unique_ptr<AbstractTask>&&task) {
	auto sz = taskStack.size()+newTasks.size();
	if (sz >= cfg.maxTaskStack) {
		throw ExecutionLimitReached(LimitType::taskStack);
	}
	newTasks.push_back(std::move(task));
	if (run_mode != RunMode::run_reset) {
		run_mode = RunMode::run_add_task;
	}
}

bool VirtualMachine::get_var(const std::string_view &name, Value &value) {
	if (scopeStack.empty()) {
		value = globalScope[name];
		return value.defined();
	}
	auto r = scopeStack.rbegin();
	while (r != scopeStack.rend()) {
		if (r->get(name, value)) {
			return value.defined();
		}
		++r;
	}
	return false;
}

bool VirtualMachine::set_var(const std::string_view &name, const Value &value) {
	if (scopeStack.empty()) return false;
	return scopeStack.back().set(name, value);
}

bool VirtualMachine::set_var(const Value &name, const Value &value) {
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
	if (e != nullptr) exp = e;
	run_mode = RunMode::run_exception; //flag VM, we need to run exception handler
}
bool VirtualMachine::run_exception() {
	exp_location.clear();
	newTasks.clear(); //in case of exception, clear all new tasks
	std::size_t p = taskStack.size();
	while (p) {
		--p;
		if (taskStack[p]->exception(*this, exp)) {
			if (taskStack.size() > p+1) taskStack.resize(p+1);
			exp = nullptr;
			return run_add_task(); //<to sync tasks and flags
		}
		auto loc = taskStack[p]->getCodeLocation();
		if (loc.has_value()) {
			exp_location.push_back(*loc);
		}
	}
	reset();
	return false;
}

bool VirtualMachine::pop_scope() {
	if (scopeStack.empty()) return false;
	tmpScopes.push_back(std::move(scopeStack.back()));
	scopeStack.pop_back();
	return true;
}

bool Scope::get(const std::string_view &name, Value &out) const {
	auto iter = std::find_if(items.begin(), items.end(), [&](const Variable &v){return v.name == name;});
	if (iter == items.end()) {
		out = base[name];
		return out.getKey() == name;
	}
	else {
		out = iter->value;
		return true;
	}
}


bool Scope::set(const std::string_view &name, const Value &v) {
	return set(Value(name),v);
}
bool Scope::set(const Value &name, const Value &v) {
	auto n = name.getString();
	if (std::find_if(items.begin(), items.end(), [&](const Variable &v){return v.name == n;}) != items.end()) return false;
	items.push_back(Variable(name, v));
	return true;
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

bool VirtualMachine::restore_state(const VMState &st) {
	if (st.scopes > scopeStack.size() || st.values > calcStack.size()) {
		return false;
	}
	scopeStack.resize(st.scopes, Scope());
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
	if (1 >= calcStack.size()) {
		throw std::runtime_error("swap_value argument out of range");
	}
	std::swap(calcStack.back(), calcStack[calcStack.size()-2]);
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

bool VirtualMachine::call_function_raw(Value fnval, Value object) {
	if (!isFunction(fnval)) {
		throw ArgumentIsNotFunction(fnval);
	} else {
		const AbstractFunction &fnobj = getFunction(fnval);
		auto sz = newTasks.size();
		fnobj.call(*this,object,fnval);
		return sz != newTasks.size();
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

void VirtualMachine::push_values(const Value &arra) {
	if (arra.isContainer()) for (Value x: arra) push_value(x);
	else push_value(arra);
}



void VirtualMachine::begin_list() {
	static Value emptylist (json::PValue::staticCast(ValueListValue::create(0)));
	push_value(emptylist);
}

void VirtualMachine::finish_list() {
	auto sz = calcStack.size();
	auto p = sz;
	while (p>0) {
		auto q = p-1;
		const Value &x = calcStack[q];
		bool pp = x.type() == json::array && (x.flags() & paramPackValue);
		if (pp) break;
		p = q;
	}
	if (p == 0) {
		throw std::runtime_error("List is not started");
	}
	Value clv = calcStack[p-1];
	auto lv = ValueListValue::create(sz-p+clv.size());
	for (Value x: clv) {
		lv->push_back(x.getHandle());
	}
	for (auto i = p; i < sz; i++) {
		lv->push_back(calcStack[i].getHandle());
	}
	calcStack.resize(p-1);
	push_value(json::PValue::staticCast(lv));
}


void VirtualMachine::push_cb_task(std::unique_ptr<AbstractTask> &&t) {
	newTasks.insert(newTasks.begin(),std::move(t));
}
}

