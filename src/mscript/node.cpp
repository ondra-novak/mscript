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
	blk.pushCmd(blk.pushInt(blk.pushConst(name), Cmd::set_var_1));
}

void Identifier::generateExpression(BlockBld &blk) const {
	blk.pushCmd(blk.pushInt(blk.pushConst(name), Cmd::get_var_1));
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
			blk.pushCmd(blk.pushInt(blk.pushConst(itm->getName()), Cmd::set_var_ir_1));
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

void AbstractParamPackNode::generateUnclosedExpression(BlockBld &blk) const {
	generateUnclosedExpression(blk);
	blk.pushCmd(blk.pushInt(count(), Cmd::def_param_pack_1));
}

void ParamPackNode::generateUnclosedExpression(BlockBld &blk) const {
	for (const PNode &z: nodes) {
		z->generateExpression(blk);
	}

}


Cmd BlockBld::pushInt(std::intptr_t val, Cmd cmd) {
	if (val >= -128 && val < 128) {
		code.push_back(static_cast<std::uint8_t>(val));
		return cmd;
	} else if (val >= -32768 && val < 32768) {
		code.push_back(static_cast<std::uint8_t>(val>>8));
		code.push_back(static_cast<std::uint8_t>(val & 0xFF));
		return static_cast<Cmd>(static_cast<int>(cmd)+1);
	} else if (val >= -32768 && val < 32768) {
		code.push_back(static_cast<std::uint8_t>(val>>8));
		code.push_back(static_cast<std::uint8_t>(val & 0xFF));
		return static_cast<Cmd>(static_cast<int>(cmd)+1);
	} else if (val >= -2147483648 && val <= 2147483647) {
		code.push_back(static_cast<std::uint8_t>(val>>24));
		code.push_back(static_cast<std::uint8_t>((val>>16) & 0xFF));
		code.push_back(static_cast<std::uint8_t>((val>>8) & 0xFF));
		code.push_back(static_cast<std::uint8_t>(val & 0xFF));
		return static_cast<Cmd>(static_cast<int>(cmd)+2);
	} else {
		code.push_back(static_cast<std::uint8_t>(val>>56));
		code.push_back(static_cast<std::uint8_t>((val>>48) & 0xFF));
		code.push_back(static_cast<std::uint8_t>((val>>40) & 0xFF));
		code.push_back(static_cast<std::uint8_t>((val>>32) & 0xFF));
		code.push_back(static_cast<std::uint8_t>((val>>24) & 0xFF));
		code.push_back(static_cast<std::uint8_t>((val>>16) & 0xFF));
		code.push_back(static_cast<std::uint8_t>((val>>8) & 0xFF));
		code.push_back(static_cast<std::uint8_t>(val & 0xFF));
		return static_cast<Cmd>(static_cast<int>(cmd)+3);
	}
}

void BlockBld::pushCmd(Cmd cmd) {
	code.push_back(static_cast<std::uint8_t>(cmd));
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
	blk.pushCmd(blk.pushInt(paramPack->count(), Cmd::call_fn_1));
}

ValueNode::ValueNode(Value n):n(n) {}

void ValueNode::generateExpression(BlockBld &blk) const {
	blk.pushCmd(blk.pushInt(blk.pushConst(n), Cmd::push_const_1));
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

}
