/*
 * function.h
 *
 *  Created on: 15. 2. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_FUNCTION_H_
#define SRC_MSCRIPT_FUNCTION_H_

#include "value.h"
#include "vm.h"

namespace mscript {


class AbstractFunction {
public:
	virtual ~AbstractFunction() {}

	///Calls the function
	/** Function can run immediately, or can create task
	 *
	 *  If function runs immediately, it picks arguments from virtual machine and pushes return value.
	 *  Then it returns nullptr for AbstractTask
	 *
	 *  @param vm virtual machine where execute the function
	 *  @return if function need to be run as task, it can create task and return it. Otherwise,
	 *  it can return null, if the result is immediately available
	 *
	 *  @param vm virtual machine
	 *  @param object object, if the function is part of an object (optional)
	 *  @param closure closure context (optional)
	 *
	 */
	virtual std::unique_ptr<AbstractTask> call(VirtualMachine &vm, Value object, Value closure) const = 0;

};


static inline Value packToValue(std::shared_ptr<AbstractFunction> &&fn, Value content) {
	return json::makeValue(std::move(fn), content);
}

static inline bool isFunction(const Value &val) {
	return isNativeType(val, typeid(std::shared_ptr<AbstractFunction>));
}

static inline Value repackFunction(const Value &fnObj, const Value &newClosure) {
	const auto &r =json::cast<std::shared_ptr<AbstractFunction> >(fnObj);
	return packToValue(std::shared_ptr<AbstractFunction>(r), newClosure);

}

static inline const AbstractFunction &getFunction(const Value &v) {
	const auto &r =json::cast<std::shared_ptr<AbstractFunction> >(v);
	return *r;
}

template<typename Fn, typename = decltype(std::declval<Fn>()(std::declval<VirtualMachine &>(), std::declval<Value>(), std::declval<Value>()))>
static inline Value defineFunction(Fn &&fn) {
	class FnClass: public AbstractFunction {
	public:
		virtual std::unique_ptr<AbstractTask> call(VirtualMachine &vm, Value object, Value closure) const override {
			return fn(vm, object, closure);
		}
		FnClass(Fn &&fn):fn(std::forward<Fn>(fn)) {}
	protected:
		Fn fn;
	};
	auto ptr = std::make_shared<FnClass>(std::forward<Fn>(fn));
	return packToValue(std::shared_ptr<AbstractFunction>(std::move(ptr)), {"@FN","native"});
}

template<typename Fn, typename = decltype(std::declval<Fn>()(std::declval<ValueList>()))>
static inline Value defineSimpleFn(Fn &&fn) {
	return defineFunction([fn = std::move(fn)](VirtualMachine &vm, Value, Value){
		auto params = vm.top_params();
		Value ret = fn(params);
		vm.del_value();
		vm.push_value(ret);
		return nullptr;
	});
}

template<typename Fn, typename = decltype(std::declval<Fn>()(std::declval<Value>(),std::declval<ValueList>()))>
static inline Value defineSimpleMethod(Fn &&fn) {
	return defineFunction([fn = std::move(fn)](VirtualMachine &vm, Value obj, Value){
		auto params = vm.top_params();
		Value ret = fn(obj, params);
		vm.del_value();
		vm.push_value(ret);
		return nullptr;
	});
}

}



#endif /* SRC_MSCRIPT_FUNCTION_H_ */
