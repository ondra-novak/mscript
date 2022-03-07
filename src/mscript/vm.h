/*
 * vm.h
 *
 *  Created on: 15. 2. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_VM_H_
#define SRC_MSCRIPT_VM_H_
#include <memory>
#include <chrono>
#include <imtjson/object.h>
#include <imtjson/value.h>
#include <mscript/param_pack.h>
#include <shared/refcnt.h>

#include "value.h"
#include "codelocation.h"

namespace mscript {



class VirtualMachine;
class VMException;




class AbstractTask {
public:

	virtual ~AbstractTask() {}

	///Inicializes task
	/**
	 * Task should prepare virtual machine to run
	 * It should allocate scopes and pick arguments if needed.
	 * If arguments are picked, they should be discarded from the stack
	 */
	virtual bool init(VirtualMachine &vm) = 0;
	///Run task
	/**
	 * @param vm reference to virtual machine
	 * @retval true suspended - call run again to continue
	 * @retval false done - remove task from vm. Task should leave virtual machine in same state
	 * as after init, only return value should be on stack (can be as ParamPack).
	 *
	 * Return value can be defined as param pack
	 */
	virtual bool run(VirtualMachine &vm) =0;

	///Called when exception happen (can be raised in child task)
	/**
	 * Function must explore exception and decide whether to process exception or not.
	 *
	 * @param vm virtual machine
	 * @param e exception
	 * @retval false exception cannot be handled. In this case, virtual machine removes this
	 * task from the execution. (will not call done)
	 * @retval true exception can be handled. In this case, virtual machine continues execution
	 * of this task. The task should create copy of the exception in its inner state
	 *
	 * when true is returned, virtual machine expect, that function restores the machine to the known
	 * state. It can use save_state a restore_state
	 */
	virtual bool exception(VirtualMachine &vm, std::exception_ptr e) {return false;}

	///Retrieve location of code - optional
	virtual std::optional<CodeLocation> getCodeLocation() const {return {};}
};



class Scope {
public:
	Scope(Value base, Value parentLink);
	Scope(Value base);

	Value create_parent_link();
	const Value &get_parent_link() const {return parentLink;}


	Value convertToObject() const;
	Value operator[](const std::string_view &name) const;
	bool set(const std::string_view &name, const Value &v);

	auto begin() const {return items.begin();}
	auto end() const {return items.end();}
	Value getBase() const {return base;}
	auto find(const std::string &name) const {return items.find(name);}

protected:
	Value base;
	Value parentLink;
	std::map<std::string, Value, std::less<> > items;

};


class VirtualMachine {
public:

	struct Config {
		///max calc stack size
		unsigned int maxCalcStack = 1000;
		///max task size (max recursion)
		unsigned int maxTaskStack = 1000;
		///max scope stack (max scope recursion)
		unsigned int maxScopeStack = 1000;

	};

	VirtualMachine(const Config &cfg);
	VirtualMachine();

	using TaskStack = std::vector<std::unique_ptr<AbstractTask> >;
	using CalcStack = std::vector<Value>;
	using ScopeStack = std::vector<Scope>;

	struct VMState {
		std::size_t values,scopes;
		std::optional<std::vector<Value> >paramPack;
	};


	///Sets global scope
	/**
	 * You can define global variables and objects here
	 * @param globalScope global scope - JSON object with variables and values
	 */
	void setGlobalScope(Value globalScope);

	void reset();
	///run virtual machine for single step
	bool run();
	///Raise exception
	/** When exception is raised, tasks are explored from top to bottom to handle exception.
	 * If task can handle exception, it will continue to run, otherwise exception is thrown to
	 * next level.
	 *
	 * @param e exception to raise
	 *
	 * @note if exception is not handled, virtual machine is stopped, exception can be received through get_exception
	 */
	void raise(std::exception_ptr e);

	///Exec current code, return value. Execption is thrown
	Value exec();

	///Exec task, return value. Exception is thrown
	Value exec(std::unique_ptr<AbstractTask> &&);


	///Retrieves exception if code stopped because exception
	/**
	 * @retval pointer pointer to exception
	 * @retval nullptr virtual machine stopped normally
	 */
	std::exception_ptr get_exception() const;

	///Save virtual machine state
	/**
	 * The returned structure allows to restore state of vm when it need to recover from deep recursive
	 * structure. It cannot restore complete different state
	 */
	VMState save_state() const;

