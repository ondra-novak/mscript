/*
 * disasm.h
 *
 *  Created on: 2. 3. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_DISASM_H_
#define SRC_MSCRIPT_DISASM_H_

#include <imtjson/string.h>
#include <mscript/function.h>
#include <mscript/node.h>
#include "block.h"

namespace mscript {

template<typename Fn>
void Block::disassemble_ip(std::size_t ip, Fn &&fn) const {
	if (ip >= code.size()) fn(DisEvent::code, ip, "(end of code)");
	else {
		auto iter = code.begin()+ip;
		disassemble_iter(iter, code.end(), std::forward<Fn>(fn));
	}
}

template<typename Fn>
void Block::disassemble_iter(std::vector<std::uint8_t>::const_iterator &iter, const std::vector<std::uint8_t>::const_iterator &end, Fn &&fn) const {



	auto loadFloat = [&]() {
		char buff[8];
		for (int i = 0; i < 8; i++) {
			if (iter == end) break;
			buff[i] = *iter;
			iter++;
		}
		return *reinterpret_cast<const double *>(buff+0);
	};

	auto loadNum = [&](char flag) -> std::int64_t {
		if (iter == end) return 0;
		int count = flag - '0';
		std::int64_t res = static_cast<signed char>(*iter);
		--count;
		++iter;
		while (count) {
			res = res << 8;
			res |= *iter;
			++iter;
			--count;
		}
		return res;

	};

	auto pos =std::distance(code.begin(), iter);
	Cmd v = static_cast<Cmd>(*iter);
	++iter;
	std::string txt ( strCmd[v]);
	auto p = txt.find('$');
	if (p != txt.npos) {
		if (txt[p+1] == 'F') {
			auto f = loadFloat();
			txt = txt.substr(0,p)+std::to_string(f);
		} else {
			auto num = loadNum(txt[p+1]);
			txt = txt.substr(0,p)+std::to_string(num)+txt.substr(p+2);
		}
	} else {
		p = txt.find('@');
		if (p != txt.npos) {
			auto num = loadNum(txt[p+1]);
			auto cnst = consts[num];
			txt = txt.substr(0,p)+cnst.stringify()+txt.substr(p+2);
			if (txt.size()>50) txt = txt.substr(0,50)+"...";
		} else {
			p = txt.find('^');
			if (p != txt.npos) {
				auto num = loadNum(txt[p+1]);
				num += std::distance(code.begin(), iter);
				txt = txt.substr(0,p)+std::to_string(num)+txt.substr(p+2);
			}

		}
	}
	fn(DisEvent::code, pos, txt);

}

template<typename Fn>
void Block::disassemble(Fn &&fn) const {


		fn(DisEvent::location, location.line, location.file);

		auto iter = code.begin();
		auto end = code.end();




		while (iter != end) {
			disassemble_iter(iter, end, std::forward<Fn>(fn));
		}

		int idx = 0;
		for (Value v: consts) {
			if (isFunction(v)) {
				const AbstractFunction &afn = getFunction(v);
				const UserFn *ufn = dynamic_cast<const UserFn *>(&afn);
				if (ufn) {
					const auto &idf = ufn->getIdentifiers();
					std::string idlist;
					if (idf.empty()) idlist = "()";
					else {
						char c = '(';
						for (const auto &x: idf) {
							idlist.push_back(c);
							idlist.append(x);
							c = ',';
						}
						idlist.push_back(')');
					}
					fn(DisEvent::begin_fn, idx, idlist);
					const Block &bk = getBlockFromValue(ufn->getCode());
					bk.disassemble(std::move(fn));
					fn(DisEvent::end_fn,idx,std::string());
				}
			}
			else if (isBlock(v)) {
				const Block &bk = getBlockFromValue(v);
				fn(DisEvent::begin_block, idx, std::string());
				bk.disassemble(std::move(fn));
				fn(DisEvent::end_block, idx, std::string());
			}
			++idx;
		}
}
}
#endif /* SRC_MSCRIPT_DISASM_H_ */
