/*
 * UserInfo.h
 *
 *  Created on: 2011/11/19
 *      Author: Ching-Hsuan Wei
 */

#ifndef USERINFO_H_
#define USERINFO_H_

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "User.h"
#include "Utils.h"

using namespace std;

class UserInfo {
public:
	UserInfo();
	void init();
	User addUser(pid_t pid, char ip[], int port);
	void delUser(int ID);
	void setUserNameByID(int ID, const string& name);
	void setUserFlagPipeOccupiedByID(int ID, bool flag);
	void getUserNameByID(int ID, char _name[]);
	void getUserFIFONameByID(int ID, char _fifoname[]);
	void getUserIPByID(int ID, char _ip[]);
	void getUserMessageByID(int ID, char _msg[]);
	int getUserPortByID(int ID);
	bool hasUser(int ID);
	bool isUserPipeOccupied(int ID);

	void send(int ID, string& str);
	void broadcast(string& msg);
	string getUserListString(int myID);
	~UserInfo();
private:
	User m_aUser[31];
};

#endif /* USERINFO_H_ */
