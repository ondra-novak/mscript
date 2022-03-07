#include <imtjson/string.h>
#include <mscript/function.h>
#include "node.h"
#include <cmath>

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
	expression->generateExpression(blk);
	assignment->generateExpression(blk);
}

Identifier::Identifier(Value name):name(name) {}

/*
void Identifier::generateAssign(BlockBld &blk) const {
	blk.pushInt(blk.pushConst(name), Cmd::set_var_1);
}*/

void Identifier::generateExpression(BlockBld &blk) const {
	blk.pushInt(blk.pushConst(name), Cmd::get_var_1,2);
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



void ParamPackNode::generateUnclosedExpression(BlockBld &blk) const {
	for (const PNode &z: nodes) {
		z->generateExpression(blk);
	}

}


void BlockBld::pushInt(std::intptr_t val, Cmd cmd, int maxSize) {
	if (cmd == Cmd::set_var_1) lastStorePos = code.size(); else lastStorePos = 0;
	if (val >= -128 && val < 128) {
		code.push_back(static_cast<std::uint8_t>(cmd));
		code.push_back(static_cast<std::uint8_t>(val));
	} else if (val >= -32768 && val < 32768) {
		if (maxSize < 2) throw BuildError(std::string("Parameter too long (2 bytes) for such instruction:").append(strCmd[cmd]));
		code.push_back(static_cast<std::uint8_t>(cmd)+1);
		code.push_back(static_cast<std::uint8_t>(val>>8));
		code.push_back(static_cast<std::uint8_t>(val & 0xFF));
	} else if (val >= -2147483648 && val <= 2147483647) {
		if (maxSize < 4) throw BuildError(std::string("Parameter too long (4 bytes) for such instruction:").append(strCmd[cmd]));
		code.push_back(static_cast<std::uint8_t>(cmd)+2);
		code.push_back(static_cast<std::uint8_t>(val>>24));
		code.push_back(static_cast<std::uint8_t>((val>>16) & 0xFF));
		code.push_back(static_cast<std::uint8_t>((val>>8) & 0xFF));
		code.push_back(static_cast<std::uint8_t>(val & 0xFF));
	} else {
		if (maxSize < 8) throw BuildError(std::string("Parameter too long (8 bytes) for such instruction:").append(strCmd[cmd]));
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


std::intptr_t BlockBld::pushConst(Value v) {
	auto r = constMap.emplace(v, constMap.size());
	return r.first->second;
}

void Identifier::generateListVars(VarSet &vars) const {
	vars.insert(name);
}

Value Identifier::getName() const {
	return name;
}


void ParamPackNode::moveTo(std::vector<PNode> &nodes) {
	for (PNode &n : this->nodes) {
		nodes.push_back(std::move(n));
	}
}

SingleParamPackNode::SingleParamPackNode(PNode &&n):n(std::move(n)) {}


void SingleParamPackNode::generateUnclosedExpression(BlockBld &blk) const {
	n->generateExpression(blk);
}

void SingleParamPackNode::moveTo(std::vector<PNode> &nodes) {
	nodes.push_back(std::move(n));
}

NumberNode::NumberNode(Value n):n(n) {}

void NumberNode::generateExpression(BlockBld &blk) const {
	if (n.flags() & (json::numberInteger|json::numberUnsignedInteger)) {
		blk.pushInt(n.getIntLong(), Cmd::push_int_1,8);
	} else {
		blk.pushCmd(Cmd::push_double);
		double v = n.getNumber();
		auto vptr = reinterpret_cast<const std::uint8_t *>(&v);
		for (std::size_t i = 0; i < 8;i++) blk.code.push_back(vptr[i]);
	}
}

FunctionCall::FunctionCall(PNode &&fn, PParamPackNode &&paramPack):fn(std::move(fn)),paramPack(std::move(paramPack)) {
}

void FunctionCall::generateListVars(VarSet &vars) const {
	fn->generateListVars(vars);
}

void FunctionCall::generateExpression(BlockBld &blk) const {
	paramPack->generateExpression(blk);
	auto ident = dynamic_cast<const Identifier *>(fn.get());
	if (ident) {
		blk.pushInt(blk.pushConst(ident->getName()), Cmd::call_1, 2);
	} else {
		fn->generateExpression(blk);
		blk.pushCmd(Cmd::call);
	}
}

ValueNode::ValueNode(Value n):n(n) {}

void ValueNode::generateExpression(BlockBld &blk) const {
	blk.pushInt(blk.pushConst(n), Cmd::push_const_1,2);
}

void ParamPackNode::generateListVars(VarSet &vars) const {
	for (const auto &x: nodes) x->generateListVars(vars);
}

std::size_t ParamPackNode::count() const {
	return nodes.size();
}

void SingleParamPackNode::generateListVars(VarSet &vars) const {
	n->generateListVars(vars);
}

std::size_t SingleParamPackNode::count() const {
	return 1;
}

/*
void EmptyParamPackNode::generateAssign(BlockBld &blk) const {
	//empty - can't assign to single param pack
}
*/

void EmptyParamPackNode::generateUnclosedExpression(BlockBld &blk) const {
	//empty
}

void EmptyParamPackNode::moveTo(std::vector<PNode> &nodes) {
	//empty
}

void EmptyParamPackNode::generateListVars(VarSet &vars) const {
	//empty
}

std::size_t EmptyParamPackNode::count() const {
	return 0;
}

void AbstractParamPackNode::generateExpression(BlockBld &blk) const {
	if (count()==1) {
		generateUnclosedExpression(blk);
	} else {
		blk.pushCmd(Cmd::begin_list);
		generateUnclosedExpression(blk);
		blk.pushCmd(Cmd::close_list);
	}
}

DirectCmdNode::DirectCmdNode(Cmd cmd):cmd(cmd) {
}

void DirectCmdNode::generateExpression(BlockBld &blk) const {
	blk.pushCmd(cmd);
}

ExecNode::ExecNode(PNode &&nd_block):nd_block(std::move(nd_block)) {}

void ExecNode::generateExpression(BlockBld &blk) const {
	const BlockValueNode *bvn = dynamic_cast<const BlockValueNode *>(nd_block.get());
	if (bvn) {
		bvn->getBlockTree()->generateExpression(blk);
	} else {
		nd_block->generateExpression(blk);
		blk.pushCmd(Cmd::exec_block);
	}
}


KwWithNode::KwWithNode(PNode &&nd_object, PNode &&nd_block):ExecNode(std::move(nd_block)),nd_object(std::move(nd_object)) {}

void KwWithNode::generateExpression(BlockBld &blk) const {
	nd_object->generateExpression(blk);
	blk.pushCmd(Cmd::push_scope_object);
	ExecNode::generateExpression(blk);
	blk.pushCmd(Cmd::pop_scope);
}


void KwExecNode::generateExpression(BlockBld &blk) const {
	blk.pushCmd(Cmd::push_scope);
	ExecNode::generateExpression(blk);
	blk.pushCmd(Cmd::pop_scope);
}

void KwExecObjectNode::generateExpression(BlockBld &blk) const {
	nd_object->generateExpression(blk);  //<block> <object>
	blk.pushCmd(Cmd::push_scope_object); //<block>    -> object to scope
	ExecNode::generateExpression(blk);
	blk.pushCmd(Cmd::del);			     //discard return value
	blk.pushCmd(Cmd::scope_to_object);	 //convert scope to object <return value is object>
	blk.pushCmd(Cmd::pop_scope);		 //pop scope
}

void KwExecNewObjectNode::generateExpression(BlockBld &blk) const {
	blk.pushCmd(Cmd::push_scope);
	ExecNode::generateExpression(blk);
	blk.pushCmd(Cmd::del);
	blk.pushCmd(Cmd::scope_to_object);
	blk.pushCmd(Cmd::pop_scope);
}

IfElseNode::IfElseNode(PNode &&cond, PNode &&nd_then, PNode &&nd_else)
:cond(std::move(cond)),nd_then(std::move(nd_then)),nd_else(std::move(nd_else)) {}

void IfElseNode::generateExpression(BlockBld &blk) const {
	cond->generateExpression(blk);   //<evaluate condition <bool>
	auto elsejmp = blk.prepareJump(Cmd::jump_false_1, 2);
	nd_then->generateExpression(blk);	//generate then expression
	auto thenjmp = blk.prepareJump(Cmd::jump_1, 2);
	blk.finishJumpHere(elsejmp, 2);
	nd_else->generateExpression(blk);	//generate else expression
	blk.finishJumpHere(thenjmp, 2);
}


DerefernceNode::DerefernceNode(PNode &&left, PNode &&right)
	:BinaryOperation(std::move(left), std::move(right), Cmd::deref) {}

void DerefernceDotNode::generateExpression(BlockBld &blk) const {
	left->generateExpression(blk);
	blk.pushInt(blk.pushConst(identifier), Cmd::deref_1,2);

}
DerefernceDotNode::DerefernceDotNode(PNode &&left, Value identifier)
: left(std::move(left)),identifier(identifier) {}

MethodCallNode::MethodCallNode(PNode &&left, Value identifier, PParamPackNode &&pp)
:left(std::move(left)),identifier(identifier),pp(std::move(pp)) {}


void MethodCallNode::generateExpression(BlockBld &blk) const {
	pp->generateExpression(blk);					//<params>
	left->generateExpression(blk);					//<params> <object>
	blk.pushInt(blk.pushConst(identifier), Cmd::mcall_1,2);
}

//Task handles executing a function - pushes argument to scope
class FunctionTask: public BlockExecution {
public:
	///Requires identifiers and block
	FunctionTask(const std::vector<std::string> &identifiers, Value block, Value object, Value closure)
		:BlockExecution(block) ,identifiers(identifiers),object(object),closure(closure) {}

	///Called during init
	/**
	 * @param vm virtual machine
	 *
	 * Function must create scope, then pushes arguments to the scope, drops arguments and starts the function
	 */
	virtual bool init(VirtualMachine &vm) {
		if (object.defined()) vm.push_scope(object);
		vm.push_scope(closure);
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
				if (object.defined()) {
					vm.pop_scope();
				}
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
	Value object;
	Value closure;
};


std::unique_ptr<AbstractTask> UserFn::call(VirtualMachine &vm, Value object, Value closure) const  {
	return std::make_unique<FunctionTask>(identifiers, code, object, closure);
}


Value defineUserFunction(std::vector<std::string> &&identifiers, PNode &&body, const CodeLocation &loc) {
	Value code = packToValue(buildCode(body, loc));
	auto ptr = std::make_unique<UserFn>(std::move(code), std::move(identifiers));
	std::string name = "Function "+loc.file+":"+std::to_string(loc.line);
	return packToValue(std::unique_ptr<AbstractFunction>(std::move(ptr)), name);
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

void BlockNode::generateExpression(BlockBld &blk) const {
	if (code.empty()) {
		blk.pushCmd(Cmd::push_null); //block must return a value - empty block returns null
	} else {
		auto iter = code.begin();
		auto end = code.end();
		(*iter)->generateExpression(blk);
		++iter;
		while (iter != end) {
			if (blk.lastStorePos) {
				constexpr auto diff = static_cast<int>(Cmd::pop_var_1)-static_cast<int>(Cmd::set_var_1);
				blk.code[blk.lastStorePos]+=diff;
				blk.lastStorePos = 0;
			} else {
				blk.pushCmd(Cmd::del);
			}
			(*iter)->generateExpression(blk);
			++iter;
		}
	}
}

BlockNode::BlockNode(std::vector<PNode> &&code):code(std::move(code)) {
}
PushArrayNode::PushArrayNode(std::vector<PNode> &&code):code(std::move(code)) {
}

void PushArrayNode::generateExpression(BlockBld &blk) const {
	for (const PNode &nd: code) {
		nd->generateExpression(blk);
	}
	blk.pushInt(code.size(), Cmd::push_array_1,4);
}

BooleanAndOrNode::BooleanAndOrNode(PNode &&left, PNode &&right,bool and_node):left(std::move(left)),right(std::move(right)),and_node(and_node) {
}

void BooleanAndOrNode::generateExpression(BlockBld &blk) const {
	left->generateExpression(blk);
	blk.pushCmd(Cmd::dup);
	auto l1 = blk.prepareJump(and_node?Cmd::jump_false_1:Cmd::jump_true_1, 2);
	blk.pushCmd(Cmd::del);
	right->generateExpression(blk);
	blk.finishJumpHere(l1, 2);
}

ForLoopNode::ForLoopNode(Value iterator, PNode &&container, std::vector<std::pair<Value,PNode> > &&init, PNode &&block)
:ExecNode(std::move(block))
,iterator(iterator)
,container(std::move(container))
,init(std::move(init))
{

}


void ForLoopNode::generateExpression(BlockBld &blk) const {
	blk.pushCmd(Cmd::push_null); //<ret>
	if (init.empty()) {
		blk.pushCmd(Cmd::push_null);
	} else {
		blk.pushCmd(Cmd::push_scope);
		for (const auto &x: init) {
			x.second->generateExpression(blk);
			blk.pushInt(blk.pushConst(x.first), Cmd::pop_var_1, 2);
		}
		blk.pushCmd(Cmd::scope_to_object);
		blk.pushCmd(Cmd::pop_scope);
	}
	// <ret><scope>
	container->generateExpression(blk);
	// <ret><scope><container>
	blk.pushCmd(Cmd::push_zero_int);
	// <ret><scope><container><idx>
	auto label = blk.code.size();
	blk.pushInt(1, Cmd::dup_1, 1);  //<ret><scope><container><idx><container>
	blk.pushInt(1, Cmd::dup_1, 1);	//<ret><scope><container><idx><container><idx>
	blk.pushCmd(Cmd::op_checkbound); //<ret><scope><container><idx><bool>
	auto jpout = blk.prepareJump(Cmd::jump_false_1, 2); //<ret><scope><container><idx>
	blk.pushInt(2, Cmd::dup_1, 1);	//<ret><scope><container><idx><scope>
	blk.pushCmd(Cmd::push_scope_object); //<ret><scope><container><idx>
	blk.pushInt(1, Cmd::dup_1, 1);  //<ret><scope><container><idx><container>
	blk.pushInt(1, Cmd::dup_1, 1);	//<ret><scope><container><idx><container><idx>
	blk.pushCmd(Cmd::deref);		//<ret><scope><container><idx><value>
	blk.pushInt(blk.pushConst(iterator), Cmd::pop_var_1, 2); //<ret><scope><container><idx>
	ExecNode::generateExpression(blk);	//generate block execution <ret><scope><container><idx><ret>
	blk.pushInt(4, Cmd::swap_1, 1);		//swap old ret with new ret
	blk.pushCmd(Cmd::del);				//<ret><scope><container><idx>
	blk.pushCmd(Cmd::scope_to_object);	//generate block execution <ret><scope><container><idx><scope>
	blk.pushCmd(Cmd::pop_scope);
	blk.pushInt(3, Cmd::swap_1, 1);	//swap old scope with new scope
	blk.pushCmd(Cmd::del);			//<ret><scope><container><idx>
	blk.pushInt(1,Cmd::op_add_const_1,8); //<ret><scope><container><idx+1>
	blk.finishJumpTo(blk.prepareJump(Cmd::jump_1, 2), label,2);
	blk.finishJumpHere(jpout, 2);	//<ret><scope><container><idx>
	blk.pushCmd(Cmd::del);			//<ret><scope><container>
	blk.pushCmd(Cmd::del);			//<ret><scope>
	blk.pushCmd(Cmd::del);			//<ret>

}

WhileLoopNode::WhileLoopNode(PNode &&condition, PNode &&block):ExecNode(std::move(block)),condition(std::move(condition)) {
}

void WhileLoopNode::generateExpression(BlockBld &blk) const {
	blk.pushCmd(Cmd::push_null);		//<retval>
	condition->generateExpression(blk); //<retval> <condition>
	auto skp = blk.prepareJump(Cmd::jump_false_1, 2);
	blk.pushCmd(Cmd::push_null);		//<retval> <scope>
	auto rephere = blk.code.size();
	blk.pushCmd(Cmd::push_scope_object);	//<retval>
	blk.pushCmd(Cmd::del);					//
	ExecNode::generateExpression(blk);		//<retval>
	blk.pushCmd(Cmd::scope_to_object);		//<retval> <scope>
	condition->generateExpression(blk);		//<retval> <scope> <condition>
	blk.pushCmd(Cmd::pop_scope);
	blk.finishJumpTo(blk.prepareJump(Cmd::jump_true_1, 2), rephere, 2); //<retval><scope>
	blk.pushCmd(Cmd::del);					//<retval>
	blk.finishJumpHere(skp, 2);
}


SimpleAssignNode::SimpleAssignNode(Value ident):ident(ident) {
}

void SimpleAssignNode::generateExpression(BlockBld &blk) const {
	blk.pushInt(blk.pushConst(ident.toString()), Cmd::set_var_1,2);
}

PackAssignNode::PackAssignNode(std::vector<Value> &&idents):idents(std::move(idents)) {

}

void PackAssignNode::generateExpression(BlockBld &blk) const {
	blk.pushInt(blk.pushConst(Value(json::array,
			idents.begin(),
			idents.end(),[](Value x){return x;})), Cmd::set_var_1,2);
}

void ConstantLeaf::generateListVars(VarSet &vars) const {
	//empty
}

void BinaryOperation::generateListVars(VarSet &vars) const {
	left->generateListVars(vars);
	right->generateListVars(vars);
}

void UnaryOperation::generateListVars(VarSet &vars) const {
	item->generateListVars(vars);
}

void Assignment::generateListVars(VarSet &vars) const {
	expression->generateListVars(vars);
}

void ExecNode::generateListVars(VarSet &vars) const {
	const BlockValueNode *bvn = dynamic_cast<const BlockValueNode *>(nd_block.get());
	if (bvn) {
		bvn->getBlockTree()->generateListVars(vars);
	} else {
		vars.insert(nullptr);
	}
}


void KwWithNode::generateListVars(VarSet &vars) const {
	ExecNode::generateListVars(vars);
	nd_object->generateListVars(vars);
}

void IfElseNode::generateListVars(VarSet &vars) const {
	cond->generateListVars(vars);
	nd_then->generateListVars(vars);
	nd_else->generateListVars(vars);
}


void DerefernceDotNode::generateListVars(VarSet &vars) const {
	left->generateListVars(vars);
}

void MethodCallNode::generateListVars(VarSet &vars) const {
	left->generateListVars(vars);
	pp->generateListVars(vars);

}

void BlockNode::generateListVars(VarSet &vars) const {
	for (const auto &x: code) x->generateListVars(vars);
}

void PushArrayNode::generateListVars(VarSet &vars) const {
	for (const auto &x: code) x->generateListVars(vars);
}

void BooleanAndOrNode::generateListVars(VarSet &vars) const {
	left->generateListVars(vars);
	right->generateListVars(vars);
}

void ForLoopNode::generateListVars(VarSet &vars) const {
	container->generateListVars(vars);
	ExecNode::generateListVars(vars);
	for (const auto &x: init) x.second->generateListVars(vars);
}

void WhileLoopNode::generateListVars(VarSet &vars) const {
	condition->generateListVars(vars);
	ExecNode::generateListVars(vars);
}

IsDefinedNode::IsDefinedNode(Value ident):ident(ident) {
}

void IsDefinedNode::generateExpression(BlockBld &blk) const {
	blk.pushInt(blk.pushConst(ident), Cmd::is_def_1,2);
}

BlockValueNode::BlockValueNode(Value n, PNode &&blockTree):ValueNode(n),blockTree(std::move(blockTree)) {
}

const PNode& BlockValueNode::getBlockTree() const {
	return blockTree;
}



void BinaryConstOperation::generateExpression(BlockBld &blk) const {
	auto vl = dynamic_cast<const NumberNode *>(left.get());
	auto vr = dynamic_cast<const NumberNode *>(right.get());
	if (vl) {
		const Value &n = vl->getValue();
		if (n.flags() & (json::numberInteger|json::numberUnsignedInteger)) {
			generateExpression(blk, n.getIntLong(), right);
			return;
		}
	}
	if (vr) {
		const Value &n = vr->getValue();
		if (n.flags() & (json::numberInteger|json::numberUnsignedInteger)) {
			generateExpression(blk, left, n.getIntLong());
			return;
		}

	}
	BinaryOperation::generateExpression(blk);
}

OpAddNode::OpAddNode(PNode &&left, PNode &&right):BinaryConstOperation(std::move(left),std::move(right), Cmd::op_add) {
}

void OpAddNode::generateExpression(BlockBld &blk, std::int64_t n, const PNode &right) const {
	right->generateExpression(blk);
	blk.pushInt(n, Cmd::op_add_const_1,2);
}

void OpAddNode::generateExpression(BlockBld &blk, const PNode &left, std::int64_t n) const {
	left->generateExpression(blk);
	blk.pushInt(n, Cmd::op_add_const_1,2);
}

OpSubNode::OpSubNode(PNode &&left, PNode &&right):BinaryConstOperation(std::move(left),std::move(right), Cmd::op_sub) {
}

void OpSubNode::generateExpression(BlockBld &blk, std::int64_t n, const PNode &right) const {
	right->generateExpression(blk);
	blk.pushInt(n, Cmd::op_negadd_const_1,2);

}

void OpSubNode::generateExpression(BlockBld &blk, const PNode &left, std::int64_t n) const {
	left->generateExpression(blk);
	blk.pushInt(-n, Cmd::op_add_const_1,2);
}

OpMultNode::OpMultNode(PNode &&left, PNode &&right):BinaryConstOperation(std::move(left),std::move(right), Cmd::op_mult) {
}

void OpMultNode::generateExpression(BlockBld &blk, std::int64_t n, const PNode &right) const {
	right->generateExpression(blk);
	blk.pushInt(n, Cmd::op_mult_const_1,2);

}

void OpMultNode::generateExpression(BlockBld &blk, const PNode &left, std::int64_t n) const {
	left->generateExpression(blk);
	blk.pushInt(n, Cmd::op_mult_const_1,2);

}

SwitchCaseNode::SwitchCaseNode(PNode &&selector, Labels &&labels, Nodes &&nodes, PNode &&defaultNode)
	:selector(std::move(selector)), labels(std::move(labels)),nodes(std::move(nodes)),defNode(std::move(defaultNode)) {}

void SwitchCaseNode::generateExpression(BlockBld &blk) const {
	selector->generateExpression(blk);
	std::vector<std::size_t> lbofs;
	std::vector<std::size_t> begins;
	std::vector<std::size_t> jumps;
	for (const auto &l: labels) {
		blk.pushInt(blk.pushConst(l.first),Cmd::op_cmp_eq_1,2);
		lbofs.push_back(blk.prepareJump(Cmd::jump_true_1, 2));
	}
	if (defNode != nullptr) {
		blk.pushCmd(Cmd::del);
		defNode->generateExpression(blk);
	}
	auto skp = blk.prepareJump(Cmd::jump_1, 2);
	for (const auto &n: nodes) {
		begins.push_back(blk.code.size());
		n->generateExpression(blk);
		jumps.push_back(blk.prepareJump(Cmd::jump_1, 2));
	}
	for (auto x: jumps) {
		blk.finishJumpHere(x, 2);
	}
	blk.finishJumpHere(skp, 2);
	std::size_t i = 0;
	for (const auto &l: labels) {
		blk.finishJumpTo(lbofs[i], begins[l.second], 2);
		i++;
	}

}

void SwitchCaseNode::generateListVars(VarSet &vars) const {
	selector->generateListVars(vars);
	if (defNode) defNode->generateListVars(vars);
	for (const PNode &nd: nodes) nd->generateListVars(vars);
}

std::size_t BlockBld::prepareJump(Cmd cmd, int sz) {
	switch(sz) {
	case 1: pushCmd(cmd);break;
	case 2: pushCmd(static_cast<Cmd>(static_cast<int>(cmd)+1));break;
	case 4: pushCmd(static_cast<Cmd>(static_cast<int>(cmd)+2));break;
	case 8: pushCmd(static_cast<Cmd>(static_cast<int>(cmd)+3));break;
	default: throw BuildError("Invalid jump size (allowed: 1,2,4,8)");
	}
	std::size_t out = code.size();
	for (int i = 0; i < sz; i++) {
		code.push_back(0);
	}
	return out;

}

void BlockBld::finishJumpHere(std::size_t jmpPos, int sz) {
	finishJumpTo(jmpPos, code.size(), sz);
}

void BlockBld::finishJumpTo(std::size_t jmpPos, std::size_t targetPos, int sz) {
	std::intptr_t distance = targetPos;
	distance -= jmpPos;
	distance -= sz;
	if (!(distance >= -128 && distance < 128)
		&&(((distance >= -32768 && distance < 32768) && (sz < 2))
			|| ((distance >= -2147483648 && distance <= 2147483647) && (sz < 4))
			|| (sz < 8))) throw BuildError("Jump is too far");
	for (int i = 0; i < sz; i++) {
		int shift = 8*(sz - i - 1);
		std::intptr_t x = distance >> shift;
		code[jmpPos+i] = x & 0xFF;
	}
}

}
