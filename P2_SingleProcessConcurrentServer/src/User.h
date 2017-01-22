/*
 * User.h
 *
 *  Created on: 2011/11/13
 *      Author: Ching-Hsuan Wei
 */

#ifndef USER_H_
#define USER_H_

#include <string>
#include <string.h>
#include <iostream>
#include <map>

#include "Utils.h"

using namespace std;

class User {
public:
	User();
	User(int _ID);
	bool NotUser();
	~User();
public:
	int ID;
	char ip[16];
	int port;
	string name;
	int fd;
	int fdpipe[2];
	map<unsigned int, struct ForwardPipe> mForwardPipe;
    int cmdno;
    string pathEnv;
	bool FLAG_PIPE_OCCUPIED;
};

#endif /* USER_H_ */
