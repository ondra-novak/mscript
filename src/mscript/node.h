/*
 * compiler.h
 *
 *  Created on: 17. 2. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_NODE_H_
#define SRC_MSCRIPT_NODE_H_
#include <mscript/block.h>
#include <mscript/function.h>
#include <unordered_set>

namespace mscript {

	struct BlockBld {
		std::unordered_map<Value, std::intptr_t> constMap;
		std::vector<std::uint8_t> code;
		std::vector<std::pair<std::size_t,std::size_t> > lines; //<code, line>
		void pushInt(std::intptr_t val, Cmd cmd, int maxSize);
		void pushCmd(Cmd cmd);
		std::intptr_t pushConst(Value v);
		std::size_t lastStorePos;

		std::size_t prepareJump(Cmd cmd, int sz);
		void finishJumpHere(std::size_t jmpPos, int sz);
		void finishJumpTo(std::size_t jmpPos, std::size_t targetPos, int sz);
		void markLine(std::size_t ln);
	};


	using VarSet = std::unordered_set<Value>;

	class INode {
	public:

		virtual ~INode() {}
		///Generate code as assignment
		/**
		 * Only some nodes supports this function
		 */
		//virtual void generateAssign(BlockBld &blk) const = 0;
		///Generate code as expression
		virtual void generateExpression(BlockBld &blk) const =0;
		virtual void generateListVars(VarSet &vars) const = 0;
	};

	using PNode = std::unique_ptr<INode>;

	class Expression: public INode {
	public:
	};

	class ConstantLeaf: public Expression {
	public:
		virtual void generateListVars(VarSet &vars) const override;
	};

	class BinaryOperation: public Expression {
	public:
		BinaryOperation(PNode &&left, PNode &&right, Cmd instruction);
		virtual void generateExpression(BlockBld &blk) const;
		virtual void generateListVars(VarSet &vars) const override;
		const PNode &getLeft() const {return left;}
		const PNode &getRight() const {return right;}
	protected:
		PNode left, right;
		Cmd instruction;
	};

	class BinaryConstOperation: public BinaryOperation {
	public:
		using BinaryOperation::BinaryOperation;
		virtual void generateExpression(BlockBld &blk) const;
		virtual void generateExpression(BlockBld &blk, std::int64_t n, const PNode &right) const = 0;
		virtual void generateExpression(BlockBld &blk, const PNode &left, std::int64_t n)  const= 0;
	};

	class OpAddNode: public BinaryConstOperation {
	public:
		OpAddNode(PNode &&left, PNode &&right);
		virtual void generateExpression(BlockBld &blk, std::int64_t n, const PNode &right) const override;
		virtual void generateExpression(BlockBld &blk, const PNode &left, std::int64_t n)  const override;
	};
	class OpSubNode: public BinaryConstOperation {
	public:
		OpSubNode(PNode &&left, PNode &&right);
		virtual void generateExpression(BlockBld &blk, std::int64_t n, const PNode &right) const override;
		virtual void generateExpression(BlockBld &blk, const PNode &left, std::int64_t n)  const override;
	};
	class OpMultNode: public BinaryConstOperation {
	public:
		OpMultNode(PNode &&left, PNode &&right);
		virtual void generateExpression(BlockBld &blk, std::int64_t n, const PNode &right) const override;
		virtual void generateExpression(BlockBld &blk, const PNode &left, std::int64_t n)  const override;
	};


	class UnaryOperation: public Expression {
	public:
		UnaryOperation(PNode &&item, Cmd instruction);
		virtual void generateExpression(BlockBld &blk) const;
		virtual void generateListVars(VarSet &vars) const override;
	protected:
		PNode item;
		Cmd instruction;
	};

	class Assignment: public Expression {
	public:
		Assignment(PNode &&assignment, PNode &&expression);
		virtual void generateExpression(BlockBld &blk) const override;
		virtual void generateListVars(VarSet &vars) const override;
	protected:
		PNode assignment, expression;
	};

	class Identifier: public INode {
	public:
		Identifier(Value name);
		virtual void generateExpression(BlockBld &blk) const override;
		virtual void generateListVars(VarSet &vars) const override;
		Value getName() const;
	protected:
		Value name;
	};

	class NumberNode: public ConstantLeaf {
	public:
		NumberNode(Value n);
		virtual void generateExpression(BlockBld &blk) const override;
		const Value &getValue() const {return n;}
	protected:
		Value n;
	};

	class ValueNode: public ConstantLeaf {
	public:
		ValueNode(Value n);
		virtual void generateExpression(BlockBld &blk) const override;
		const Value &getValue() const {return n;}
	protected:
		Value n;
	};

	class BlockValueNode: public ValueNode {
	public:
		BlockValueNode(Value n, PNode &&blockTree);
		const PNode &getBlockTree() const;
		virtual void generateListVars(VarSet &vars) const override;
	protected:
		PNode blockTree;
	};

	class ValueListNode: public Expression {
	public:

		struct Item {
			PNode node;
			bool expandArray;
		};
		using Items = std::vector<Item>;

		ValueListNode(Items &&items);
		virtual void generateExpression(BlockBld &blk) const override;
		virtual void generateListVars(VarSet &vars) const override;
		const Items &getItems() const {return items;}
	protected:
		Items items;
	};

	using PValueListNode = std::unique_ptr<ValueListNode>;

