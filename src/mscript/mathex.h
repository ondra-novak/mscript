/*
 * mathex.h
 *
 *  Created on: 13. 3. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_MATHEX_H_
#define SRC_MSCRIPT_MATHEX_H_
#include "param_pack.h"
#include "vm.h"


namespace mscript {

void mathIntegral(VirtualMachine &vm, ValueList params);
void mathRoot(VirtualMachine &vm, ValueList params);
}


#endif /* SRC_MSCRIPT_MATHEX_H_ */
