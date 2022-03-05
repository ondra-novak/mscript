/*
 * codelocation.h
 *
 *  Created on: 5. 3. 2022
 *      Author: ondra
 */

#ifndef SRC_MSCRIPT_CODELOCATION_H_
#define SRC_MSCRIPT_CODELOCATION_H_

namespace mscript {
struct CodeLocation {
	std::string file;
	std::size_t line;
};

}




#endif /* SRC_MSCRIPT_CODELOCATION_H_ */
