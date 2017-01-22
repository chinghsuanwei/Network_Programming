/*
 * Utils.h
 *
 *  Created on: 2011/11/20
 *      Author: Ching-Hsuan Wei
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <string.h>
#include <string>
#include <sstream>

//#define DEBUG

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
	static int string2Int(string str);
	static void DebugOn();
	static void write2Concole(const string& info);
	static void write2Concole(const char info[]);
	~Utils();
};
#endif /* UTILS_H_ */
