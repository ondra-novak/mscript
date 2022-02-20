/*
 * vm.h
 *
 *  Created on: 15. 2. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_VM_H_
#define SRC_MSCRIPT_VM_H_
#include <memory>
#include <imtjson/object.h>
#include <imtjson/value.h>
#include <shared/refcnt.h>

#include "param_pack.h"
#include "value.h"

namespace mscript {



class VirtualMachine;
class VMException;


struct CodeLocation {
	std::string file;
	std::size_t line;
};


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


	Value convertToObject();
	Value operator[](const std::string_view &name) const;
	bool set(const std::string_view &name, const Value &v);

	auto begin() const {return items.begin();}
	auto end() const {return items.end();}
	Value getBase() const {return base;}

protected:
	Value base;
	Value parentLink;
	std::map<std::string, Value, std::less<> > items;

};


class VirtualMachine {
public:

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
	void del_value();
	void dup_value();
	void swap_value();
	Value pop_value();
	Value top_value() const;
	///retrieve value from stack, index is counted from top
	Value get_value(std::size_t idx) const;
	ParamPack top_params() const;
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
	std::optional<CodeLocation> getExceptionCodeLocation() const;

protected:

	TaskStack taskStack;
	CalcStack calcStack;
	ScopeStack scopeStack;
	Value globalScope;
	std::size_t paramPack = 0;
	std::exception_ptr exp = nullptr;
	std::optional<CodeLocation> exp_location;



};







}



#endif /* SRC_MSCRIPT_VM_H_ */
