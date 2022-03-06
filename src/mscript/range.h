/*
 * range.h
 *
 *  Created on: 6. 3. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_RANGE_H_
#define SRC_MSCRIPT_RANGE_H_
#include <imtjson/basicValues.h>


namespace mscript {


class RangeValue: public json::AbstractArrayValue {
public:

	RangeValue(json::Int begin, json::Int end);
	virtual json::RefCntPtr<const json::IValue> itemAtIndex(std::size_t index) const override;
	virtual std::size_t size() const override;
	virtual bool equal(const json::IValue *other) const override;
	json::Int getBegin() const;
	json::Int getEnd() const;
	double getMult() const;
protected:
	json::Int base, dir;
	json::UInt sz;
};

json::Value newRange(json::Int begin, json::Int end);

}


#endif /* SRC_MSCRIPT_RANGE_H_ */
