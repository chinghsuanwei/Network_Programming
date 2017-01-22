/*
 * Utils.cpp
 *
 *  Created on: 2012/1/1
 *      Author: ChingHsuanWei
 */

#include "Utils.h"

Utils::Utils() {
	// TODO Auto-generated constructor stub

}

string Utils::integer2String(int integer)
{
	stringstream ss;
	string str;

	ss << integer;
	ss >> str;

	return str;
}

int Utils::string2Integer(string str)
{
	stringstream ss;
	int integer;

	ss << str;
	ss >> integer;

	return integer;
}

unsigned short Utils::string2UnsignedShort(string str)
{
	stringstream ss;
	unsigned short ush;

	ss << str;
	ss >> ush;

	return ush;
}



Utils::~Utils() {
	// TODO Auto-generated destructor stub
}

