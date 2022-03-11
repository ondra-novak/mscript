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

json::PValue ArrBldNode::collapse() const {
	auto arr = json::ArrayValue::create(lsize + rsize);
	for (std::size_t i = 0; i < lsize; i++) {
		arr->push_back(left->itemAtIndex(i));
	}
	for (std::size_t i = 0; i < rsize; i++) {
		arr->push_back(right->itemAtIndex(i));
	}
	return json::PValue::staticCast(arr);
}

json::PValue ArrBldNode::oneItemArray(json::PValue nd){
	auto arr2 = json::ArrayValue::create(1);
	arr2->push_back(nd);
	return json::PValue::staticCast(arr2);
}

json::PValue ArrBldNode::create(json::PValue left, json::PValue right) {
	return json::PValue(new ArrBldNode(left,right));
}

json::PValue ArrBldNode::push_back(json::PValue x) const {
	json::PValue nl,nr;
	if (lsize==rsize) {
		nl = collapse();
		nr = oneItemArray(x);
	} else {
		nl = left;
		if (rsize == 1) {
			nr = create(right, oneItemArray(x));
		} else {
			nr = static_cast<const ArrBldNode *>(static_cast<const json::IValue *>(right))->push_back(x);
		}
	}
	return create(nl,nr);
}

json::PValue ArrBldNode::push_front(json::PValue x) const {
	json::PValue nl,nr;
	if (lsize==rsize) {
		nl = oneItemArray(x);
		nr = collapse();
	} else {
		nr = right;
		if (lsize == 1) {
			nl = create(oneItemArray(x),left);
		} else {
			nl = static_cast<const ArrBldNode *>(static_cast<const json::IValue *>(left))->push_front(x);
		}
	}
	return create(nl,nr);
}

ArrTruncate::ArrTruncate(json::PValue src, std::size_t offset, std::size_t newsz):src(src),offs(offset),newsz(newsz) {
}

json::PValue ArrTruncate::itemAtIndex(std::size_t index) const {
	return src->itemAtIndex(index+offs);
}

std::size_t ArrTruncate::size() const {
	return newsz;
}

Value arrayPushBack(Value arr, Value item) {
	auto bld = dynamic_cast<const ArrBldNode *>(arr.getHandle()->unproxy());
	if (bld) {
		return  bld->push_back(item.getHandle());
	} else if (arr.empty()) {
		return ArrBldNode::oneItemArray(item.getHandle());
	} else {
		return ArrBldNode::create(arr.getHandle(), ArrBldNode::oneItemArray(item.getHandle()));
	}
}

Value arrayPushFront(Value arr, Value item) {
	auto bld = dynamic_cast<const ArrBldNode *>(arr.getHandle()->unproxy());
	if (bld) {
		return bld->push_front(item.getHandle());
	} else if (arr.empty()) {
		return ArrBldNode::oneItemArray(item.getHandle());
	} else {
		return ArrBldNode::create(ArrBldNode::oneItemArray(item.getHandle()), arr.getHandle());
	}
}

Value arrayPopBack(Value arr) {
	if (arr.size()<2) return Value(json::array);
	auto trn = dynamic_cast<const ArrTruncate *>(arr.getHandle()->unproxy());
	if (trn) {
		if (trn->size()*2 < trn->getSrc()->size()) {
			return arr.slice(0,-1);
		} else {
			json::PValue ret = new ArrTruncate(trn->getSrc(), 0, trn->size()-1);
			return Value(ret);
		}
	} else {
		json::PValue ret = new ArrTruncate(arr.getHandle()->unproxy(), 0, arr.size()-1);
		return Value(ret);
	}
}

Value arrayPopFront(Value arr) {
	if (arr.size()<2) return Value(json::array);
	auto trn = dynamic_cast<const ArrTruncate *>(arr.getHandle()->unproxy());
	if (trn) {
		if (trn->size()*2 < trn->getSrc()->size()) {
			return arr.slice(1);
		} else {
			json::PValue ret = new ArrTruncate(trn->getSrc(),trn->getOffset()+1, trn->size()-1);
			return Value(ret);
		}
	} else {
		json::PValue ret = new ArrTruncate(arr.getHandle()->unproxy(), 1, arr.size()-1);
		return Value(ret);
	}
}



}
