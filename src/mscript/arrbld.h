/*
 * arrbld.h
 *
 *  Created on: 11. 3. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_ARRBLD_H_
#define SRC_MSCRIPT_ARRBLD_H_
#include <imtjson/basicValues.h>
#include <mscript/value.h>



namespace mscript {

class ArrBldNode: public json::AbstractArrayValue {
public:
	ArrBldNode(json::PValue left, json::PValue right);
	virtual json::PValue itemAtIndex(std::size_t index) const override;
	virtual std::size_t size() const override;

	json::RefCntPtr<ArrBldNode> push_back(json::PValue) const;

protected:
	json::PValue left;
	json::PValue right;
	std::size_t lsize;
	std::size_t rsize;
};


class ArrTruncate: public json::AbstractArrayValue {
public:
	ArrTruncate(json::PValue src, std::size_t newsz);
	virtual json::PValue itemAtIndex(std::size_t index) const override;
	virtual std::size_t size() const override;
	std::size_t getNewSz() const {return newsz;}
	const json::PValue& getSrc() const {return src;}

protected:
	json::PValue src;
	std::size_t newsz;
};

Value arrayPushBack (Value arr, Value item);
Value arrayPopBack (Value arr);



}

#endif /* SRC_MSCRIPT_ARRBLD_H_ */
