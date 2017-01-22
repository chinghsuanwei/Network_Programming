/*
 * User.cpp
 *
 *  Created on: 2011/11/13
 *      Author: Ching-Hsuan Wei
 */

#include "User.h"

User::User()
{
	FLAG_PIPE_OCCUPIED = false;
	name = "(no name)";
	pathEnv = "bin:.";
	cmdno = 0;
}

User::User(int _ID) {

	ID = _ID;
}

bool User::NotUser()
{
	if(this->ID<0) return true;
	else return false;
}

User::~User() {

}

