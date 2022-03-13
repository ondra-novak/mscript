/*
 * mathex.cpp
 *
 *  Created on: 13. 3. 2022
 *      Author: ondra
 */
#include <iostream>
#include <mscript/function.h>
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
			double x = (b-a)*y.size()/(pts-1);
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

}
