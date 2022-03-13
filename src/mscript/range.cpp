/*
 * range.cpp
 *
 *  Created on: 6. 3. 2022
 *      Author: ondra
 */

#include <imtjson/value.h>
#include "range.h"

namespace mscript {

RangeValue::RangeValue(json::Int begin, json::Int end)
		:base(begin),
		 dir(begin>end?-1:1),
		 sz((begin>end?begin-end:end-begin)+1) {
}

std::size_t RangeValue::size() const {
	return sz;
}

bool RangeValue::equal(const json::IValue *other) const  {
	auto othr = dynamic_cast<const RangeValue *>(other->unproxy());
	if (othr) {
		return othr->base == base && othr->sz == sz && othr->dir == dir;
	} else {
		return AbstractArrayValue::equal(other);
	}

}


json::RefCntPtr<const json::IValue> RangeValue::itemAtIndex(std::size_t index) const {
	json::Value ret(json::Int(base + dir*index));
	return ret.getHandle();
}

json::Int RangeValue::getBegin() const {
	return base;
}

json::Int RangeValue::getEnd() const {
	return base + dir*sz;
}


}

json::Value mscript::newRange(json::Int begin, json::Int end) {
	return json::Value(new RangeValue(begin, end));
}
