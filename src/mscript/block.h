/*



 * block.h
 *
 *  Created on: 15. 2. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_BLOCK_H_
#define SRC_MSCRIPT_BLOCK_H_


#include <string>
#include "vm.h"
#include "exceptions.h"

namespace mscript {

enum class Cmd:std::uint8_t{
	noop,			///<no operation
	push_int_1,		///<push integer 1 byte
	push_int_2,		///<push integer 2 bytes
	push_int_4,		///<push integer 4 bytes
	push_int_8,		///<push integer 8 bytes
	push_double,	///<push double 8 bytes
	push_const_1,	///<push constant from array of consts 1 byte index
	push_const_2,   ///<push constant from array of consts 2 bytes index
	def_param_pack_1,  ///<define param pack 1 byte
	def_param_pack_2,  ///<define param pack 2 bytes
	expand_param_pack_1,  ///<similar to def, if last item is array, it is expanded - 1 byte
	expand_param_pack_2,  ///<similar to def, if last item is array, it is expanded - 2 bytes
	collapse_param_pack, ///<if param pack defined, it is collapsed to array or single value
	dup,			   ///<duplicate item on stack
	del,				///<del item on stack
	swap,				///<swap top most items
	get_var_1,			///<push value from variable, name of variable is in consts 1 byte index
	get_var_2,			///<push value from variable, name of variable is in consts 2 byte index
	deref,				///<pick value and index, derefence object or array
	deref_1,			///<dereference value, pick index from consts 1 byte
	deref_2,			///<dereference value, pick index from consts 2 byte
	deref_fn_1,			///<dereference of function of the value
	deref_fn_2,			///<dereference of function of the value
	call_fn_1,			///<call function, requires <params>, <function>- argument contains count of parameters
	call_fn_2,			///<call function, requires <params>, <function>- argument contains count of parameters
	exec_block,			///<execute block, requires <block> -> returns result
	push_scope,			///<create empty scope
	pop_scope,			///<destroy toplevel scope
	push_scope_object,	///<create scope with object as base requires <object> -> it is consumed
	scope_to_object,	///<makes object from scope and pushes it to stack
	raise,				///<raise exception -> requires <exception>

	push_true,			///<push true value
	push_false,			///<push false value
	push_null,			///<push null
	push_undefined,		///<push undefined
	push_this,			///<push this value
	push_array_1,		///<push array (argument count items)
	push_array_2,		///<push array (argument count items)
	push_array_4,		///<push array (argument count items)

	/// set variables from param pack
	set_var_1,		    ///<pick name of variable from consts (1 byte), set variable
	set_var_2,		    ///<pick name of variable from consts (2 byte), set variable
	pop_var_1,		    ///<pick name of variable from consts (1 byte), set variable
	pop_var_2,		    ///<pick name of variable from consts (2 byte), set variable

	is_def_1,			///<pick name of varuable, put on stack result (true if defined)
	is_def_2,			///<pick name of varuable, put on stack result (true if defined)

	/// operations

	op_add,
	op_sub,
	op_mult,
	op_div,
	op_cmp_eq,
	op_cmp_less,
	op_cmp_greater,
	op_cmp_less_eq,
	op_cmp_greater_eq,
	op_cmp_not_eq,
	op_bool_and,
	op_bool_or,
	op_bool_not,
	op_power,
	op_unary_minus,
	op_mod,

	op_add_const_1,
	op_add_const_2,
	op_add_const_4,
	op_add_const_8,
	op_negadd_const_1,
	op_negadd_const_2,
	op_negadd_const_4,
	op_negadd_const_8,
	op_mult_const_1,
	op_mult_const_2,
	op_mult_const_4,
	op_mult_const_8,

	/// jumps

	jump_1,			///<relative jump 1 byte
	jump_2,			///<relative jump 2 bytes
	jump_true_1,	///consumes bool and jumps if true
	jump_true_2,	///consumes bool and jumps if true
	jump_false_1,	///consumes bool and jumps if false
	jump_false_2,	///consumes bool and jumps if false
	exit_block,		///exit current block

};


extern json::NamedEnum<Cmd> strCmd;


struct Block {
public:
	///constants - pushed from code to stack
	std::vector<Value> consts;
	///
	std::vector<std::uint8_t> code;

	CodeLocation location;

	enum class DisEvent {
		code,
		begin_fn,
		end_fn,
		begin_block,
		end_block,
		location,
	};

	template<typename Fn> void disassemble(Fn &&fn) const;
};

static inline Value packToValue(Block &&block) {
	return json::makeValue(std::move(block), json::Object{{"@BLOCK",{block.location.file, block.location.line}}});
}

static inline bool isBlock(const Value &v) {
	return isNativeType(v, typeid(Block));
}

static inline const Block &getBlockFromValue(const Value &v) {
	return json::cast<Block>(v);
}


class BlockExecution: public AbstractTask {
public:
	BlockExecution(Value block);
	BlockExecution(const BlockExecution &other);

	virtual bool init(VirtualMachine &vm);
	virtual bool run(VirtualMachine &vm);
	virtual bool exception(VirtualMachine &vm, std::exception_ptr e);
	virtual std::optional<CodeLocation> getCodeLocation() const;

protected:
	Value block_value;
	const Block &block;
	std::size_t ir = 0;
	std::size_t ip = 0;


	std::intptr_t load_int1();
	std::intptr_t load_int2();
	std::intptr_t load_int4();
	std::int64_t load_int8();
	double load_double();

	void getVar(VirtualMachine &vm, std::intptr_t idx);
	void deref(VirtualMachine &vm, Value idx);
	void deref_fn(VirtualMachine &vm, Value idx);
	void call_fn(VirtualMachine &vm, std::intptr_t param_pack_size);
	void call_method(VirtualMachine &vm, std::intptr_t param_pack_size);
	void exec_block(VirtualMachine &vm);
	void do_raise(VirtualMachine &vm);
	void set_var_parampack(VirtualMachine &vm, std::intptr_t cindex);
	void set_var_arr_parampack(VirtualMachine &vm, std::intptr_t cindex);
	void set_var(VirtualMachine &vm, std::intptr_t cindex);

	void bin_op(VirtualMachine &vm, Value (*fn)(const Value &a, const Value &b));
	void unar_op(VirtualMachine &vm, Value (*fn)(const Value &a));
	void op_cmp(VirtualMachine &vm, bool (*fn)(int z));
	void bin_op_const(VirtualMachine &vm, std::int64_t val, Value (*fn)(const Value &a, const Value &b));

	void expand_param_pack(VirtualMachine &, std::intptr_t amount);

	void invalid_instruction(VirtualMachine &vm, Cmd cmd);
	void variable_not_found(VirtualMachine &vm, std::string_view name);
	void variable_already_assigned(VirtualMachine &vm, std::string_view name);
	void invalid_dereference(VirtualMachine &vm, Value idx);
	void argument_is_not_function(VirtualMachine &vm, Value v);
	void argument_is_not_block(VirtualMachine &vm, Value v);
	void do_push_array(VirtualMachine &vm, std::intptr_t count);

	void do_isdef(VirtualMachine &vm, std::intptr_t idx);

	static Value op_add(const Value &a, const Value &b);
	static Value op_sub(const Value &a, const Value &b);
	static Value op_mult(const Value &a, const Value &b);
	static Value op_mod(const Value &a, const Value &b);
	static Value op_div(const Value &a, const Value &b);
	static Value op_and(const Value &a, const Value &b);
	static Value op_or(const Value &a, const Value &b);
	static Value op_power(const Value &a, const Value &b);
	static Value op_not(const Value &a);
	static Value op_unar_minus(const Value &a);


	static Value do_deref(const Value where, const Value &what);


};

class InvalidInstruction: public VMException {
public:
	InvalidInstruction(Cmd cmd):cmd(cmd) {}
	virtual void what(std::string &out) const override;
protected:
	Cmd cmd;
};

class VariableNotFound: public VMException {
public:
	VariableNotFound(const std::string_view &name):name(name) {}
	virtual void what(std::string &out) const override;
protected:
	std::string name;
};

class VariableAlreadyAssigned: public VMException {
public:
	VariableAlreadyAssigned(const std::string_view &name):name(name) {}
	virtual void what(std::string &out) const override;
protected:
	std::string name;
};

class InvalidDereference: public VMException {
public:
	InvalidDereference(const Value &idx):idx(idx) {}
	virtual void what(std::string &out) const override;
protected:
	Value idx;
};


}


#endif /* SRC_MSCRIPT_BLOCK_H_ */
