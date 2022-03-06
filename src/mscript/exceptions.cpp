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

void ArgumentIsNotFunction::what(std::string &out) const {
	out.append("Argument is not a function: '");
	idx.serialize([&](char c){out.push_back(c);});
	out.append("'");
}

void ArgumentIsNotBlock::what(std::string &out) const {
	out.append("Argument is not a block: '");
	idx.serialize([&](char c){out.push_back(c);});
	out.append("'");


}

void MaxExecutionTimeReached::what(std::string &out) const {
	out = "Max execution time reached.";
}

json::NamedEnum<LimitType> strLimitType({
	{LimitType::calcStack, "calc.stack"},
	{LimitType::scopeStack, "recursion count - max scope count"},
	{LimitType::taskStack, "recursion count - max task count"}
});

ExecutionLimitReached::ExecutionLimitReached(LimitType t):t(t) {

}

void ExecutionLimitReached::what(std::string &out) const {
	out.append("ExecutionLimitReached: ");
	out.append(strLimitType[t]);
}

CompileError::CompileError(const std::string &text, const CodeLocation &loc):text(text),loc(loc) {
}

void CompileError::what(std::string &out) const {
	out.append(loc.file);
	out.push_back(':');
	out.append(std::to_string(loc.line));
	out.append(": error: ");
	out.append(text);
}

void BuildError::what(std::string &out) const {
	out = text;
}

BuildError::BuildError(const std::string &text):text(text) {
}

}
