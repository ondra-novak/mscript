/*
 * generator.h
 *
 *  Created on: 12. 3. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_GENERATOR_H_
#define SRC_MSCRIPT_GENERATOR_H_


#include <condition_variable>
#include <thread>
#include <mutex>


namespace mscript {

///Generator class - implemented using threads until C++2x, where co_rutines will be used
class Generator {
public:

	Generator();
	virtual ~Generator();

	virtual void generatingFunction() = 0;

	void resume_generator();
	void suspend_generator();

protected:
	std::mutex mx;
	std::condition_variable cond;
	std::thread thr;
	int side = 0;
	bool exit = false;
	bool done = false;

	enum _Terminate {terminate};

	void worker();


};





}



#endif /* SRC_MSCRIPT_GENERATOR_H_ */
