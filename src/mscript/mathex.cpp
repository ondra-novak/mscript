/*
 * mathex.cpp
 *
 *  Created on: 13. 3. 2022
 *      Author: ondra
 */
#include <iostream>
#include <cmath>
#include "function.h"
#include "mathex.h"

namespace mscript {

template<typename Fn>
void mathIntegralGetPoints(VirtualMachine &vm, std::size_t pts, double a, double b, Value fnval, Fn &&cb) {

	class Task: public AbstractTask {
	public:
		Task(double a, double b, std::size_t pts, Value fnval, Fn &&cb)
			:cb(std::forward<Fn>(cb)), fnval(fnval), pts(pts),a(a),b(b) {
			y.reserve(pts);
		}

		virtual bool init(VirtualMachine &vm) {
			double x = a+(b-a)*y.size()/(pts-1);
			vm.call_function(fnval, Value(), x);
			return true;
		}

		virtual bool run(VirtualMachine &vm) {
			Value z = vm.pop_value();
			y.push_back(z.getNumber());
			if (y.size() == pts) {
				cb(vm, y);
				return false;
			}
			else return init(vm);
		}

	protected:
		Fn cb;
		Value fnval;
		std::vector<double> y;
		std::size_t pts;
		double a,b;
	};

	vm.push_task(std::make_unique<Task>(a,b,pts,fnval,std::forward<Fn>(cb)));
}

void mathIntegral(VirtualMachine &vm, ValueList params) {
	Value fn = params[0];
	double a = params[1].getNumber();
	double b = params[2].getNumber();
	bool rev = b<a;
	if (rev) std::swap(a,b);
	auto steps2 =std::max<std::size_t>(2,std::min<std::size_t>(17,params[3].getValueOrDefault(json::UInt(10))));

	auto totalpts = (1<<steps2);
	auto scanpts = totalpts*3-2;

	mathIntegralGetPoints(vm, scanpts, a, b, fn, [a,b,totalpts,rev](VirtualMachine &vm, const std::vector<double> &y){
		std::vector<std::pair<double,double> > iy;
		iy.reserve(totalpts);
		iy.push_back({a,0.0});
		double h = b-a;
		double cnt1 = y.size()-1;
		for (long int i = 1; i < totalpts; i++) {
			auto j = i*3;
			double x0 = a+(j-3)*h/cnt1;
			double x3 = a+(j-0)*h/cnt1;
			double y0 = y[j-3];
			double y1 = y[j-2];
			double y2 = y[j-1];
			double y3 = y[j-0];
			double s = (x3-x0)*(y0+3*y1+3*y2+y3)/8.0;
			iy.push_back({x3,s});
		}
		auto ptcnt = iy.size();
		for (std::size_t k = 1; k < ptcnt; k = 2*k) {
			for (std::size_t l = k; l < ptcnt; l += 2*k) {
				double base = iy[l-1].second;
				for (std::size_t m = 0; m < k; m++) {
					iy[l+m].second+=base;
				}
			}
		}
		if (rev) {
			double fin = iy.back().second;
			for (auto &x: iy) x.second-=fin;
		}

/*		for (const auto &k: iy) {
			std::cout << k.first<< "," << k.second << std::endl;
		}*/

		Value outfn = defineSimpleFn([y=std::move(iy)](ValueList lst){
			double x = lst[0].getNumber();
			auto iter = std::lower_bound(y.begin(), y.end(), std::pair(x,0.0), std::less());
			auto pos = iter == y.end()?lst.size():std::distance(y.begin(), iter);
			pos = std::max<std::size_t>(2, std::min(lst.size()-2,pos));
			double x1 = y[pos-2].first;
			double y1 = y[pos-2].second;
			double x2 = y[pos-1].first;
			double y2 = y[pos-1].second;
			double x3 = y[pos].first;
			double y3 = y[pos].second;
			double x4 = y[pos+1].first;
			double y4 = y[pos+1].second;
			auto p1 = [&](double x) {return (x-x2)*(x-x3)*(x-x4);};
			auto p2 = [&](double x) {return (x-x1)*(x-x3)*(x-x4);};
			auto p3 = [&](double x) {return (x-x1)*(x-x2)*(x-x4);};
			auto p4 = [&](double x) {return (x-x1)*(x-x2)*(x-x3);};
			double res = p1(x)*y1/p1(x1)+p2(x)*y2/p2(x2)+p3(x)*y3/p3(x3)+p4(x)*y4/p4(x4);
			return res;
		});
		vm.push_value(outfn);



	});

}


class MathRootTask: public AbstractTask {
public:
	MathRootTask(ValueList params);
	virtual bool init(VirtualMachine &vm) override;
	virtual bool run(VirtualMachine &vm) override;
	bool sendError(VirtualMachine &vm) const;

protected:
	Value fn;
	double from = 0;
	double to = 0;
	double middle = 0;
	double fromVal = 0;
	double toVal = 0;
	std::size_t count;
	enum Action {
		start,
		getFromVal,
		getToVal,
		getMiddle
	};
	Action action = start;
};

MathRootTask::MathRootTask(ValueList ls) {
	fn = ls[0];
	from = ls[1].getNumber();
	to = ls[2].getNumber();
	count = ls[3].getValueOrDefault(std::size_t(30));
}

bool MathRootTask::init(VirtualMachine &vm) {
	action =start;
	return true;
}

bool MathRootTask::run(VirtualMachine &vm) {
	Value z;
	double m = 0;
	switch (action) {
	case start:  //get value at "from" point
		action = getFromVal;
		vm.call_function(fn, Value(), from);
		return true;
	case getFromVal:
		z = vm.pop_value(); //pick value "from" point
		if (z.type() != json::number) return sendError(vm); //must be number
		fromVal = z.getNumber();	//store value
		if (fromVal == 0) {
			vm.push_value(from);
			return false;
		}
		if (std::isnan(fromVal)) return sendError(vm); //if nan, send error
		action = getToVal;                           //ask for "to" value
		vm.call_function(fn, Value(), to);			//call function
		return true;
	case getToVal:
		z = vm.pop_value();		//pick value "to" point
		if (z.type() != json::number) return sendError(vm); //must be number
		toVal = z.getNumber(); //store value
		if (toVal == 0) {
			vm.push_value(to);
			return false;
		}
		//value must not be nan and fromVal and toVal must have different sign bit
		if (std::isnan(toVal) || std::signbit(fromVal) == std::signbit(toVal)) return sendError(vm);
		//ask to middle value
		action = getMiddle;
		//calculate middle
		middle = (from+to)*0.5;
		//call function
		vm.call_function(fn, Value(), middle);
		return true;
	case getMiddle: {
		z = vm.pop_value();	 //pick middle
		if (z.type() != json::number) return sendError(vm); //must be number
		m = z.getNumber();  //get number
		if (std::isnan(m)) {		//nan value is also considered as root
			vm.push_value(middle);
			return false;
		}
		count--;     //decrease tries

		auto sgz = std::signbit(m);       //pick result sign
		if (sgz == std::signbit(fromVal)) {	//fromVal and result has same sign bit
			from = middle;					//remove from-middle, continue middle-to
			fromVal = m;					//remember result as fromVal
		} else if (sgz == std::signbit(toVal)) {	//toVal and result has same sign bt
			to = middle;					//remove middle-to, continue from from-middle
			toVal = m;						//remember result as toVal
		} else {
			return sendError(vm);			//this is strange, report error
		}
		middle = (from+to)*0.5;				//half interval
		if (count == 0) {					//if reached count, return half as result
			vm.push_value(middle);
			return false;
		} else {
			vm.call_function(fn, Value(), middle);		//call function for next cycle
			return true;
		}
	}
	}
	return false;
}
bool MathRootTask::sendError(VirtualMachine &vm) const {
	vm.push_value(Value());
	return false;
}


void mathRoot(VirtualMachine &vm, ValueList params) {
	vm.push_task(std::make_unique<MathRootTask>(params));
}


}
