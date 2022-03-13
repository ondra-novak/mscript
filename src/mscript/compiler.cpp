/*
 * compiler.cpp
 *
 *  Created on: 17. 2. 2022
 *      Author: ondra
 */

#include <imtjson/array.h>
#include "compiler.h"
#include <unordered_map>
#include <unordered_set>

using json::Array;

namespace mscript {




PNode Compiler::compile(std::vector<Element> &&code, const CodeLocation &loc) {
	this->code = std::move(code);
	this->loc = loc;
	curLine = 0;
	lastLine = 0;
	curSymbol = 0;
	return compileBlockContent();
}

const Element& Compiler::next() {
	if (curSymbol >= code.size()) return eof;
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
		throw compileError(std::string("Expected element: ").append(strKeywords[exp.symbol]));
	}
}

void Compiler::sync(const Symbol &symbol) {
	const Element &exp = next();
	if (exp.symbol == symbol) {
		commit();
	} else {
		throw compileError(std::string("Expected symbol: `").append(strKeywords[symbol]).append("`, found:`").append(strKeywords[exp.symbol]).append("`"));
	}

}

void Compiler::checkBeginBlock() {
	auto n = next();
	if (n.symbol != Symbol::identifier && n.symbol != Symbol::s_left_brace) {
		throw compileError(std::string("Expected begin of block, found: ").append(strKeywords[n.symbol]));
	}
}


PNode Compiler::handleValueSuffixes(PNode expr) {
	PNode tmp;
	auto s = next();
	switch (s.symbol) {
	case Symbol::s_left_bracket:
		commit();
		return handleValueSuffixes(std::make_unique<FunctionCall>(std::move(expr), compileValueList()));
	case Symbol::s_left_square_bracket:
		commit();
		tmp = compileExpression();
		sync(Symbol::s_right_square_bracket);
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
				return handleValueSuffixes(std::make_unique<MethodCallNode>(std::move(expr), s.data, compileValueList()));
			} else {
				return handleValueSuffixes(std::make_unique<DerefernceDotNode>(std::move(expr), s.data));
			}
		}
		throw compileError("Expected identifier after '.' ");
		break;
	case Symbol::s_cast:
		commit();
		s = next();
		if (s.symbol == Symbol::s_left_bracket) {
			return compileCast(std::move(expr),compileValue());
		} else if (s.symbol == Symbol::identifier) {
			commit();
			return compileCast(std::move(expr),std::make_unique<Identifier>(s.data));
		} else throw compileError("Only identifier or brackets can follow the cast operator '->': X->Y, X->(Y)");
	default:
		return expr;
	}
}

