/*
 * User.cpp
 *
 *  Created on: 2011/11/19
 *      Author: Ching-Hsuan Wei
 */

#include "User.h"

User::User()
{
	// TODO Auto-generated constructor stub
}

User::User(int _ID)
{
	ID = _ID;
}

void User::init(int N)
{
	ID = -1;
	pid = -1;
	bzero(name, sizeof(name));
	bzero(ip, sizeof(ip));
	port = -1;
	bzero(msg, sizeof(msg));
	bzero(fifo, sizeof(fifo));
	char str[16];
	sprintf(str, "../fifo/%d.fifo", N);
	strcpy(fifo, str);
	FLAG_PIPE_OCCUPIED = false;
}

bool User::notUsed()
{
	if(ID < 0) return true;
	else return false;
}

bool User::isUser()
{
	if(ID < 0) return false;
	else return true;
}

User::~User() {
	// TODO Auto-generated destructor stub
}

