/*
 * compiler.cpp
 *
 *  Created on: 17. 2. 2022
 *      Author: ondra
 */

#include "compiler.h"

namespace mscript {




Block compile(const std::vector<Element> &code, Value globalScope, const CodeLocation &loc) {

	PNode tree;

	return buildCode(tree, loc);

}



const Element& Compiler::next() {
	if (curSymbol > code.size()) return eof;
	else return code[curSymbol];
}

void Compiler::commit() {
	curSymbol++;
	if (next().symbol == Symbol::separator) curLine++;
}

void Compiler::sync(const Element &elem) {
	const Element &exp = next();
	if (exp == elem) {
		commit();
	} else {
		throw std::runtime_error(std::string("Expected element: ").append(strKeywords[exp.symbol]));
	}
}

void Compiler::sync(const Symbol &symbol) {
	const Element &exp = next();
	if (exp.symbol == symbol) {
		commit();
	} else {
		throw std::runtime_error(std::string("Expected symbol: ").append(strKeywords[exp.symbol]));
	}

}

void Compiler::checkBeginBlock() {
	auto n = next();
	if (n.symbol != Symbol::identifier && n.symbol != Symbol::s_left_brace) {
		throw std::runtime_error(std::string("Expected begin of block, found: ").append(strKeywords[n.symbol]));
	}
}


PNode Compiler::handleValueSuffixes(PNode expr) {
	PNode tmp;
	auto s = next();
	switch (s.symbol) {
	case Symbol::s_left_bracket:
		commit();
		return handleValueSuffixes(std::make_unique<FunctionCall>(std::move(expr), parseParamPack()));
	case Symbol::s_left_square_bracket:
		commit();
		tmp = parseValue();
		sync(Symbol::s_right_bracket);
		return handleValueSuffixes(std::make_unique<DerefernceNode>(std::move(expr), std::move(tmp)));
	case Symbol::s_dot:
		commit();
		s = next();
		if (s.symbol == Symbol::identifier) {
			commit();
			PNode vn = std::make_unique<ValueNode>(s.data);
			auto t = next();
			if (t.symbol == Symbol::s_left_bracket) {
				commit();
				return handleValueSuffixes(std::make_unique<MethodCallNode>(std::move(expr), s.data, parseParamPack()));
			} else {
				return handleValueSuffixes(std::make_unique<DerefernceDotNode>(std::move(expr), s.data));
			}
		}
		throw std::runtime_error("Expected identifier after '.' ");
		break;
	default:
		return expr;
	}
}

PNode Compiler::parseValue() {
	PNode out;
	auto s = next();
	switch(s.symbol) {
	case Symbol::identifier:			//can be value or function call
		out = std::make_unique<Identifier>(s.data);
		commit();
		if (next().symbol == Symbol::s_arrow) out = compileDefineFunction(std::move(out));
		else out = handleValueSuffixes(std::move(out));
		break;
	case Symbol::number:
		commit();
		out = std::make_unique<NumberNode>(s.data);
		break;
	case Symbol::string:
		commit();
		out = std::make_unique<ValueNode>(s.data);
		break;
	case Symbol::kw_true:
		commit();
		out = std::make_unique<BooleanNode>(true);
		break;
	case Symbol::kw_false:
		commit();
		out = std::make_unique<BooleanNode>(false);
		break;
	case Symbol::kw_null:
		commit();
		out = std::make_unique<NullNode>();
		break;
	case Symbol::kw_undefined:
		commit();
		out = std::make_unique<UndefinedNode>();
		break;
	case Symbol::kw_exec:
		commit();
		checkBeginBlock();
		out = handleValueSuffixes(std::make_unique<UnaryOperation>(parseValue(), Cmd::exec_block));
		break;
	case Symbol::kw_with: {
			commit();
			PNode obj = parseValue();
			checkBeginBlock();
			PNode blk = parseValue();
			out = std::make_unique<KwWithNode>(std::move(obj), std::move(blk));
			out = handleValueSuffixes(std::move(out));
		}
		break;
	case Symbol::kw_object: {
			commit();
			PNode x = parseValue();
			auto n = next();
			if (n.symbol == Symbol::identifier || n.symbol == Symbol::s_left_brace) {
				PNode y = parseValue();
				out = std::make_unique<KwExecObjectNode>(std::move(x), std::move(y));
			} else {
				out = std::make_unique<KwExecNewObjectNode>(std::move(x));
			}
			out = handleValueSuffixes(std::move(out));
		}
		break;
	case Symbol::kw_if:
		commit();
		out = handleValueSuffixes(parseIfElse());
		break;

	case Symbol::s_left_bracket:
		commit();
		out = parseParamPack();
		if (next().symbol == Symbol::s_arrow) out = compileDefineFunction(std::move(out));
		break;
	case Symbol::s_left_brace:
		commit();
		out = compileBlock();
		break;
	case Symbol::s_exclamation:
		commit();
		out = std::make_unique<UnaryOperation>(parseValue(),Cmd::op_bool_not);
		break;
	case Symbol::kw_not:
		commit();
		out = std::make_unique<UnaryOperation>(parseValue(),Cmd::op_bool_not);
		break;
	case Symbol::s_minus:
		commit();
		if (next().symbol == Symbol::number) {
			out = std::make_unique<NumberNode>(-next().data.getNumber());
			commit();
		} else {
			out = std::make_unique<UnaryOperation>(parseValue(),Cmd::op_unary_minus);
		}
		break;
	case Symbol::s_plus:
		commit();
		out = parseValue();
		break;
	case Symbol::s_left_square_bracket:
		commit();
		out = compileArray();
		break;
	default:
		break;
	}
	return out;
}

PNode Compiler::parseIfElse() {
	PNode cond = parseValue();
	checkBeginBlock();
	PNode blk1 = parseValue();
	if (next().symbol == Symbol::kw_else) {
		commit();
		PNode blk2;
		if (next().symbol == Symbol::kw_if) {
			commit();
			blk2 = parseIfElse();
		} else {
			checkBeginBlock();
			blk2 = parseValue();
		}
		return std::make_unique<IfElseNode>(std::move(cond), std::move(blk1), std::move(blk2));
	} else {
		return std::make_unique<IfOnlyNode>(std::move(cond), std::move(blk1));
	}

}

PParamPackNode Compiler::parseParamPack() {
	if (next().symbol == Symbol::s_right_bracket) {
		return std::make_unique<EmptyParamPackNode>();
	}
	while (next().symbol == Symbol::separator) commit();
	PParamPackNode s =std::make_unique<SingleParamPackNode>(compileExpression());
	while (next().symbol == Symbol::separator) commit();
	while (next().symbol == Symbol::s_comma) {
		commit();
		while (next().symbol == Symbol::separator) commit();
		PParamPackNode s =std::make_unique<ParamPackNode>(std::move(s),compileExpression());
		while (next().symbol == Symbol::separator) commit();
	}
	sync(Symbol::s_right_bracket);
	return s;
}

PNode Compiler::compileDefineFunction(PNode expr) {
	const Identifier *in = dynamic_cast<const Identifier *>(expr.get());
	std::vector<std::string> identifiers;
	if (in) {
		identifiers.push_back(in->getName().getString());
	} else {
		try {
			AbstractParamPackNode &pp = dynamic_cast<AbstractParamPackNode &>(*expr);
			std::vector<PNode> nodes;
			pp.moveTo(nodes);
			for (const PNode &k: nodes) {
				const Identifier &x = dynamic_cast<const Identifier &>(*k);
				identifiers.push_back(x.getName().getString());
			}
		} catch (const std::bad_cast &) {
			throw std::runtime_error("Invalid function parameter definition");
		}
	}
	commit();
	PNode blk;
	if (next().symbol != Symbol::s_left_brace) {
		blk = compileBlockContent();
	} else {
		commit();
		blk = compileBlock();
	}
	Value fn = defineUserFunction(std::move(identifiers), std::move(blk), {loc.file, loc.line+curLine});
	return std::make_unique<ValueNode>(fn);

}

PNode Compiler::compileBlockContent() {
	VirtualMachine vm;
	vm.setGlobalScope(globalScope);
	vm.push_scope(Value());

	std::vector<PNode> nodes, code;


	auto s = next();
	while (s.symbol != Symbol::s_right_brace && s.symbol != Symbol::eof) {
		if (s.symbol != Symbol::separator) {
			PNode cmd = compileCommand();
			Value bk = packToValue(buildCode(cmd, loc));
			vm.del_value();
			vm.push_task(std::make_unique<BlockExecution>(bk));
			while (vm.run());
			if (vm.get_exception()) {
				nodes.push_back(std::move(cmd));
			}
		} else {
			commit();
		}
	}
	Value variables = vm.scope_to_object();
	for (Value x: variables) {
		code.push_back(
			std::make_unique<Assignment>(
					std::make_unique<Identifier>(x.getKey()),
					std::make_unique<ValueNode>(x)));

	}
	for (PNode &nd: nodes) {
		code.push_back(std::move(nd));
	}

	return std::make_unique<BlockNode>(std::move(code));

}

PNode Compiler::compileBlock() {
	PNode ret = compileBlockContent();
	sync(Symbol::s_right_brace);
	return ret;
}

PNode Compiler::compileCommand() {
	auto s = next();
	if (s.symbol == Symbol::separator) return nullptr;
	PNode cmd = compileExpression();
	if (s.symbol == Symbol::s_equal) {
		commit();
		s = next();
		cmd = std::make_unique<Assignment>(std::move(cmd), compileExpression());
	}
	return cmd;
}

PNode Compiler::compileExpression() {
	PNode nd1 = compileOr();
	auto s = next();
	if (s.symbol == Symbol::s_questionmark) {
		commit();
		while (next().symbol == Symbol::separator) commit();
		PNode nd2 = compileExpression();
		while (next().symbol == Symbol::separator) commit();
		auto s = next();
		if (s.symbol == Symbol::s_doublecolon) {
			commit();
			while (next().symbol == Symbol::separator) commit();
			PNode nd3 = compileExpression();
			while (next().symbol == Symbol::separator) commit();
			return std::make_unique<IfElseNode>(std::move(nd1),std::move(nd2),std::move(nd3));
		} else{
			throw std::runtime_error("Expected ':' ");
		}
	} else {
		return nd1;
	}
}

PNode Compiler::compileOr() {
	PNode nd = compileAnd();
	if (next().symbol == Symbol::kw_or) {
		commit();
		return std::make_unique<BooleanAndOrNode>(std::move(nd), compileOr(), false);
	} else {
		return nd;
	}
}

PNode Compiler::compileAnd() {
	PNode nd = compileCompare();
	if (next().symbol == Symbol::kw_and) {
		commit();
		return std::make_unique<BooleanAndOrNode>(std::move(nd), compileAnd(), true);
	} else {
		return nd;
	}
}

PNode Compiler::compileCompare() {
	PNode nd = compileAddSub();
	Cmd cmd;
	switch(next().symbol) {
	case Symbol::s_equal: cmd = Cmd::op_cmp_eq; break;
	case Symbol::s_not_equal: cmd = Cmd::op_cmp_not_eq; break;
	case Symbol::s_less: cmd = Cmd::op_cmp_less; break;
	case Symbol::s_greater: cmd = Cmd::op_cmp_greater; break;
	case Symbol::s_less_equal: cmd = Cmd::op_cmp_less_eq; break;
	case Symbol::s_greater_equal: cmd = Cmd::op_cmp_greater_eq; break;
	default: return nd;
	}
	commit();
	return std::make_unique<BinaryOperation>(std::move(nd), compileCompare(), cmd);
}

PNode Compiler::compileAddSub() {
	PNode nd = compileMultDiv();
	Cmd cmd;
	switch(next().symbol) {
	case Symbol::s_plus: cmd = Cmd::op_add; break;
	case Symbol::s_minus: cmd = Cmd::op_sub; break;
	default: return nd;
	}
	commit();
	return std::make_unique<BinaryOperation>(std::move(nd), compileAddSub(), cmd);
}

PNode Compiler::compileMultDiv() {
	PNode nd = compilePower();
	Cmd cmd;
	switch(next().symbol) {
	case Symbol::s_star: cmd = Cmd::op_mult; break;
	case Symbol::s_slash: cmd = Cmd::op_div; break;
	case Symbol::s_percent: cmd = Cmd::op_mod; break;
	default: return nd;
	}
	commit();
	return std::make_unique<BinaryOperation>(std::move(nd), compileMultDiv(), cmd);
}

PNode Compiler::compilePower() {
	PNode nd = parseValue();
	Cmd cmd;
	switch(next().symbol) {
	case Symbol::s_power: cmd = Cmd::op_power; break;
	default: return nd;
	}
	commit();
	return std::make_unique<BinaryOperation>(std::move(nd), compilePower(), cmd);
}

PNode Compiler::compileArray() {
	std::vector<PNode> nds;
	while (next().symbol == Symbol::separator) commit();
	if (next().symbol != Symbol::s_right_square_bracket) {
		nds.push_back(compileExpression());
		while (next().symbol == Symbol::separator) commit();
		while (next().symbol == Symbol::s_comma) {
			commit();
			while (next().symbol == Symbol::separator) commit();
			nds.push_back(compileExpression());
			while (next().symbol == Symbol::separator) commit();
		}
		while (next().symbol == Symbol::separator) commit();
		sync(Symbol::s_right_square_bracket);
	} else {
		commit();
	}
	return std::make_unique<PushArrayNode>(std::move(nds));
}

}