PNode Compiler::compileValue() {
	PNode out;
	auto s = next();
	switch(s.symbol) {
	case Symbol::identifier:			//can be value or function call
		out = std::make_unique<Identifier>(s.data);
		commit();
		if (next().symbol == Symbol::s_arrow) out = compileDefineFunction(std::move(out));
		else out = std::move(out);
		break;
	case Symbol::number:
		out = compileNumber();
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
		out = std::make_unique<KwExecNode>(compileValue());
		break;
	case Symbol::kw_with: {
			commit();
			PNode obj = compileValue();
			checkBeginBlock();
			PNode blk = compileValue();
			out = std::make_unique<KwWithNode>(std::move(obj), std::move(blk));
		}
		break;
	case Symbol::kw_object: {
			commit();
			PNode x = compileValue();
			auto n = next();
			if (n.symbol == Symbol::identifier || n.symbol == Symbol::s_left_brace) {
				PNode y = compileValue();
				out = std::make_unique<KwExecObjectNode>(std::move(x), std::move(y));
			} else {
				out = std::make_unique<KwExecNewObjectNode>(std::move(x));
			}
		}
		break;
	case Symbol::kw_if:
		commit();
		out = parseIfElse();
		break;

	case Symbol::s_left_bracket:
		commit();
		out = compileValueList();
		if (next().symbol == Symbol::s_arrow) out = compileDefineFunction(std::move(out));
		break;
	case Symbol::s_left_brace:
		commit();
		out = compileBlock();
		out = std::make_unique<BlockValueNode>(packToValue(buildCode(out, {loc.file, loc.line+curLine})), std::move(out));
		break;
	case Symbol::s_exclamation:
		commit();
		out = std::make_unique<UnaryOperation>(compileValue(),Cmd::op_bool_not);
		break;
	case Symbol::kw_not:
		commit();
		out = std::make_unique<UnaryOperation>(compileValue(),Cmd::op_bool_not);
		break;
	case Symbol::s_minus:
		commit();
		if (next().symbol == Symbol::number) {
			auto c = compileNumber();
			auto d = static_cast<const NumberNode *>(c.get());
			out = std::make_unique<NumberNode>(-(d->getValue().getNumber()));
		} else {
			out = std::make_unique<UnaryOperation>(compileValue(),Cmd::op_unary_minus);
		}
		break;
	case Symbol::s_plus:
		commit();
		out = compileValue();
		break;
	case Symbol::s_left_square_bracket:
		commit();
		out = compileArray();
		break;
	case Symbol::kw_for:
		commit();
		sync(Symbol::s_left_bracket);
		out = compileFor();
		break;
	case Symbol::kw_while:
		commit();
		sync(Symbol::s_left_bracket);
		out = compileWhile();
		break;
	case Symbol::kw_switch:
		commit();
		out = compileSwitch();
		break;
	case Symbol::kw_constexpr:
		commit(); {
			Value v = expressionToConst();
			if (v.type() == json::number) out = std::make_unique<NumberNode>(v);
			else out = std::make_unique<ValueNode>(v);
		}
		break;
	default:
		throw compileError(std::string("Unexpected symbol: ").append(strKeywords[s.symbol]));
		break;
	}
	return handleValueSuffixes(std::move(out));
}

PNode Compiler::parseIfElse() {
	PNode cond = compileValue();
	checkBeginBlock();
	PNode blk1 = compileBlockOrExpression();
	bool eat = eatSeparators();
	if (next().symbol == Symbol::kw_else) {
		commit();
		PNode blk2;
		if (next().symbol == Symbol::kw_if) {
			commit();
			blk2 = parseIfElse();
		} else {
			blk2 = compileBlockOrExpression();
		}
		return std::make_unique<IfElseNode>(std::move(cond), std::move(blk1), std::move(blk2));
	} else {
		if (eat) {curSymbol--;curLine--;} //only else is allowed on next line, otherwise go one symbol back
		return std::make_unique<IfElseNode>(std::move(cond), std::move(blk1), std::move(std::make_unique<DirectCmdNode>(Cmd::push_null)));
	}

}

PValueListNode Compiler::compileValueList() {
	ValueListNode::Items items;
	if (next().symbol != Symbol::s_right_bracket) {
		bool rep = true;
		while (rep) {
			eatSeparators();
			PNode n = compileExpression();
			bool expand = false;
			if (next().symbol == Symbol::s_threedots) {
				expand = true;
				commit();
			}
			items.push_back({
				std::move(n),
				expand
			});
			eatSeparators();
			if (next().symbol == Symbol::s_comma) {
				commit();
			} else {
				sync(Symbol::s_right_bracket);
				rep = false;
			}
		}
	} else {
		commit();
	}
	return std::make_unique<ValueListNode>(std::move(items));
}

/*PParamPackNode Compiler::parseParamPack() {
	if (next().symbol == Symbol::s_right_bracket) {
		commit();
		return std::make_unique<EmptyParamPackNode>();
	}
	eatSeparators();
	PParamPackNode s =std::make_unique<SingleParamPackNode>(compileExpression());
	eatSeparators();
	while (next().symbol == Symbol::s_comma) {
		commit();
		eatSeparators();
		s =std::make_unique<ParamPackNode>(std::move(s),compileExpression());
		eatSeparators();
	}
	if (next().symbol == Symbol::s_threedots) {
		commit();
		s->setInfinite(true);
	}
	sync(Symbol::s_right_bracket);
	return s;
}*/