	///Restore virtual machine state
	/**
	 * Allows to restore state of vm when it need to recover from deep recursive
	 * structure. It cannot restore complete different state
	 */
	bool restore_state(const VMState &st);



	void push_value(const Value &push);
	void push_values(const Value &arra);
	void del_value();
	void dup_value();
	void swap_value();
	void dup_value(std::size_t ofs);
	void swap_value(std::size_t ofs);
	Value pop_value();
	Value top_value() const;
	///retrieve value from stack, index is counted from top
	Value get_value(std::size_t idx) const;
	ValueList top_params() const;
	///Defines param pack
	/** Param pack can exist only on top of stack
	 * Param pack can be empty.
	 *
	 * You can get param pack using top_params()
	 *
	 * param pack is poped as array. if anything is pushed after param pack, it converted to array
	 *
	 * Param pack with single value is not converted
	 *
	 * @note if current param pack is defined, it is collapsed
	 */
	void define_param_pack(std::size_t arguments);
	void collapse_param_pack();

	bool set_var(const std::string_view &name, const Value &value);
	bool get_var(const std::string_view &name, Value &value);
	///"this" is defined as scope object one level below current scope
	Value get_this();
	///"scope_base" is defined as base object of current scope
	Value get_scope_base() const;


	///Add new task to TaskStack. The task will continue to run on next run()
	/**
	 * @param new task
	 *
	 * Calls init on the task before it is pushed. If the init() returns false, task is not pushed
	 * (it is discarded)
	 */
	void push_task(std::unique_ptr<AbstractTask> &&);

	///push new scope
	/**
	 * @param base optional value contains base object for lambda function and objects. The base
	 * object becomes visible in new scope, but variables cannot be changed
	 */
	void push_scope(const Value base);

	bool pop_scope();

	Value scope_to_object();

	std::optional<CodeLocation> getCodeLocation() const;

	///When machine stops because exception, this returns code location of the exception
	std::vector<CodeLocation> getExceptionCodeLocation() const;

	///Passes arguments to stack and calls function
	/**
	 * @param fnval function value
	 * @param args arguments
	 * @retval false function executed synchronously - result is immediately available
	 * @retval true function executed asynchronously - result will be available on next cycle
	 */
	template<typename ... Args>
	bool call_function(Value fnval, Value object, const Args & ... args);

	///Calls function, while arguments are already ready on stack
	/**
	 * @param fnval function value
	 * @param argCnt count of arguments ready on stack
	 * @retval false function executed synchronously - result is immediately available
	 * @retval true function executed asynchronously - result will be available on next cycle
	 */
	bool call_function_raw(Value fnval, Value object);


	bool isComileTime() const {
		return comile_time;
	}

	void setComileTime(bool comileTime) {
		comile_time = comileTime;
	}


	///Specifies time point when execution stops
	void setTimeStop(std::chrono::system_clock::time_point timeStop);
	///Disables time stop
	void clearTimeStop();

	///Sets max execution time
	/**
	 * When execution time is reached, it must be reset otherwise no futher execution is possible.
	 * @param dur duration
	 */
	template<typename T,typename U>
	void setMaxExecutionTime(const std::chrono::duration<T,U> &dur) {
		setTimeStop(std::chrono::system_clock::now()+dur);
	}

	const CalcStack& getCalcStack() const {
		return calcStack;
	}

	const ScopeStack& getScopeStack() const {
		return scopeStack;
	}

	const TaskStack& getTaskStack() const {
		return taskStack;
	}

	void begin_list();
	void finish_list();

protected:

	Config cfg;
	TaskStack taskStack;
	CalcStack calcStack;
	ScopeStack scopeStack;
	Value globalScope;
	std::exception_ptr exp = nullptr;
	std::vector<CodeLocation> exp_location;
	std::optional<std::chrono::system_clock::time_point> timeStop;

	template<typename ... Args>
	void push_arguments(const Value &v, const Args & ... args) {
		push_value(v);
		push_arguments(args...);
	}

	void push_arguments() {}

	bool comile_time = false;


};

template<typename ... Args>
inline bool VirtualMachine::call_function(Value fnval, Value object, const Args &... args) {
	push_arguments(args...);
	define_param_pack(sizeof...(Args));
	return call_function_raw(fnval, object);
}






}


#endif /* SRC_MSCRIPT_VM_H_ */
