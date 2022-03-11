/*
 * procarr.cpp
 *
 *  Created on: 11. 3. 2022
 *      Author: ondra
 */

#include "value.h"
#include "range.h"
#include "procarr.h"

namespace mscript {

ProcArray::ProcArray(Value fn):fn(fn) {
}

Value packProcArray(const Value &fn, std::size_t sz) {
	return json::makeValue(ProcArray(fn), newRange(1, sz));
}

bool isProcArray(const Value &val) {
	return isNativeType(val, typeid(ProcArray));
}

const ProcArray &getProcArray(const Value &v) {
	const auto &r =json::cast<ProcArray>(v);
	return r;
}

}
