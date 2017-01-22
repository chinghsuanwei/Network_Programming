/*
 * Utils.cpp
 *
 *  Created on: 2011/11/20
 *      Author: Ching-Hsuan Wei
 */

#include "Utils.h"

Utils::Utils() {
	// TODO Auto-generated constructor stub

}

string Utils::int2String(int integer)
{
	stringstream ss;
	string str;

	ss << integer;
	ss >> str;

	return str;
}

int Utils::string2Int(string str)
{
	stringstream ss;
	int integer;

	ss << str;
	ss >> integer;

	return integer;
}

void Utils::DebugOn()
{
	dup2(STDOUT_FILENO, 1023);
}

void Utils::write2Concole(const string& info)
{
	write(1023, info.c_str(), info.size());
}

void Utils::write2Concole(const char info[])
{
	write(1023, info, strlen(info));
}

Utils::~Utils() {
	// TODO Auto-generated destructor stub
}

