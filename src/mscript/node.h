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
		Cmd pushInt(std::intptr_t val, Cmd cmd);
		void pushCmd(Cmd cmd);
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
		static void makeAssign(BlockBld &blk, const PNode &n);
		virtual std::size_t count() const = 0;
		virtual void generateUnclosedExpression(BlockBld &blk) const = 0;
		virtual void generateExpression(BlockBld &blk) const override;
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

}



#endif /* SRC_MSCRIPT_NODE_H_ */
