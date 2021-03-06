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
	begin_list, ///<start building param pack
	close_list, ///<finish building param pack
	expand_array, ///<expand array to stack (...)
	collapse_list_1, ///<converts value list to array, skipping first N items (argument)
	dup,			   ///<duplicate item on stack
	del,				///<del item on stack
	dup_1,			   ///<duplicate nth-item on stack
	vlist_pop,			///<separate last value in value list to separate value. If last value is not value list, does nothing
	combine,			///<combine two results into value list. Joins two value lists
	swap,				///<swap top most items
	swap_1,				///<swap top most items with nth-item on stack
	get_var_1,			///<push value from variable, name of variable is in consts 1 byte index
	get_var_2,			///<push value from variable, name of variable is in consts 2 byte index
	deref,				///<pick value and index, derefence object or array
	deref_1,			///<dereference value, pick index from consts 1 byte
	deref_2,			///<dereference value, pick index from consts 2 byte
	call,				///<call function, requires <arguments as list><function>
	call_1,				///<call function, requires <arguments as list><function>
	call_2,				///<call function, requires <arguments as list><function>
	mcall,				///<call method, requires <arguments as list><object><function>
	mcall_1,			///<call method, requires <arguments as list><object> - argument is fn-name
	mcall_2,			///<call method, requires <arguments as list><object> - argument is fn-name
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
	push_zero_int,		///<push integer zero
	push_array_1,		///<push array (argument count items)
	push_array_2,		///<push array (argument count items)
	push_array_4,		///<push array (argument count items)

	// set variables from param pack
	set_var_1,		    ///<pick name of variable from consts (1 byte), set variable
	set_var_2,		    ///<pick name of variable from consts (2 byte), set variable
	pop_var_1,		    ///<pick name of variable from consts (1 byte), set variable
	pop_var_2,		    ///<pick name of variable from consts (2 byte), set variable

	is_def,
	is_def_1,			///<pick name of varuable, put on stack result (true if defined)
	is_def_2,			///<pick name of varuable, put on stack result (true if defined)

	// operations

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
	op_mkrange,
	op_cmp_eq_1,		//<compare with constant - replace value with true if equal, push false, if not
	op_cmp_eq_2,		//<compare with constant - replace value with true if equal, push false, if not
	op_checkbound,		//Binary op, requires <container><index>, replaces with true, when index is above or below range

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

	// jumps

	jump_1,			///<relative jump 1 byte
	jump_2,			///<relative jump 2 bytes
	jump_true_1,	///consumes bool and jumps if true
	jump_true_2,	///consumes bool and jumps if true
	jump_false_1,	///consumes bool and jumps if false
	jump_false_2,	///consumes bool and jumps if false
	exit_block,		///exit current block

	// loops


};


extern json::NamedEnum<Cmd> strCmd;


struct Block {
public:
	///constants - pushed from code to stack
	std::vector<Value> consts;
	///code
	std::vector<std::uint8_t> code;
	///Addresses map to lines - for debugging
	std::vector<std::pair<std::size_t,std::size_t>> lines; //{code, line, ordered backward}

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
	template<typename Fn> void disassemble_ip(std::size_t ip, Fn &&fn) const;
	template<typename Fn> void disassemble_iter(std::vector<std::uint8_t>::const_iterator &iter, const std::vector<std::uint8_t>::const_iterator &end, Fn &&fn) const;
};

static inline Value packToValue(Block &&block) {
	return json::makeValue(std::move(block), {"@BLOCK",block.location.file, block.location.line});
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

	const Block &getBlock() const {return block;}
	std::size_t getIP() const {return ip;}

protected:
	Value block_value;
	const Block &block;
	///Instruction pointer
	std::size_t ip = 0;


	std::intptr_t load_int1();
	std::intptr_t load_int2();
	std::intptr_t load_int4();
	std::int64_t load_int8();
	double load_double();

	void getVar(VirtualMachine &vm, std::intptr_t idx);
	Value pickVar(VirtualMachine &vm, std::intptr_t idx);
	void deref(VirtualMachine &vm, Value idx);
	json::Value deref(VirtualMachine &vm, Value src, Value idx);
	void call_fn(VirtualMachine &vm);
	void mcall_fn(VirtualMachine &vm, Value method);
	void exec_block(VirtualMachine &vm);
	void do_raise(VirtualMachine &vm);
	void set_var(VirtualMachine &vm, std::intptr_t cindex);

	void bin_op(VirtualMachine &vm, Value (*fn)(const Value &a, const Value &b));
	void unar_op(VirtualMachine &vm, Value (*fn)(const Value &a));
	void op_cmp(VirtualMachine &vm, bool (*fn)(int z));
	void op_cmp_const(VirtualMachine &vm, int idx);
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

	void vlist_pop(VirtualMachine &vm);
	void combine_results(VirtualMachine &vm);

	static Value op_add(const Value &a, const Value &b);
	static Value op_sub(const Value &a, const Value &b);
	static Value op_mult(const Value &a, const Value &b);
	static Value op_mod(const Value &a, const Value &b);
	static Value op_div(const Value &a, const Value &b);
	static Value op_and(const Value &a, const Value &b);
	static Value op_or(const Value &a, const Value &b);
	static Value op_power(const Value &a, const Value &b);
	static Value op_mkrange(const Value &a, const Value &b);
	static Value op_not(const Value &a);
	static Value op_unar_minus(const Value &a);
	static Value op_checkbound(const Value &a, const Value &b);



//	static Value do_deref(const Value where, const Value &what);


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
