/*
 * procarr.h
 *
 *  Created on: 11. 3. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_PROCARR_H_
#define SRC_MSCRIPT_PROCARR_H_
#include "value.h"


namespace mscript {

class ProcArray {
public:
	ProcArray(Value fn);

	Value fn;
};

Value packProcArray(const Value &fn, std::size_t sz);
bool isProcArray(const Value &val);
const ProcArray &getProcArray(const Value &v);


}


#endif /* SRC_MSCRIPT_PROCARR_H_ */
