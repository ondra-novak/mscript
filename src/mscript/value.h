/*
 * value.h
 *
 *  Created on: 15. 2. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_VALUE_H_
#define SRC_MSCRIPT_VALUE_H_

#include <imtjson/wrap.h>

namespace mscript {

using Value = json::Value;


static inline bool isNativeType(const Value &val, const std::type_info &type) {
	const json::_details::IWrap *wp = dynamic_cast<const json::_details::IWrap *>(static_cast<const json::IValue *>(val.getHandle()));
	if (wp == nullptr) return false;
	return wp->cast(type) != nullptr;
}

}



#endif /* SRC_MSCRIPT_VALUE_H_ */
