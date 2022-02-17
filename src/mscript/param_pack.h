/*
 * param_pack.h
 *
 *  Created on: 15. 2. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_PARAM_PACK_H_
#define SRC_MSCRIPT_PARAM_PACK_H_

#include "value.h"
namespace mscript {




class ParamPack {
public:
	ParamPack(const Value *start, std::size_t count):start(start),count(count),null(nullptr) {}
	std::size_t size() const {return count;}
	bool empty() const {return count == 0;}
	const Value *begin() const {return start;}
	const Value *end() const {return start+count;}
	const Value &operator[](std::size_t idx) {return idx<count?start[idx]:null;}
protected:
	const Value *start;
	std::size_t count;
	Value null;

};

}



#endif /* SRC_MSCRIPT_PARAM_PACK_H_ */
