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

	json::PValue push_back(json::PValue) const;
	json::PValue push_front(json::PValue) const;
	json::PValue collapse() const;
	static json::PValue create(json::PValue left, json::PValue right);
	static json::PValue oneItemArray(json::PValue nd);

protected:
	json::PValue left;
	json::PValue right;
	std::size_t lsize;
	std::size_t rsize;

};


class ArrTruncate: public json::AbstractArrayValue {
public:
	ArrTruncate(json::PValue src, std::size_t offset, std::size_t newsz);
	virtual json::PValue itemAtIndex(std::size_t index) const override;
	virtual std::size_t size() const override;
	std::size_t getNewSz() const {return newsz;}
	const json::PValue& getSrc() const {return src;}
	std::size_t getOffset() const {return offs;}

protected:
	json::PValue src;
	std::size_t offs;
	std::size_t newsz;
};

Value arrayPushBack (Value arr, Value item);
Value arrayPopBack (Value arr);
Value arrayPushFront (Value arr, Value item);
Value arrayPopFront (Value arr);



}

#endif /* SRC_MSCRIPT_ARRBLD_H_ */
