/*
 * Utils.h
 *
 *  Created on: 2012/1/1
 *      Author: ChingHsuanWei
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <sstream>
#include <string>

using namespace std;

class Utils {
public:
	Utils();
	static string integer2String(int integer);
	static int string2Integer(string str);
	static unsigned short string2UnsignedShort(string str);
	~Utils();
};

#endif /* UTILS_H_ */
