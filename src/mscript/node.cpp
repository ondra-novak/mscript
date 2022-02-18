#include <mscript/function.h>
#include "node.h"

namespace mscript {

BinaryOperation::BinaryOperation(PNode &&left, PNode &&right, Cmd instruction)
	:left(std::move(left)),right(std::move(right)),instruction(instruction) {}

void BinaryOperation::generateExpression(BlockBld &blk) const {
	left->generateExpression(blk);
	right->generateExpression(blk);
	blk.pushCmd(instruction);
}

UnaryOperation::UnaryOperation(PNode &&item, Cmd instruction)
:item(std::move(item)),instruction(instruction) {}


void UnaryOperation::generateExpression(BlockBld &blk) const {
	item->generateExpression(blk);
	blk.pushCmd(instruction);
}

Assignment::Assignment(PNode &&assignment, PNode &&expression)
:assignment(std::move(assignment)),expression(std::move(expression)) {}

void Assignment::generateExpression(BlockBld &blk) const {
	assignment->generateExpression(blk);
	assignment->generateAssign(blk);
}

Identifier::Identifier(Value name):name(name) {}

void Identifier::generateAssign(BlockBld &blk) const {
	blk.pushInt(blk.pushConst(name), Cmd::set_var_1);
}

void Identifier::generateExpression(BlockBld &blk) const {
	blk.pushInt(blk.pushConst(name), Cmd::get_var_1);
}

ParamPackNode::ParamPackNode(PNode &&current, PNode &&nw) {
	AbstractParamPackNode *prv = dynamic_cast<AbstractParamPackNode *>(current.get());
	if (prv) {
		prv->moveTo(nodes);
		nodes.push_back(std::move(nw));
	} else {
		nodes.push_back(std::move(current));
		nodes.push_back(std::move(nw));
	}
}

void AbstractParamPackNode::makeAssign(BlockBld &blk, const PNode &n) {
	const Underscore *itm = dynamic_cast<const Underscore *>(n.get());
	if (itm == nullptr) {
		const Identifier *itm = dynamic_cast<const Identifier *>(n.get());
		if (itm == nullptr) {
			throw std::runtime_error("Invalid assignment expression");
		} else {
			blk.pushInt(blk.pushConst(itm->getName()), Cmd::set_var_ir_1);
		}
	}
}

void ParamPackNode::generateAssign(BlockBld &blk) const {
	blk.pushCmd(Cmd::reset_ir);
	auto iter = nodes.begin();
	auto end = nodes.end();
	while (true) {
		makeAssign(blk,*iter);
		++iter;
		if (iter == end) break;
		blk.pushCmd(Cmd::inc_ir);
	}

}


void ParamPackNode::generateUnclosedExpression(BlockBld &blk) const {
	for (const PNode &z: nodes) {
		z->generateExpression(blk);
	}

}


void BlockBld::pushInt(std::intptr_t val, Cmd cmd) {
	if (val >= -128 && val < 128) {
		code.push_back(static_cast<std::uint8_t>(cmd));
		code.push_back(static_cast<std::uint8_t>(val));
	} else if (val >= -32768 && val < 32768) {
		code.push_back(static_cast<std::uint8_t>(cmd)+1);
		code.push_back(static_cast<std::uint8_t>(val>>8));
		code.push_back(static_cast<std::uint8_t>(val & 0xFF));
	} else if (val >= -2147483648 && val <= 2147483647) {
		code.push_back(static_cast<std::uint8_t>(cmd)+2);
		code.push_back(static_cast<std::uint8_t>(val>>24));
		code.push_back(static_cast<std::uint8_t>((val>>16) & 0xFF));
		code.push_back(static_cast<std::uint8_t>((val>>8) & 0xFF));
		code.push_back(static_cast<std::uint8_t>(val & 0xFF));
	} else {
		code.push_back(static_cast<std::uint8_t>(cmd)+3);
		code.push_back(static_cast<std::uint8_t>(val>>56));
		code.push_back(static_cast<std::uint8_t>((val>>48) & 0xFF));
		code.push_back(static_cast<std::uint8_t>((val>>40) & 0xFF));
		code.push_back(static_cast<std::uint8_t>((val>>32) & 0xFF));
		code.push_back(static_cast<std::uint8_t>((val>>24) & 0xFF));
		code.push_back(static_cast<std::uint8_t>((val>>16) & 0xFF));
		code.push_back(static_cast<std::uint8_t>((val>>8) & 0xFF));
		code.push_back(static_cast<std::uint8_t>(val & 0xFF));
	}
}

void BlockBld::pushCmd(Cmd cmd) {
	code.push_back(static_cast<std::uint8_t>(cmd));
}

void BlockBld::setInt2(std::intptr_t val, std::size_t pos) {
	code[pos] = static_cast<std::uint8_t>(val>>8);
	code[pos+1] = static_cast<std::uint8_t>(val & 0xFF);
}

std::intptr_t BlockBld::pushConst(Value v) {
	auto r = constMap.emplace(v, constMap.size());
	return r.first->second;
}

Value Identifier::getName() const {
	return name;
}

void Underscore::generateAssign(BlockBld &blk) const {
	// nothing, can't assign to underscore
}

void Underscore::generateExpression(BlockBld &blk) const {
	throw std::runtime_error("Invalid identifier - '_' can be used in expression");
}

void ParamPackNode::moveTo(std::vector<PNode> &nodes) {
	for (PNode &n : this->nodes) {
		nodes.push_back(std::move(n));
	}
}

SingleParamPackNode::SingleParamPackNode(PNode &&n):n(std::move(n)) {}

void SingleParamPackNode::generateAssign(BlockBld &blk) const {
	blk.pushCmd(Cmd::reset_ir);
	makeAssign(blk, n);
}

void SingleParamPackNode::generateUnclosedExpression(BlockBld &blk) const {
	n->generateExpression(blk);
}

void SingleParamPackNode::moveTo(std::vector<PNode> &nodes) {
	nodes.push_back(std::move(n));
}

NumberNode::NumberNode(Value n):n(n) {}

void NumberNode::generateExpression(BlockBld &blk) const {
	blk.pushCmd(Cmd::push_double);
	double v = n.getNumber();
	auto vptr = reinterpret_cast<const std::uint8_t *>(&v);
	for (std::size_t i = 0; i < 8;i++) blk.code.push_back(vptr[i]);
}

FunctionCall::FunctionCall(PNode &&fn, PParamPackNode &&paramPack):fn(std::move(fn)),paramPack(std::move(paramPack)) {
}

void FunctionCall::generateExpression(BlockBld &blk) const {
	paramPack->generateExpression(blk);
	fn->generateExpression(blk);
	blk.pushInt(paramPack->count(), Cmd::call_fn_1);
}

ValueNode::ValueNode(Value n):n(n) {}

void ValueNode::generateExpression(BlockBld &blk) const {
	blk.pushInt(blk.pushConst(n), Cmd::push_const_1);
}

std::size_t ParamPackNode::count() const {
	return nodes.size();
}


std::size_t SingleParamPackNode::count() const {
	return 1;
}

void EmptyParamPackNode::generateAssign(BlockBld &blk) const {
	//empty - can't assign to single param pack
}

void EmptyParamPackNode::generateUnclosedExpression(BlockBld &blk) const {
	//empty
}

void EmptyParamPackNode::moveTo(std::vector<PNode> &nodes) {
	//empty
}

std::size_t EmptyParamPackNode::count() const {
	return 0;
}

void AbstractParamPackNode::generateExpression(BlockBld &blk) const {
	generateUnclosedExpression(blk);
	blk.pushInt(count(), Cmd::def_param_pack_1);

}

DirectCmdNode::DirectCmdNode(Cmd cmd) {
}

void DirectCmdNode::generateExpression(BlockBld &blk) const {
	blk.pushCmd(cmd);
}

KwWithNode::KwWithNode(PNode &&nd_object, PNode &&nd_block):nd_object(std::move(nd_object)),nd_block(std::move(nd_block)) {}

void KwWithNode::generateExpression(BlockBld &blk) const {
	nd_block->generateExpression(blk);
	nd_object->generateExpression(blk);
	blk.pushCmd(Cmd::push_scope_object);
	blk.pushCmd(Cmd::exec_block);
	blk.pushCmd(Cmd::pop_scope);
}

KwExecNode::KwExecNode(PNode &&nd_block):nd_block(std::move(nd_block)) {}

void KwExecNode::generateExpression(BlockBld &blk) const {
	nd_block->generateExpression(blk);
	blk.pushCmd(Cmd::push_scope);
	blk.pushCmd(Cmd::exec_block);
	blk.pushCmd(Cmd::pop_scope);
}

void KwExecObjectNode::generateExpression(BlockBld &blk) const {
	nd_block->generateExpression(blk);   //<block>
	nd_object->generateExpression(blk);  //<block> <object>
	blk.pushCmd(Cmd::push_scope_object); //<block>    -> object to scope
	blk.pushCmd(Cmd::exec_block);        //<exec return value>
	blk.pushCmd(Cmd::del);			     //discard return value
	blk.pushCmd(Cmd::scope_to_object);	 //convert scope to object <return value is object>
	blk.pushCmd(Cmd::pop_scope);		 //pop scope
}

void KwExecNewObjectNode::generateExpression(BlockBld &blk) const {
	nd_block->generateExpression(blk);
	blk.pushCmd(Cmd::push_scope);
	blk.pushCmd(Cmd::exec_block);
	blk.pushCmd(Cmd::del);
	blk.pushCmd(Cmd::scope_to_object);
	blk.pushCmd(Cmd::pop_scope);
}

IfElseNode::IfElseNode(PNode &&cond, PNode &&nd_then, PNode &&nd_else)
:cond(std::move(cond)),nd_then(std::move(nd_then)),nd_else(std::move(nd_else)) {}

void IfElseNode::generateExpression(BlockBld &blk) const {
	cond->generateExpression(blk);   //<evaluate condition <bool>
	blk.pushCmd(Cmd::jump_false_2);	 //will consume result and jumps
	std::size_t lb1 = blk.code.size();	//mark code location for jump
	blk.code.push_back(0);			  //reserve bytes
	blk.code.push_back(0);				//reserve bytes
	nd_then->generateExpression(blk);	//generate then expression
	blk.pushCmd(Cmd::jump_2);			//unconditional jump
	std::size_t lb2 = blk.code.size();	//mark code location
	blk.code.push_back(0);				//reserve bytes
	blk.code.push_back(0);				//reserve bytes
	std::size_t lb3 = blk.code.size();	//mark code location
	nd_else->generateExpression(blk);	//genrate else expression
	std::size_t lb4 = blk.code.size();	//mark code location
	auto dist1 = lb3-lb1-2;				//create relative distance (relative jump)
	auto dist2 = lb4-lb2-2;				//create relative distance
	if (dist1 > 0x7FFF || dist2 > 0x7FFF) throw std::runtime_error("Branch is too long (virtual machine supports max 32kB per branch)");
	blk.setInt2(dist1, lb1);			//update distance
	blk.setInt2(dist2, lb2);			//update distance
}

IfOnlyNode::IfOnlyNode(PNode &&cond, PNode &&nd_then)
:cond(std::move(cond)),nd_then(std::move(nd_then)) {}


void IfOnlyNode::generateExpression(BlockBld &blk) const {
	cond->generateExpression(blk);   //<evaluate condition <bool>
	blk.pushCmd(Cmd::jump_false_2);	 //will consume result and jumps
	std::size_t lb1 = blk.code.size();	//mark code location for jump
	blk.code.push_back(0);			  //reserve bytes
	blk.code.push_back(0);				//reserve bytes
	nd_then->generateExpression(blk);	//generate then expression
	std::size_t lb2 = blk.code.size();	//mark code location
	auto dist1 = lb2-lb1-2;				//create relative distance (relative jump)
	if (dist1 > 0x7FFF) throw std::runtime_error("Branch is too long (virtual machine supports max 32kB per branch)");
	blk.setInt2(dist1, lb1);			//update distance
}

DerefernceNode::DerefernceNode(PNode &&left, PNode &&right)
	:BinaryOperation(std::move(left), std::move(right), Cmd::deref) {}

void DerefernceDotNode::generateExpression(BlockBld &blk) const {
	left->generateExpression(blk);
	blk.pushInt(blk.pushConst(identifier), Cmd::deref_1);

}
DerefernceDotNode::DerefernceDotNode(PNode &&left, Value identifier)
: left(std::move(left)),identifier(identifier) {}

MethodCallNode::MethodCallNode(PNode &&left, Value identifier, PParamPackNode &&pp)
:left(std::move(left)),identifier(identifier),pp(std::move(pp)) {}


void MethodCallNode::generateExpression(BlockBld &blk) const {
	pp->generateExpression(blk);					//<params>
	left->generateExpression(blk);					//<params> <object>
	blk.pushCmd(Cmd::dup);							//<params> <object> <object>
	blk.pushCmd(Cmd::dup);							//<params> <object> <object> <object>
	blk.pushCmd(Cmd::push_scope_object);			//<params> <object> <object>   -> scope
	blk.pushInt(blk.pushConst("this"),Cmd::set_var_1); //<params> <object>    -> variable this
	blk.pushInt(blk.pushConst(identifier), Cmd::deref_1);	//<params> <function>    -> <object>.<identifier>
	blk.pushInt(pp->count(), Cmd::call_fn_1);		//<result>
	blk.pushCmd(Cmd::pop_scope);					//<result>
}

//Task handles executing a function - pushes argument to scope
class FunctionTask: public BlockExecution {
public:
	///Requires identifiers and block
	FunctionTask(const std::vector<std::string> &identifiers, Value block)
		:BlockExecution(block) ,identifiers(identifiers) {}

