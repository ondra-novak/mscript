/*
 * generator.cpp
 *
 *  Created on: 12. 3. 2022
 *      Author: ondra
 */

#include "generator.h"

namespace mscript {

Generator::Generator() {
	thr = std::thread([&]{
		worker();
	});
}

Generator::~Generator() {
	exit = true;
	side = 1;
	this->cond.notify_all();
	thr.join();
}

void Generator::resume_generator() {
	std::unique_lock _(mx);
	side=1;
	this->cond.notify_all();
	this->cond.wait(_, [&]{return side==0;});
	if (exit) throw terminate;

}

void Generator::suspend_generator() {
	std::unique_lock _(mx);
	side=0;
	this->cond.notify_all();
	this->cond.wait(_, [&]{return side==1;});
}

void Generator::worker() {
	try {
		suspend_generator();
		if (exit) return;
		generatingFunction();
	} catch (_Terminate) {
		//empty;
	}
	done = true;
	while (!exit) {
		suspend_generator();
	}
}

}
