/*
 * compiler.h
 *
 *  Created on: 17. 2. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_NODE_H_
#define SRC_MSCRIPT_NODE_H_
#include <mscript/block.h>
#include <unordered_map>

namespace mscript {

	struct BlockBld {
		std::unordered_map<Value, std::intptr_t> constMap;
		std::vector<std::uint8_t> code;
		void pushInt(std::intptr_t val, Cmd cmd);
		void pushCmd(Cmd cmd);
		void setInt2(std::intptr_t val, std::size_t pos);
		std::intptr_t pushConst(Value v);
	};


	class INode {
	public:

		virtual ~INode() {}
		///Generate code as assignment
		/**
		 * Only some nodes supports this function
		 */
		virtual void generateAssign(BlockBld &blk) const = 0;
		///Generate code as expression
		virtual void generateExpression(BlockBld &blk) const =0;
	};

	using PNode = std::unique_ptr<INode>;

	class Expression: public INode {
	public:
		virtual void generateAssign(BlockBld &blk) const override {
			throw std::runtime_error("Invalid assignment");
		}
	};

	class BinaryOperation: public Expression {
	public:
		BinaryOperation(PNode &&left, PNode &&right, Cmd instruction);
		virtual void generateExpression(BlockBld &blk) const;
	protected:
		PNode left, right;
		Cmd instruction;
	};

	class UnaryOperation: public Expression {
	public:
		UnaryOperation(PNode &&item, Cmd instruction);
		virtual void generateExpression(BlockBld &blk) const;
	protected:
		PNode item;
		Cmd instruction;
	};

	class Assignment: public Expression {
	public:
		Assignment(PNode &&assignment, PNode &&expression);
		virtual void generateExpression(BlockBld &blk) const override;
	protected:
		PNode assignment, expression;
	};

	class Identifier: public INode {
	public:
		Identifier(Value name);
		virtual void generateAssign(BlockBld &blk) const override;
		virtual void generateExpression(BlockBld &blk) const override;
		Value getName() const;
	protected:
		Value name;
	};

	class NumberNode: public Expression {
	public:
		NumberNode(Value n);
		virtual void generateExpression(BlockBld &blk) const override;
	protected:
		Value n;
	};

	class ValueNode: public Expression {
	public:
		ValueNode(Value n);
		virtual void generateExpression(BlockBld &blk) const override;
	protected:
		Value n;
	};

	class Underscore: public INode {
	public:
		virtual void generateAssign(BlockBld &blk) const override;
		virtual void generateExpression(BlockBld &blk) const override;
	};

	class AbstractParamPackNode: public INode {
	public:
		virtual void moveTo(std::vector<PNode> &nodes) = 0;
		static void makeAssign(BlockBld &blk, const PNode &n, bool arr);
		virtual std::size_t count() const = 0;
		virtual void generateUnclosedExpression(BlockBld &blk) const = 0;
		virtual void generateExpression(BlockBld &blk) const override;
		void setInfinite(bool inf) {infinite = inf;}
		bool isInfinte() const {return infinite;}
	protected:
		bool infinite = false;
	};

	using PParamPackNode = std::unique_ptr<AbstractParamPackNode>;

	class ParamPackNode: public AbstractParamPackNode {
	public:
		ParamPackNode(PNode &&current, PNode &&nw);
		virtual void generateAssign(BlockBld &blk) const override;
		virtual void generateUnclosedExpression(BlockBld &blk) const override;
		virtual void moveTo(std::vector<PNode> &nodes) override;;
		virtual std::size_t count() const override;
	protected:
		std::vector<PNode> nodes;
	};

	class SingleParamPackNode: public AbstractParamPackNode {
	public:
		SingleParamPackNode(PNode &&n);
		virtual void generateAssign(BlockBld &blk) const override;
		virtual void generateUnclosedExpression(BlockBld &blk) const override;
		virtual void moveTo(std::vector<PNode> &nodes) override;;
		virtual std::size_t count() const override;
	protected:
		PNode n;
	};

	class EmptyParamPackNode: public AbstractParamPackNode {
	public:
		virtual void generateAssign(BlockBld &blk) const override;
		virtual void generateUnclosedExpression(BlockBld &blk) const override;
		virtual void moveTo(std::vector<PNode> &nodes) override;;
		virtual std::size_t count() const override;
	protected:
		PNode n;
	};

	class FunctionCall: public Expression {
	public:
		FunctionCall(PNode &&fn, PParamPackNode &&paramPack);
	protected:
		virtual void generateExpression(BlockBld &blk) const override;
		PNode fn;
		PParamPackNode paramPack;
	};

	class DirectCmdNode: public Expression {
	public:
		DirectCmdNode(Cmd cmd);
	protected:
		Cmd cmd;
		virtual void generateExpression(BlockBld &blk) const override;
	};

	class BooleanNode: public DirectCmdNode{
	public:
		BooleanNode(bool val): DirectCmdNode(val?Cmd::push_true:Cmd::push_false){}
	};
	class NullNode: public DirectCmdNode{
	public:
		NullNode(): DirectCmdNode(Cmd::push_null) {}
	};
	class UndefinedNode: public DirectCmdNode{
	public:
		UndefinedNode(): DirectCmdNode(Cmd::push_undefined) {}
	};

	class KwExecNode: public Expression {
	public:
		KwExecNode(PNode &&nd_block);
		virtual void generateExpression(BlockBld &blk) const override;
	protected:
		PNode nd_block;
	};

	class KwWithNode: public Expression {
	public:
		KwWithNode(PNode &&nd_object, PNode &&nd_block);
		virtual void generateExpression(BlockBld &blk) const override;
	protected:
		PNode nd_object;
		PNode nd_block;
	};

	class KwExecObjectNode: public KwWithNode {
	public:
		using KwWithNode::KwWithNode;
		virtual void generateExpression(BlockBld &blk) const override;
	};

	class KwExecNewObjectNode: public KwExecNode {
	public:
		using KwExecNode::KwExecNode;
		virtual void generateExpression(BlockBld &blk) const override;
	};

	class IfElseNode: public Expression {
	public:
		IfElseNode(PNode &&cond, PNode &&nd_then, PNode &&nd_else);
		virtual void generateExpression(BlockBld &blk) const override;
	protected:
		PNode &&cond;
		PNode &&nd_then;
		PNode &&nd_else;
	};

	class IfOnlyNode: public Expression {
	public:
		IfOnlyNode(PNode &&cond, PNode &&nd_then);
		virtual void generateExpression(BlockBld &blk) const override;
	protected:
		PNode &&cond;
		PNode &&nd_then;
	};

	class DerefernceNode: public BinaryOperation {
	public:
		DerefernceNode(PNode &&left, PNode &&right);
	};

	class DerefernceDotNode: public Expression {
	public:
		DerefernceDotNode(PNode &&left, Value identifier);
		virtual void generateExpression(BlockBld &blk) const override;
	protected:
		PNode left;
		Value identifier;
	};

	class MethodCallNode: public Expression {
	public:
		MethodCallNode(PNode &&left, Value identifier, PParamPackNode &&pp);
		virtual void generateExpression(BlockBld &blk) const override;
	protected:
		PNode &&left;
		Value identifier;
		PParamPackNode &&pp;
	};


	Value defineUserFunction(std::vector<std::string> &&identifiers, PNode &&body, const CodeLocation &loc);

	Block buildCode(const PNode &nd, const CodeLocation &loc);

	class BlockNode: public Expression {
	public:
		BlockNode(std::vector<PNode> &&code);
		virtual void generateExpression(BlockBld &blk) const override;
	protected:
		std::vector<PNode> code;
	};

	class PushArrayNode: public Expression {
	public:
		PushArrayNode(std::vector<PNode> &&code);
		virtual void generateExpression(BlockBld &blk) const override;
	protected:
		std::vector<PNode> code;
	};

	class BooleanAndOrNode: public Expression {
	public:
		BooleanAndOrNode(PNode &&left,PNode &&right, bool and_node);
		virtual void generateExpression(BlockBld &blk) const override;
	protected:
		PNode left;
		PNode right;
		bool and_node;
	};

	class ForLoopNode: public Expression {
	public:
		ForLoopNode(Value iterator, PNode &&container, std::vector<std::pair<Value,PNode> > &&init, PNode &&block);
		virtual void generateExpression(BlockBld &blk) const override;
	protected:
		Value iterator;
		PNode container;
		PNode block;
		std::vector<std::pair<Value,PNode>> init;
	};

	class WhileLoopNode: public Expression {
	public:
		WhileLoopNode(PNode &&condition, PNode &&block);
		virtual void generateExpression(BlockBld &blk) const override;
	protected:
		PNode condition;
		PNode block;
	};

	class UpdateLineNode: public Expression {
	public:
		UpdateLineNode(int amount);
		virtual void generateExpression(BlockBld &blk) const override;
	protected:
		int amount;
	};

}



#endif /* SRC_MSCRIPT_NODE_H_ */