/*	class AbstractParamPackNode: public Expression {
	public:
		virtual void moveTo(std::vector<PNode> &nodes) = 0;
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
		virtual void generateUnclosedExpression(BlockBld &blk) const override;
		virtual void generateListVars(VarSet &vars) const override;
		virtual void moveTo(std::vector<PNode> &nodes) override;;
		virtual std::size_t count() const override;
	protected:
		std::vector<PNode> nodes;
	};

	class SingleParamPackNode: public AbstractParamPackNode {
	public:
		SingleParamPackNode(PNode &&n);
		virtual void generateUnclosedExpression(BlockBld &blk) const override;
		virtual void generateListVars(VarSet &vars) const override;
		virtual void moveTo(std::vector<PNode> &nodes) override;;
		virtual std::size_t count() const override;
	protected:
		PNode n;
	};

	class EmptyParamPackNode: public AbstractParamPackNode {
	public:
		virtual void generateUnclosedExpression(BlockBld &blk) const override;
		virtual void generateListVars(VarSet &vars) const override;
		virtual void moveTo(std::vector<PNode> &nodes) override;;
		virtual std::size_t count() const override;
	};
*/
	class FunctionCall: public Expression {
	public:
		FunctionCall(PNode &&fn, PValueListNode &&paramPack);
		virtual void generateListVars(VarSet &vars) const override;
	protected:
		virtual void generateExpression(BlockBld &blk) const override;
		PNode fn;
		PValueListNode paramPack;
	};

	class DirectCmdNode: public ConstantLeaf {
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

	class ExecNode: public Expression {
	public:
		ExecNode(PNode &&nd_block);
		virtual void generateExpression(BlockBld &blk) const override;
		virtual void generateListVars(VarSet &vars) const override;

	protected:
		PNode nd_block;
	};


	class KwExecNode: public ExecNode {
	public:
		using ExecNode::ExecNode;
		virtual void generateExpression(BlockBld &blk) const override;
	protected:
		PNode nd_block;
	};

	class KwWithNode: public ExecNode {
	public:
		KwWithNode(PNode &&nd_object, PNode &&nd_block);
		virtual void generateExpression(BlockBld &blk) const override;
		virtual void generateListVars(VarSet &vars) const override;
	protected:
		PNode nd_object;

		void pushScope(BlockBld &blk) const;
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
		virtual void generateListVars(VarSet &vars) const override;
	protected:
		PNode cond;
		PNode nd_then;
		PNode nd_else;
	};


	class DerefernceNode: public BinaryOperation {
	public:
		DerefernceNode(PNode &&left, PNode &&right);
	};


	class DerefernceDotNode: public Expression {
	public:
		DerefernceDotNode(PNode &&left, Value identifier);
		virtual void generateExpression(BlockBld &blk) const override;
		virtual void generateListVars(VarSet &vars) const override;
		const PNode &getLeft() const {return left;}
		const Value getIdentifier() const {return identifier;}
	protected:
		PNode left;
		Value identifier;
	};

	class MethodCallNode: public Expression {
	public:
		MethodCallNode(PNode &&left, Value identifier, PValueListNode &&pp);
		virtual void generateExpression(BlockBld &blk) const override;
		virtual void generateListVars(VarSet &vars) const override;
		static bool canReturnValueList(const PNode &nd) ;
	protected:
		PNode left;
		Value identifier;
		PValueListNode pp;
	};


	Value defineUserFunction(std::vector<std::string> &&identifiers, bool expand_last, PNode &&body, const CodeLocation &loc);

	Block buildCode(const PNode &nd, const CodeLocation &loc);

	class BlockNode: public Expression {
	public:
		BlockNode(std::vector<PNode> &&code);
		virtual void generateExpression(BlockBld &blk) const override;
		virtual void generateListVars(VarSet &vars) const override;
	protected:
		std::vector<PNode> code;
	};

	class PushArrayNode: public Expression {
	public:
		PushArrayNode(std::vector<PNode> &&code);
		virtual void generateExpression(BlockBld &blk) const override;
		virtual void generateListVars(VarSet &vars) const override;
	protected:
		std::vector<PNode> code;
	};

	class BooleanAndOrNode: public Expression {
	public:
		BooleanAndOrNode(PNode &&left,PNode &&right, bool and_node);
		virtual void generateExpression(BlockBld &blk) const override;
		virtual void generateListVars(VarSet &vars) const override;
	protected:
		PNode left;
		PNode right;
		bool and_node;
	};


	class ForLoopNode: public ExecNode {
	public:
		ForLoopNode(Value iterator, PNode &&container, std::vector<std::pair<Value,PNode> > &&init, PNode &&block);
		virtual void generateExpression(BlockBld &blk) const override;
		virtual void generateListVars(VarSet &vars) const override;
	protected:
		Value iterator;
		PNode container;
		std::vector<std::pair<Value,PNode>> init;
	};

	class WhileLoopNode: public ExecNode {
	public:
		WhileLoopNode(PNode &&condition, PNode &&block);
		virtual void generateExpression(BlockBld &blk) const override;
		virtual void generateListVars(VarSet &vars) const override;
	protected:
		PNode condition;
	};



	class UserFn: public AbstractFunction {
	public:
		UserFn(Value &&code, std::vector<std::string> &&identifiers, bool expand_last)
			:code(std::move(code)), identifiers(identifiers),expand_last(expand_last) {}
		virtual void call(VirtualMachine &vm, Value object, Value closure) const override;
		const Value& getCode() const {return code;}
		const std::vector<std::string>& getIdentifiers() const {return identifiers;}
		bool is_expand_all() const {return expand_last;}

	protected:
		Value code;
		std::vector<std::string> identifiers;
		bool expand_last;
	};

	class SimpleAssignNode: public ConstantLeaf {
	public:
		SimpleAssignNode(Value ident);
		virtual void generateExpression(BlockBld &blk) const override;
		Value getIdent() const {return ident;}
	protected:
		Value ident;
	};

	class IsDefinedNode: public ConstantLeaf {
	public:
		IsDefinedNode(Value ident);
		virtual void generateExpression(BlockBld &blk) const override;
	protected:
		Value ident;
	};


	class PackAssignNode: public ConstantLeaf {
	public:
		PackAssignNode(std::vector<Value> &&idents, Value expandIdent);
		virtual void generateExpression(BlockBld &blk) const override;
	protected:
		std::vector<Value> idents;
		Value expandIdent;
	};

	class SwitchCaseNode: public Expression {
	public:
		using Labels = std::vector<std::pair<Value, std::size_t> >;
		using Nodes = std::vector<PNode>;
		SwitchCaseNode(PNode &&selector, Labels &&labels, Nodes &&nodes, PNode &&defaultNode);
	protected:
		virtual void generateExpression(BlockBld &blk) const override;
		virtual void generateListVars(VarSet &vars) const override;
		PNode selector;
		Labels labels;
		Nodes nodes;
		PNode defNode;
	};

	class InputLineMapNode: public Expression {
	public:
		InputLineMapNode(std::size_t line, PNode &&nx);
		virtual void generateExpression(BlockBld &blk) const override;
		virtual void generateListVars(VarSet &vars) const override;
	protected:
		std::size_t line;
		PNode nx;
	};

	class CustomOperatorNode: public Expression {
	public:
		CustomOperatorNode(Value name, PNode &&left, PNode &&right);
		virtual void generateExpression(BlockBld &blk) const override;
		virtual void generateListVars(VarSet &vars) const override;
	protected:
		Value name;
		PNode left;
		PNode right;
	};

	class CastMethodCallNode: public Expression {
	public:
		CastMethodCallNode(PNode &&expr, PNode &&baseObj, std::vector<Value> &&path, PValueListNode &&vlist);
		virtual void generateExpression(BlockBld &blk) const override;
		virtual void generateListVars(VarSet &vars) const override;
	protected:
		std::vector<Value> path;
		PNode expr;
		PNode baseObj;
		PValueListNode vlist;
	};
}



#endif /* SRC_MSCRIPT_NODE_H_ */
