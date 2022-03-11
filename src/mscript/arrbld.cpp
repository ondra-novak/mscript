/*
 * arrbld.cpp
 *
 *  Created on: 11. 3. 2022
 *      Author: ondra
 */


#include <imtjson/arrayValue.h>
#include "arrbld.h"

namespace mscript {


ArrBldNode::ArrBldNode(json::PValue left, json::PValue right):left(left),right(right),lsize(left->size()),rsize(right->size()) {

}

std::size_t ArrBldNode::size() const {
	return lsize+rsize;
}

json::PValue ArrBldNode::itemAtIndex(std::size_t index) const {
	if (index<lsize) return left->itemAtIndex(index);
	else return right->itemAtIndex(index-lsize);
}

json::RefCntPtr<ArrBldNode> ArrBldNode::push_back(json::PValue x) const {
	json::PValue nl,nr;
	if (lsize==rsize) {
		auto arr = json::ArrayValue::create(lsize+rsize);
		for (std::size_t i = 0; i < lsize; i++) {
			arr->push_back(left->itemAtIndex(i));
		}
		for (std::size_t i = 0; i < rsize; i++) {
			arr->push_back(right->itemAtIndex(i));
		}
		nl = json::PValue::staticCast(arr);
		auto arr2 = json::ArrayValue::create(1);
		arr2->push_back(x);
		nr = json::PValue::staticCast(arr2);
	} else {
		nl = left;
		if (rsize == 1) {
			auto arr2 = json::ArrayValue::create(1);
			arr2->push_back(x);
			json::RefCntPtr<ArrBldNode> arr = new ArrBldNode(right, json::PValue::staticCast(arr2));
			nr = json::PValue::staticCast(arr);
		} else {
			auto arr = static_cast<const ArrBldNode *>(static_cast<const json::IValue *>(right))->push_back(x);
			nr = json::PValue::staticCast(arr);
		}
	}
	return new ArrBldNode(nl,nr);
}

ArrTruncate::ArrTruncate(json::PValue src, std::size_t newsz):src(src),newsz(newsz) {
}

json::PValue ArrTruncate::itemAtIndex(std::size_t index) const {
	return src->itemAtIndex(index);
}

std::size_t ArrTruncate::size() const {
	return newsz;
}

Value arrayPushBack(Value arr, Value item) {
	auto bld = dynamic_cast<const ArrBldNode *>(arr.getHandle()->unproxy());
	if (bld) {
		auto ret = bld->push_back(item.getHandle());
		return Value(json::PValue::staticCast(ret));
	} else if (arr.empty()) {
		return Value(json::array,{item},false);
	} else {
		json::PValue ret = new ArrBldNode(arr.getHandle(),Value(json::array,{item}).getHandle());
		return ret;
	}
}

Value arrayPopBack(Value arr) {
	if (arr.size()<2) return Value(json::array);
	auto trn = dynamic_cast<const ArrTruncate *>(arr.getHandle()->unproxy());
	if (trn) {
		if (trn->size()*2 < trn->getSrc()->size()) {
			return arr.slice(0,-1);
		} else {
			json::PValue ret = new ArrTruncate(trn->getSrc(), trn->size()-1);
			return Value(ret);
		}
	} else {
		json::PValue ret = new ArrTruncate(arr.getHandle()->unproxy(), arr.size()-1);
		return Value(ret);
	}
}

}