	///Called during init
	/**
	 * @param vm virtual machine
	 *
	 * Function must create scope, then pushes arguments to the scope, drops arguments and starts the function
	 */
	virtual bool init(VirtualMachine &vm) {
		vm.push_scope(Value());
		scope = true;
		auto args = vm.top_params();
		std::size_t idx = 0;
		for (const auto &a: identifiers) {
			vm.set_var(a, args[idx]);
			++idx;
		}
		vm.del_value();
		return true;
	}
	///Only handles end of the function
	/**
	 * @param vm virtual machine
	 *
	 * when function exits, removes scope from the virtual machine
	 */
	virtual bool run(VirtualMachine &vm) {
		if (!run(vm)) {
			if (scope) {
				vm.pop_scope();
				scope = false;
			}
			return false;
		} else {
			return true;
		}
	}

protected:
	bool scope = false;
	///identifiers are used only during init - so it can be const
	const std::vector<std::string> &identifiers;
};


Value defineUserFunction(std::vector<std::string> &&identifiers, PNode &&body, const CodeLocation &loc) {
	Value code = packToValue(buildCode(body, loc));

	return defineFunction([identifiers = std::move(identifiers), code](VirtualMachine &vm, Value closure) -> std::unique_ptr<AbstractTask> {
		return std::make_unique<FunctionTask>(identifiers, code);
	});
}

Block buildCode(const PNode &nd, const CodeLocation &loc) {
	BlockBld bld;
	nd->generateExpression(bld);
	std::vector<Value> consts;
	consts.resize(bld.constMap.size());
	for (const auto &itm: bld.constMap) {
		consts[itm.second] = itm.first;
	}

	return {
		consts,
		bld.code,
		loc
	};
}

}

