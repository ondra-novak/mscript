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

}




#endif /* SRC_MSCRIPT_EXCEPTIONS_H_ */
