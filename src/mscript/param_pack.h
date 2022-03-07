/*
 * param_pack.h
 *
 *  Created on: 15. 2. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_PARAM_PACK_H_
#define SRC_MSCRIPT_PARAM_PACK_H_

#include <imtjson/arrayValue.h>
#include "value.h"
namespace mscript {


static constexpr json::ValueTypeFlags paramPackValue = 65536;

class ListIterator;


///Allows to work with a value as ValueList. ValueList must be constructred by ValueListValue::create,
///Other values are mapped as single value list. ValueListValue is mapped as array.
class ValueList: private Value {
public:
	ValueList(const Value &x):Value(x),islist(x.type() == json::array  && (x.flags() & paramPackValue))  {}
	Value operator[](std::size_t idx) const {
		Value x;
		if (islist) {
			x = Value::operator[](idx);
			if (!x.hasValue()) x = nullptr;
		} else {
			x = idx?Value(nullptr):Value(*this);
		}
		return x;
	}
	std::size_t size() const {
		if (islist) return Value::size();else return 1;
	}
	bool empty() const {
		if (islist) return Value::empty(); else return false;
	}


	ListIterator begin() const;
	ListIterator end() const ;
	using Value::isCopyOf;

	friend class ListIterator;


	Value toValue() const;
protected:
	using Value::v;
	bool islist;
};


class ListIterator {
public:
	using UInt = json::UInt;
	using Int = json::Int;
	ValueList v;
	UInt index;

	ListIterator(ValueList v,UInt index):v(v),index(index) {}
	Value operator *() const {return v[index];}
	ListIterator &operator++() {++index;return *this;}
	ListIterator operator++(int) {++index;return ListIterator(v,index-1);}
	ListIterator &operator--() {--index;return *this;}
	ListIterator operator--(int) {--index;return ListIterator(v,index+1);}

	bool operator==(const ListIterator &other) const {
		return (other.atEnd() && atEnd())
				|| (index == other.index && v.isCopyOf(other.v));
	}
	bool operator!=(const ListIterator &other) const {
		return !operator==(other);
	}
	///Returns whether iterator points to the end
	bool atEnd() const {
		return index >= v.size();
	}
	bool atBegin() const {
		return index == 0;
	}
	ListIterator &operator+=(int p) {index+=p;return *this;}
	ListIterator &operator-=(int p) {index-=p;return *this;}
	ListIterator operator+(int p) const {return ListIterator(v,index+p);}
	ListIterator operator-(int p) const {return ListIterator(v,index-p);}
	Int operator-(ListIterator &other) const {return index - other.index;}

	typedef std::random_access_iterator_tag iterator_category;
    typedef Value        value_type;
    typedef Value *        pointer;
    typedef Value &        reference;
    typedef Int  difference_type;

};


///Implementation of value list
class ValueListValue: public json::ArrayValue {
public:
	using json::ArrayValue::ArrayValue;

	static json::RefCntPtr<ValueListValue> create(std::size_t capacity) {
		AllocInfo req(capacity);
		return new(req) ValueListValue(req);
	}

	virtual json::ValueTypeFlags flags() const override { return paramPackValue; }


};

inline ListIterator ValueList::begin() const {return ListIterator(*this, 0);}
inline ListIterator ValueList::end() const {return ListIterator(*this, size());}

inline Value ValueList::toValue() const {
	if (islist) {
		return Value(json::array, begin(), end(), [&](Value x){
			return x;
		});
	}else {
		return *this;
	}

}

}


#endif /* SRC_MSCRIPT_PARAM_PACK_H_ */

