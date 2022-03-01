/*
 * value.cpp
 *
 *  Created on: 27. 2. 2022
 *      Author: ondra
 */

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

}
