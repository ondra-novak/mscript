/*
 * dynmap.h
 *
 *  Created on: 6. 3. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_DYNMAP_H_
#define SRC_MSCRIPT_DYNMAP_H_

namespace mscript {

template<typename Fn>
class DynMap: public json::AbstractArrayValue {
public:
	DynMap(json::Value v, Fn &&fn):v(v),fn(std::forward<Fn>(fn)) {}
	virtual json::ValueType type() const override { return v.type();}
	virtual json::RefCntPtr<const IValue> itemAtIndex(std::size_t index) const override {
		json::Value x= fn(v[index]);
		return json::Value(x.getKey(),x.getHandle()).getHandle();
	}
	virtual json::RefCntPtr<const json::IValue> member(const std::string_view &name) const override {
		json::Value x= fn(v[name]);
		return json::Value(name,x.getHandle()).getHandle();
	}
	virtual std::size_t size() const override {
		return v.size();
	}

protected:
	json::Value v;
	Fn fn;
};

template<typename Fn>
Value newDynMap(Value v, Fn &&fn) {
	return Value(new DynMap(v, std::forward<Fn>(fn)));
}


}
#endif /* SRC_MSCRIPT_DYNMAP_H_ */
