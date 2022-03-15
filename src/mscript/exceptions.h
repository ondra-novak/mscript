/*
 * exceptions.h
 *
 *  Created on: 16. 2. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_EXCEPTIONS_H_
#define SRC_MSCRIPT_EXCEPTIONS_H_

#include <stdexcept>
#include "value.h"
#include "codelocation.h"

namespace mscript {

class VMException: public std::exception {
public:

	 virtual const char*what() const noexcept override ;


	 virtual void what(std::string &out) const = 0;
	 virtual Value asValue() const;

protected:
	mutable std::string what_msg;
};


class CustomVMException: public VMException {
public:
	CustomVMException(const Value &value);
protected:
	Value value;
	virtual void what(std::string &out) const override;
	virtual Value asValue() const override;


};


class ArgumentIsNotFunction: public VMException {
public:
	ArgumentIsNotFunction(const Value &idx):idx(idx) {}
	virtual void what(std::string &out) const override;
protected:
	Value idx;
};

class ArgumentIsNotBlock: public VMException {
public:
	ArgumentIsNotBlock(const Value &idx):idx(idx) {}
	virtual void what(std::string &out) const override;
protected:
	Value idx;
};

class MaxExecutionTimeReached: public VMException {
public:
	virtual void what(std::string &out) const override;
};

enum class LimitType {
	calcStack,
	taskStack,
	scopeStack,
};

extern json::NamedEnum<LimitType> strLimitType;

class ExecutionLimitReached: public VMException {
public:
	ExecutionLimitReached(LimitType t);
	virtual void what(std::string &out) const override;
protected:
	LimitType t;

};


class CompileError: public VMException {
public:
	CompileError(const std::string &text, const CodeLocation &loc);

	const CodeLocation& getLoc() const {
		return loc;
	}

	const std::string& getText() const {
		return text;
	}

protected:
	virtual void what(std::string &out) const override;
	std::string text;
	CodeLocation loc;
};

class BuildError: public VMException {
public:
	BuildError(const std::string &text);
protected:
	virtual void what(std::string &out) const override;
	std::string text;
};

}




#endif /* SRC_MSCRIPT_EXCEPTIONS_H_ */