PNode Compiler::compileDefineFunction(PNode expr) {
	const Identifier *in = dynamic_cast<const Identifier *>(expr.get());
	std::vector<Value> identifiers;
	bool expandLast = false;
	if (in) {
		identifiers.push_back(in->getName().getString());
	} else {
		ValueListNode &pp = dynamic_cast<ValueListNode &>(*expr);
		const auto &items = pp.getItems();
		for (const ValueListNode::Item &k: items) {
			const Identifier *x = dynamic_cast<const Identifier *>(k.node.get());
			if (x == nullptr) throw compileError("Expected identifier in argument of the function definition");
			if (expandLast) throw compileError("Symbol ... (three dots) is allowed only for the last argument");
			expandLast = k.expandArray;
			identifiers.push_back(x->getName());
		}
	}
	commit();
	PNode blk=compileBlockOrExpression();
	Value fn = defineUserFunction(std::move(identifiers), expandLast,std::move(blk), {loc.file, loc.line+curLine});
	return std::make_unique<ValueNode>(fn);

}

class CompileTimeContent: public AbstractTask {
public:

	CompileTimeContent(bool &exception):exp(exception) {}
	virtual bool init(VirtualMachine &vm)override  {
		st = vm.save_state();
		return true;
	}
	virtual bool run(VirtualMachine &vm) override {
		return false;
	}
	virtual bool exception(VirtualMachine &vm, std::exception_ptr e) {
		exp= true;
		vm.restore_state(st);
		return true;
	}


protected:
	VirtualMachine::VMState st;
	bool &exp;


};

PNode Compiler::compileBlockContent() {
	VirtualMachine vm;
	vm.setGlobalScope(globalScope);
	vm.setComileTime(true);
	vm.push_scope(Value());
	vm.push_scope(Value());

	std::vector<PNode> nodes;
	std::unordered_set<std::string> vars;
	std::optional<Value> result;


	auto s = next();
	while (s.symbol != Symbol::s_right_brace && s.symbol != Symbol::eof) {
		auto l = curLine;
		if (s.symbol != Symbol::separator) {
			PNode cmd = compileCommand();
			Value bk = packToValue(buildCode(cmd, loc));
			result.reset();
			bool err = false;
			vm.push_task(std::make_unique<CompileTimeContent>(err));
			vm.push_task(std::make_unique<BlockExecution>(bk));
			while (vm.run());
			if (err) {
				nodes.push_back(std::make_unique<InputLineMapNode>(l,std::move(cmd)));
			} else {
				Value variables = vm.scope_to_object();
				bool has_res = true;
				for (Value x: variables) {
					auto k = x.getKey();
					if (vars.insert(std::string(k)).second) {
						has_res = false;
						nodes.push_back(
							std::make_unique<Assignment>(
									std::make_unique<SimpleAssignNode>(k),
									std::make_unique<ValueNode>(x)));
					}
				}
				if (has_res) result = vm.pop_value();

			}
		} else {
			commit();
		}
		s = next();
	}
	if (result.has_value()) {
		nodes.push_back(std::make_unique<ValueNode>(*result));
	}

	return std::make_unique<BlockNode>(std::move(nodes));

}

PNode Compiler::compileBlock() {
	PNode ret = compileBlockContent();
	sync(Symbol::s_right_brace);
	return ret;
}

PNode Compiler::compileCommand() {
	auto s = next();
	if (s.symbol == Symbol::separator) return nullptr;
	auto sv=curSymbol;
	PNode assg = tryCompileAssgn();
	if (assg == nullptr) {
		curSymbol = sv;
		PNode cmd = compileExpression();
		syncSeparator();
		return cmd;
	} else {
		PNode cmd;
		if (next().symbol == Symbol::s_qequal) {
			commit();
			const SimpleAssignNode &nd = dynamic_cast<const SimpleAssignNode &>(*assg);
			Value ident = nd.getIdent();
			cmd = compileExpression();
			cmd = std::make_unique<IfElseNode>(
					std::make_unique<IsDefinedNode>(ident),
					std::make_unique<Identifier>(ident),
					std::move(cmd)
			);
		} else {
			commit();
			cmd = compileExpression();
		}
		syncSeparator();
		return std::make_unique<Assignment>(std::move(assg),std::move(cmd));

	}

}

