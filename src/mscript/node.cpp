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



void ParamPackNode::generateUnclosedExpression(BlockBld &blk) const {
	for (const PNode &z: nodes) {
		z->generateExpression(blk);
	}

}


void BlockBld::pushInt(std::intptr_t val, Cmd cmd) {
	if (cmd == Cmd::set_var_1) lastStorePos = code.size(); else lastStorePos = 0;
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
		blk.pushInt(n.getIntLong(), Cmd::push_int_1);
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
	paramPack->generateUnclosedExpression(blk);
	fn->generateExpression(blk);
	blk.pushInt(paramPack->count(), Cmd::call_fn_1);
}

ValueNode::ValueNode(Value n):n(n) {}

void ValueNode::generateExpression(BlockBld &blk) const {
	blk.pushInt(blk.pushConst(n), Cmd::push_const_1);
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
	generateUnclosedExpression(blk);
	if (count() != 1) {
		blk.pushInt(count(), infinite?Cmd::expand_param_pack_1:Cmd::def_param_pack_1);
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
	nd_else->generateExpression(blk);	//generate else expression
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
	pp->generateUnclosedExpression(blk);					//<params>
	left->generateExpression(blk);					//<params> <object>
	blk.pushCmd(Cmd::dup);							//<params> <object> <object>
	blk.pushCmd(Cmd::push_scope_object);			//<params> <object> <object>   -> scope
	blk.pushInt(blk.pushConst(identifier), Cmd::deref_fn_1);	//<params> <function>    -> <object>.<identifier>
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


std::unique_ptr<AbstractTask> UserFn::call(VirtualMachine &vm, Value closure) const  {
	return std::make_unique<FunctionTask>(identifiers, code);
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
	blk.pushInt(code.size(), Cmd::push_array_1);
}

BooleanAndOrNode::BooleanAndOrNode(PNode &&left, PNode &&right,bool and_node):left(std::move(left)),right(std::move(right)),and_node(and_node) {
}

void BooleanAndOrNode::generateExpression(BlockBld &blk) const {
	left->generateExpression(blk);
	blk.pushCmd(Cmd::dup);
	blk.pushCmd(and_node?Cmd::jump_false_2:Cmd::jump_true_2);
	std::size_t pos = blk.code.size();
	blk.code.push_back(0);
	blk.code.push_back(0);
	blk.pushCmd(Cmd::del);
	right->generateExpression(blk);
	auto dist = blk.code.size() - pos -2;
	blk.setInt2(dist, pos);
}

ForLoopNode::ForLoopNode(Value iterator, PNode &&container, std::vector<std::pair<Value,PNode> > &&init, PNode &&block)
:iterator(iterator)
,container(std::move(container))
,block(std::move(block))
,init(std::move(init))
{

}

class ForLoopFnTask: public AbstractTask {
public:
	virtual bool init(mscript::VirtualMachine &vm) {
		//parameters are: <block> <container> <iterator> <init>....
		auto params = vm.top_params();
		container = params[1]; //container - param[1]
		if (container.type() == json::number) { //container can be number - in this case, it is count of cycles
			max_cnt = (std::size_t)std::round(container.getNumber());
			no_container = true;		//we have no container
		} else {
			max_cnt = container.size();		//count are equal to size of container
			no_container = false;			//we have container
		}
		if (max_cnt == 0) return false;		//if no counting - skip everything

		block = params[0];					//pick block

		json::Object hlp;					//0,1,2 3+ init
		for (std::size_t i = 3; i < params.size(); i+=2) {
			hlp.set(params[i+1].getString(), params[i]);	//build object for scope
		}
		prevScope = hlp;					//create scope object
		iterator = params[2];				//pick iterator
		pos = 0;							//initialize position
		vm.del_value();						//discard parameters
		return true;						//function can continue

	}
	virtual bool run(mscript::VirtualMachine &vm) {
		if (pos) {									//if (pos != 0) - not first cycle
			if (pos >= max_cnt) {
				vm.pop_scope();
				return false;
			}
			vm.del_value();
			prevScope = vm.scope_to_object();
			vm.pop_scope();
		}
		vm.push_scope(prevScope);
		vm.set_var(iterator.getString(), no_container?Value(pos):container[pos]);
		vm.push_task(std::make_unique<BlockExecution>(block));
		++pos;
		return true;
	}
	Value prevScope;
	Value container;
	Value block;
	Value iterator;
	std::size_t pos;
	std::size_t max_cnt;
	bool no_container;
};

void ForLoopNode::generateExpression(BlockBld &blk) const {
	block->generateExpression(blk);
	container->generateExpression(blk);
	blk.pushInt(blk.pushConst(iterator), Cmd::push_const_1);
	for (const auto &x: init) {
		x.second->generateExpression(blk);
		blk.pushInt(blk.pushConst(x.first), Cmd::push_const_1);
	}
	static Value forLoopFn = defineFunction([](VirtualMachine &vm, Value closure){
		return std::make_unique<ForLoopFnTask>();
	});
	blk.pushInt(blk.pushConst(forLoopFn), Cmd::push_const_1);
	blk.pushInt(init.size()*2+3, Cmd::call_fn_1);


}

WhileLoopNode::WhileLoopNode(PNode &&condition, PNode &&block):condition(std::move(condition)),block(std::move(block)) {
}

void WhileLoopNode::generateExpression(BlockBld &blk) const {
	condition->generateExpression(blk); //<condition>
	blk.pushCmd(Cmd::jump_false_2);		//.... jumps if false
	blk.pushCmd(Cmd::push_null);		//<retval>
	std::intptr_t lb1 = blk.code.size();
	blk.code.push_back(0);
	blk.code.push_back(0);
	blk.pushCmd(Cmd::push_null);		//<retval> <scope>
	std::intptr_t lb2 = blk.code.size();    //<retval> <scope>
	blk.pushCmd(Cmd::push_scope_object);	//<retval>
	blk.pushCmd(Cmd::del);					//
	block->generateExpression(blk);			//<block>
	blk.pushCmd(Cmd::exec_block);			//<retval>
	blk.pushCmd(Cmd::scope_to_object);		//<retval> <scope>
	condition->generateExpression(blk);		//<retval> <scope> <condition>
	blk.pushCmd(Cmd::jump_true_2);			//<retval> <scope>   ... jumps if true
	std::intptr_t lb3 = blk.code.size();
	blk.code.push_back(0);
	blk.code.push_back(0);
	blk.pushCmd(Cmd::del);					//<retval>
	std::intptr_t lb4 = blk.code.size();	//<retval>
	blk.setInt2(lb4-lb1-2, lb1);
	blk.setInt2(lb2-lb3-2, lb3);
}


SimpleAssignNode::SimpleAssignNode(Value ident):ident(ident) {
}

void SimpleAssignNode::generateExpression(BlockBld &blk) const {
	blk.pushInt(blk.pushConst(ident.toString()), Cmd::set_var_1);
}

PackAssignNode::PackAssignNode(std::vector<Value> &&idents):idents(std::move(idents)) {

}

void PackAssignNode::generateExpression(BlockBld &blk) const {
	blk.pushInt(blk.pushConst(Value(json::array,
			idents.begin(),
			idents.end(),[](Value x){return x;})), Cmd::set_var_1);
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

void IfOnlyNode::generateListVars(VarSet &vars) const {
	cond->generateListVars(vars);
	nd_then->generateListVars(vars);
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
	const BlockValueNode *bvn = dynamic_cast<const BlockValueNode *>(block.get());

	if (bvn) bvn->generateListVars(vars);
	else vars.insert(nullptr);


	for (const auto &x: init) x.second->generateListVars(vars);
}

void WhileLoopNode::generateListVars(VarSet &vars) const {
	condition->generateListVars(vars);
	block->generateListVars(vars);
}

IsDefinedNode::IsDefinedNode(Value ident):ident(ident) {
}

void IsDefinedNode::generateExpression(BlockBld &blk) const {
	blk.pushInt(blk.pushConst(ident), Cmd::is_def_1);
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
	blk.pushInt(n, Cmd::op_add_const_1);
}

void OpAddNode::generateExpression(BlockBld &blk, const PNode &left, std::int64_t n) const {
	left->generateExpression(blk);
	blk.pushInt(n, Cmd::op_add_const_1);
}

OpSubNode::OpSubNode(PNode &&left, PNode &&right):BinaryConstOperation(std::move(left),std::move(right), Cmd::op_sub) {
}

void OpSubNode::generateExpression(BlockBld &blk, std::int64_t n, const PNode &right) const {
	right->generateExpression(blk);
	blk.pushInt(n, Cmd::op_negadd_const_1);

}

void OpSubNode::generateExpression(BlockBld &blk, const PNode &left, std::int64_t n) const {
	left->generateExpression(blk);
	blk.pushInt(-n, Cmd::op_add_const_1);
}

OpMultNode::OpMultNode(PNode &&left, PNode &&right):BinaryConstOperation(std::move(left),std::move(right), Cmd::op_mult) {
}

void OpMultNode::generateExpression(BlockBld &blk, std::int64_t n, const PNode &right) const {
	right->generateExpression(blk);
	blk.pushInt(n, Cmd::op_mult_const_1);

}

void OpMultNode::generateExpression(BlockBld &blk, const PNode &left, std::int64_t n) const {
	left->generateExpression(blk);
	blk.pushInt(n, Cmd::op_mult_const_1);

}

SwitchCaseNode::SwitchCaseNode(PNode &&selector, Labels &&labels, Nodes &&nodes, PNode &&defaultNode)
	:selector(std::move(selector)), labels(std::move(labels)),nodes(std::move(nodes)),defNode(std::move(defaultNode)) {}

void SwitchCaseNode::generateExpression(BlockBld &blk) const {
	selector->generateExpression(blk);
	std::vector<std::size_t> lbofs;
	std::vector<std::size_t> begins;
	std::vector<std::size_t> jumps;
	std::size_t defaultJump;
	for (const auto &l: labels) {
		blk.pushInt(blk.pushConst(l.first),Cmd::op_cmp_eq_1);
		blk.pushCmd(Cmd::jump_true_2);
		lbofs.push_back(blk.code.size());
		blk.code.push_back(0);
		blk.code.push_back(0);
	}
	blk.pushCmd(Cmd::del);
	if (defNode != nullptr) {
		defNode->generateExpression(blk);
	}
	blk.pushCmd(Cmd::jump_2);
	defaultJump = blk.code.size();
	blk.code.push_back(0);
	blk.code.push_back(0);
	for (const auto &n: nodes) {
		begins.push_back(blk.code.size());
		n->generateExpression(blk);
		blk.pushCmd(Cmd::jump_2);
		jumps.push_back(blk.code.size());
		blk.code.push_back(0);
		blk.code.push_back(0);
	}
	std::size_t endpos = blk.code.size();
	for (auto x: jumps) {
		blk.setInt2(endpos-x-2, x);
	}
	blk.setInt2(endpos-defaultJump-2, defaultJump);
	std::size_t i = 0;
	for (const auto &l: labels) {
		auto p = lbofs[i];
		i++;
		auto q = begins[l.second];
		blk.setInt2(q-p-2, p);
	}

}

void SwitchCaseNode::generateListVars(VarSet &vars) const {
	selector->generateListVars(vars);
	if (defNode) defNode->generateListVars(vars);
	for (const PNode &nd: nodes) nd->generateListVars(vars);
}

}
