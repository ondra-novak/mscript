#ifndef SRC_MSCRIPT_UTILS_H_
#define SRC_MSCRIPT_UTILS_H_
#include <string_view>
#include "value.h"
#include "codelocation.h"
#include "parser.h"
#include "compiler.h"
#include "block.h"

namespace mscript {

template<typename Src, typename IncludeResolver>
Value compileCode(Value globalScope, const CodeLocation &loc, Src &&src, IncludeResolver &&reslv) {
	std::vector<Element> elements;
	parseScript(std::forward<Src>(src), elements);

	class Cmp: public Compiler {
	public:
		Cmp(IncludeResolver &&reslv, const std::vector<Element> &code, Value globalScope, const CodeLocation &loc)
			:Compiler(code, globalScope, loc), reslv(std::forward<IncludeResolver>(reslv)) {}

		virtual bool onInclude(std::string_view name, CodeLocation &loc, std::vector<Element> &code) {
			return reslv(name, loc, code);
		}
	protected:
		IncludeResolver reslv;
	};

	Cmp c(elements,globalScope, loc);
	return packToValue(buildCode(c.compile(), loc));
}


template<typename IncludeResolver>
Value compileString(Value globalScope, const CodeLocation &loc, std::string_view text, IncludeResolver &&reslv) {
	std::size_t n=0;
	std::size_t sz = text.size();
	return compileCode(globalScope, loc, [&](){return n>=sz?-1:(int)text[n++];}, std::forward<IncludeResolver>(reslv));
}


}


#endif