PNode Compiler::tryCompileAssgn() {
	auto s = next();
	if (s.symbol==Symbol::identifier) {
		commit();
		if (next().symbol==Symbol::s_equal || next().symbol==Symbol::s_qequal) {
			return std::make_unique<SimpleAssignNode>(s.data);
		}else{
			return nullptr;
		}
	} else if (s.symbol ==Symbol::s_left_bracket){
        std::vector<Value> idents;
        do {
        	commit();
        	s = next();
        	if (s.symbol==Symbol::identifier) {
        		idents.push_back(s.data);
        		commit();
        	} else if (s.symbol == Symbol::s_minus) {
        		commit();
        		idents.push_back(nullptr);
        	} else {
        		return nullptr;
        	}
        	s = next();
        } while (s.symbol==Symbol::s_comma);
        Value expand;;
        if (s.symbol == Symbol::s_threedots) {
        	expand = idents.back();
        	idents.pop_back();
        	commit();
        }
        if (s.symbol ==Symbol::s_right_bracket){
        	commit();
    		if (next().symbol==Symbol::s_equal) {
    			return std::make_unique<PackAssignNode>(std::move(idents), expand);
    		}else{
    			return nullptr;
    		}

        } else {
        	return nullptr;
        }
	} else  {
    	return nullptr;
    }

}



