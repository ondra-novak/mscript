/*
 * exceptions.cpp
 *
 *  Created on: 16. 2. 2022
 *      Author: ondra
 */

#include "exceptions.h"
#include <imtjson/serializer.h>

namespace mscript {

const char* VMException::what() const noexcept {
	if (what_msg.empty()) {
		what(what_msg);
	}
	return what_msg.c_str();
}

Value VMException::asValue() const {
	std::string buff;
	what(buff);
	return Value(json::object,{
			json::Value("VMException", buff)
	});
}


CustomVMException::CustomVMException(const Value &value):value(value) {
}

void CustomVMException::what(std::string &out) const {
	out.append("Code exception: ");
	value.serialize([&](char c){
		out.push_back(c);
	});
}

Value CustomVMException::asValue() const {
	return value;
}

}


