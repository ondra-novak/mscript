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

}




#endif /* SRC_MSCRIPT_EXCEPTIONS_H_ */