PNode Compiler::compileExpression() {
	PNode nd1 = compileOr();
	auto s = next();
	if (s.symbol == Symbol::s_questionmark) {
		commit();
		eatSeparators();
		PNode nd2 = compileExpression();
		eatSeparators();
		auto s = next();
		if (s.symbol == Symbol::s_doublecolon) {
			commit();
			eatSeparators();
			PNode nd3 = compileExpression();
			eatSeparators();
			return std::make_unique<IfElseNode>(std::move(nd1),std::move(nd2),std::move(nd3));
		} else{
			throw compileError("Expected ':' ");
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
	PNode nd = compileCustomBinary();
	Cmd cmd;
	switch(next().symbol) {
	case Symbol::s_dequal:
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

	switch(next().symbol) {
	case Symbol::s_plus:
		commit();
		return std::make_unique<OpAddNode>(std::move(nd), compileAddSub());
	case Symbol::s_minus:
		commit();
		return std::make_unique<OpSubNode>(std::move(nd), compileAddSub());
	default: return nd;
	}
}

PNode Compiler::compileMultDiv() {
	PNode nd = compilePower();
	Cmd cmd;
	switch(next().symbol) {
	case Symbol::s_star:
		commit();
		return std::make_unique<OpMultNode>(std::move(nd), compileMultDiv());
	case Symbol::s_slash: cmd = Cmd::op_div; break;
	case Symbol::s_percent: cmd = Cmd::op_mod; break;
	default: return nd;
	}
	commit();
	return std::make_unique<BinaryOperation>(std::move(nd), compileMultDiv(), cmd);
}

PNode Compiler::compilePower() {
	PNode nd = compileValue();
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
	eatSeparators();
	if (next().symbol != Symbol::s_right_square_bracket) {
		nds.push_back(compileExpression());
		eatSeparators();
		while (next().symbol == Symbol::s_comma) {
			commit();
			eatSeparators();
			nds.push_back(compileExpression());
			eatSeparators();
		}
		eatSeparators();
		sync(Symbol::s_right_square_bracket);
	} else {
		commit();
	}
	return std::make_unique<PushArrayNode>(std::move(nds));
}


bool Compiler::eatSeparators() {
	bool k = false;
	while (next().symbol == Symbol::separator) {
		commit();
		k = true;
	}
	return k;
}

PNode Compiler::compileFor() {
	Value iterator;
	PNode iter_value;
	PNode tmp;
	std::vector<std::pair<Value,PNode> > init;
	if (next().symbol != Symbol::s_right_bracket) {
		while (true) {
			eatSeparators();
			if (next().symbol != Symbol::identifier) {
				throw compileError("Expected identifier");
			}
			Value ident = next().data;
			commit();
			switch(next().symbol) {
			case Symbol::s_doublecolon:
				commit();
				iterator = ident;
				iter_value = compileExpression();
				break;
			case Symbol::s_equal:
				commit();
				init.push_back({ident, compileExpression()});
				break;
			default:
				throw compileError(std::string("Unexpected symbol. Expected ':' or '=' :").append(strKeywords[next().symbol]));
			}
			eatSeparators();
			if (next().symbol == Symbol::s_comma) {
				commit();
			} else {
				sync(Symbol::s_right_bracket);
				break;
			}
		}
	} else {
		commit();
	}
	if (iter_value == nullptr) {
		throw compileError("Operator `for` must have an iterator 'for (iterator: container)' ");
	}
	checkBeginBlock(); //block follows
	return std::make_unique<ForLoopNode>(iterator, std::move(iter_value), std::move(init), compileValue());
}

PNode Compiler::compileWhile() {
	PNode cond = compileExpression();
	sync(Symbol::s_right_bracket);
	checkBeginBlock(); //block follows
	return std::make_unique<WhileLoopNode>(std::move(cond), compileValue());
}

PNode Compiler::compileBlockOrExpression() {
	auto s = next();
	PNode out;
	if (s.symbol == Symbol::s_left_brace) {
		commit();
		out = compileBlock();
	} else {
		out = compileExpression();
	}
	return out;
}

CompileError Compiler::compileError(const std::string &text) {
	return CompileError(text, {loc.file, loc.line+curLine});
}

void Compiler::syncSeparator() {
	auto s= next();
	if (s.symbol == Symbol::separator) {
		commit();
	} else if (s.symbol != Symbol::eof && s.symbol != Symbol::s_right_brace) {
		throw compileError("Expected end of line or end of file");
	}
}

PNode Compiler::compileSwitch() {
	static Element case_elem = {Symbol::identifier,"case"};
	static Element default_elem = {Symbol::identifier,"default"};
	PNode selector = compileExpression();
	auto s = next();
	sync(Symbol::s_left_brace);
	eatSeparators();
	using ConstTable = std::unordered_map<Value, std::size_t>;
	using NodeTable = std::vector<PNode>;
	ConstTable ctbl;
	NodeTable ndtbl;
	PNode defaultNode;
	while (next().symbol != Symbol::s_right_brace) {
		eatSeparators();
		auto s = next();
		if (s == case_elem) {
			bool cont;
			do {
				commit();
				Value v = expressionToConst();
				if (!ctbl.emplace(v, ndtbl.size()).second) {
					throw compileError("switch-Duplicate label");
				}
				s = next();
				cont = s.symbol == Symbol::s_comma;
			} while (cont);
			sync(Symbol::s_doublecolon);
			ndtbl.push_back(compileBlockOrExpression());
			syncSeparator();
		} else if (s == default_elem) {
			if (defaultNode != nullptr) throw compileError("switch-Duplicate default");
			commit();
			sync(Symbol::s_doublecolon);
			defaultNode = compileBlockOrExpression();
			syncSeparator();
		} else if (s.symbol == Symbol::s_right_brace) {
			break;
		} else {
			throw compileError("Expected 'case' or 'default'");
		}
	}
	commit();
	if (ndtbl.empty()) {
		if (defaultNode == nullptr) return selector;
		return defaultNode;
	} else {
		return std::make_unique<SwitchCaseNode>(std::move(selector),SwitchCaseNode::Labels(ctbl.begin(), ctbl.end()), std::move(ndtbl), std::move(defaultNode));
	}
}

PNode Compiler::compileRange() {
	PNode nd = compileAddSub();
	if (next().symbol == Symbol::s_twodots) {
		commit();
		return std::make_unique<BinaryOperation>(std::move(nd),compileAddSub(), Cmd::op_mkrange);
	} else {
		return nd;
	}

}

Value Compiler::expressionToConst() {
	PNode nd = compileBlockOrExpression();
	Value code = packToValue(buildCode(nd, {loc.file, loc.line+curLine}));
	VirtualMachine vm;
	vm.setMaxExecutionTime(std::chrono::seconds(5));
	return vm.exec(std::make_unique<BlockExecution>(code));
}

PNode Compiler::compileCustomBinary() {
	PNode a = compileRange();
	auto s = next();
	while (s.symbol == Symbol::identifier) {
		commit();
		PNode b = compileRange();
		a = std::make_unique<CustomOperatorNode>(s.data, std::move(a), std::move(b));
		s = next();
	}
	return a;
}

PNode Compiler::compileNumber() {
	std::string buffer;
	buffer.append(next().data.getString());
	commit();
	Value res;
	bool dbl = false;
	auto s = next();
	while (s.symbol == Symbol::number) {
		buffer.append(next().data.getString());
		commit();
		s = next();
	}
	if (s.symbol == Symbol::s_dot) {
		commit();
		s = next();
		if (s.symbol == Symbol::number) {
			buffer.push_back('.');
			buffer.append(s.data.getString());
			dbl = true;
			commit();
			s = next();
			while (s.symbol == Symbol::number) {
				buffer.append(next().data.getString());
				commit();
				s = next();
			}

		}
	}
	s = next();
	if (s.symbol == Symbol::identifier) {
		std::string_view eval = s.data.getString();
		if (eval.length() == 1 &&  std::toupper(eval[0]) == 'E') {
			commit();
			s = next();
			buffer.append(eval);
			switch (s.symbol) {
			case Symbol::s_plus: buffer.push_back('+'); commit(); break;
			case Symbol::s_minus: buffer.push_back('-'); commit(); break;
			default: break;
			}
			s =next();
			if (s.symbol != Symbol::number) {
				compileError("Invalid number format");
			}
			buffer.append(s.data.getString());
			commit();
			dbl = true;
			s = next();
			while (s.symbol == Symbol::number) {
				buffer.append(next().data.getString());
				commit();
				s = next();
			}
		} else if (eval.length() > 1 &&  std::toupper(eval[0]) == 'E') {
			commit();
			buffer.push_back(eval[0]);
			eval = eval.substr(1);
			for (char c: eval) {
				if (!isdigit(c)) {
					compileError("Invalid number format, digits are expected");
				}
			}
			buffer.append(eval);
			dbl = true;
			s = next();
			while (s.symbol == Symbol::number) {
				buffer.append(next().data.getString());
				commit();
				s = next();
			}
		}
	}
	if (dbl) {
		res =strtod(buffer.c_str(),nullptr);
	} else {
		res = strtoull(buffer.c_str(),nullptr, 10);
	}
	return std::make_unique<NumberNode>(res);
}

Value Compiler::compileString(const CodeLocation &loc, const std::string_view &str) {
	auto iter = str.begin();
	auto e = str.end();
	return compileText(loc, [&](){return iter == e?-1:static_cast<int>(*iter++);});
}

PNode Compiler::compileCast(PNode &&expr, PNode &&baseObj) {
	auto s = next();
	std::vector<Value> path;
	while (s.symbol == Symbol::s_dot) {
		commit();
		s = next();
		sync(Symbol::identifier);
		path.push_back(s.data);
		s = next();
	}
	sync(Symbol::s_left_bracket);
	auto params = compileValueList();
	return std::make_unique<CastMethodCallNode>(std::move(expr), std::move(baseObj), std::move(path), std::move(params));
}

}

