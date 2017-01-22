/*
 * User.h
 *
 *  Created on: 2011/11/19
 *      Author: Ching-Hsuan Wei
 */

#ifndef USER_H_
#define USER_H_

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

class User {
public:
	User();
	User(int _ID);
	void init(int N);
	bool notUsed();
	bool isUser();
	~User();
public:
	int ID;
	pid_t pid;
	char name[30];
	char ip[16];
	int port;
	char msg[1024];
	char fifo[20];
	bool FLAG_PIPE_OCCUPIED;
};

#endif /* USER_H_ */
