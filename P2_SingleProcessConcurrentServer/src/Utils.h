/*
 * Utils.h
 *
 *  Created on: 2011/11/13
 *      Author: Ching-Hsuan Wei
 */

#ifndef UTILS_H_
#define UTILS_H_

#define DEBUG
//#define DEMO

#include <iostream>
#include <sstream>
#include <string>
#include <string.h>

using namespace std;

struct ForwardPipe
{
	int rpipe;
	int wpipe;
};

class Utils {
public:
	Utils();
	static string int2String(int integer);
	static int string2Int(string& str);
	static void write2Concole(const string& info);
	static void write2Concole(const char info[]);
	~Utils();
};

#endif /* UTILS_H_ */
