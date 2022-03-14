/*
 * scope.cpp
 *
 *  Created on: 14. 3. 2022
 *      Author: ondra
 */



#include "function.h"
#include "scope.h"

namespace mscript {



static const std::size_t hashSizes[]={29, 47, 101, 199, 263, 521, 839, 1279, 2207, 3571, 9349, 19937, 60649};

static std::size_t getNextHashSize(std::size_t cur) {
	if (cur == 0) return hashSizes[0];
	for (auto iter = std::begin(hashSizes); iter != std::end(hashSizes);++iter) {
		if (*iter > cur) return *iter;
	}
	return cur*3/2;
}


void Scope::init(Value base){
	this->base = base;
	items.clear();
	items.resize(getNextHashSize(0));
	count = 0;
	rehash_treshold = items.size()*2/3;
}


Value Scope::convertToObject() const {
	json::Object obj(base);
	for (const auto &x: items) {
		if (x.has_value()) {
			obj.set(x->name.getString(), x->value);
		}
	}
	if (isFunction(base)) {
		return repackFunction(base, obj);
	} else {
		return obj;
	}
}


bool Scope::get(const std::string_view &name, Value &out) const {
	auto pos = findLocation(name);
	const auto &x = items[pos];
	if (x.has_value()) {
		out = x->value;
		return true;
	} else {
		out = base[name];
		return out.getKey() == name;
	}
}


bool Scope::set(const std::string_view &name, const Value &v) {
	return set(Value(name),v);
}


bool Scope::set(const Value &name, const Value &v) {
	auto pos = findLocation(name.getString());
	auto &x = items[pos];
	if (x.has_value()) return false;
	x = Variable{name, v};
	count++;
	if (count > rehash_treshold) {
		std::vector<VarItem> old;
		old.resize(getNextHashSize(items.size()));
		std::swap(old, items);
		rehash_treshold = items.size()*2/3;
		for (auto &v :  old) {
			if (v.has_value()) {
				pos = findLocation(v->name.getString());
				items[pos] = std::move(v);
			}
		}
	}
	return true;
}

std::size_t Scope::findLocation(const std::string_view &name) const {
	std::size_t sz = items.size();
	std::hash<std::string_view> h;
	auto hash = h(name);
	std::size_t pos = hash%sz;
	while (items[pos].has_value() && items[pos]->name.getString() != name) {
		pos = (pos + 1) % sz;
	}
	return pos;
}


std::vector<Scope::VarItem>::const_iterator Scope::find(const std::string_view &key) const {
	auto loc = findLocation(key);
	const auto &x = items[loc];
	if (x.has_value()) return items.begin()+loc;
	else return items.end();
}
}
