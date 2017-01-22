/*
 * UserInfo.h
 *
 *  Created on: 2011/11/13
 *      Author: Ching-Hsuan Wei
 */

#ifndef USERINFO_H_
#define USERINFO_H_

#include <map>
#include <string>
#include <iostream>
#include <sys/socket.h>

#include "User.h"
#include "Utils.h"

using namespace std;

class UserInfo {
public:
    UserInfo();
	User addUser(int fd, const char ip[], int port);
	void delUser(int ID);
	bool hasUser(int ID);

	User getUserByID(int ID);
	User getUserByFD(int fd);
	int getUserIDByFD(int fd);
	string getUserNameByID(int ID);
	int getUserFDByID(int ID);
	void getUserIPByID(int ID, char _ip[]);
	int getUserPortByID(int ID);
	string getUserPathEnvByID(int ID);
	int getUserRPipeByID(int ID);
	void setUserPathEnvByID(int ID, const string& pathEnv);
	map<unsigned int, struct ForwardPipe>* getUserForwardPipePointerByID(int ID);
	int increaseUserCommandNo(int ID);

	bool isUserPipeOccupied(int ID);
	void setFlagPipeOccupied(int ID, bool flag);
	void setUserPipeByID(int ID, int fdpipe[2]);
	void setUserNameByID(int ID, const string& name);
	string getUserListStringByID(int myID);
	void broadcast(string& msg);
	~UserInfo();
public:
	map<int, User> m_mUser;
};

#endif /* USERINFO_H_ */
