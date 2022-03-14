/*
 * value.cpp
 *
 *  Created on: 27. 2. 2022
 *      Author: ondra
 */

#include "block.h"
#include "function.h"
#include "value.h"

namespace mscript {

json::NamedEnum<json::ValueType> strTypeClasses ( {
		{json::undefined, "Undefined"},
		{json::null,"Null"},
		{json::boolean,"Boolean"},
		{json::number,"Number"},
		{json::string,"String"},
		{json::array,"Array"},
		{json::object,"Object"},
});

std::string_view getTypeClass(const Value &val) {
	if (isNativeType(val)) {
		if (isFunction(val)) return "Function";
		else if (isBlock(val)) return "Block";
		else return "Native";
	} else {
		return strTypeClasses[val.type()];
	}
}

}
