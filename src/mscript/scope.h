/*
 * scope.h
 *
 *  Created on: 14. 3. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_SCOPE_H_
#define SRC_MSCRIPT_SCOPE_H_

#include <optional>
#include "value.h"

namespace mscript {


class Scope {
public:
	void init(Value base);
	Value getBase() const {return base;}

	Value convertToObject() const;

	bool set(const Value &name, const Value &v);
	bool set(const std::string_view &name, const Value &v);
	bool get(const std::string_view &name, Value &out) const;

	struct Variable {
		json::Value name;
		json::Value value;
	};


	using VarItem = std::optional<Variable>;

	auto begin() const {return items.begin();}
	auto end() const {return items.end();}
	std::vector<VarItem>::const_iterator find(const std::string_view &key) const;
protected:

	Value base;
	std::vector<VarItem> items;
	std::size_t count=0, rehash_treshold=0;

	std::size_t findLocation(const std::string_view &name) const;
};

}


#endif /* SRC_MSCRIPT_SCOPE_H_ */
