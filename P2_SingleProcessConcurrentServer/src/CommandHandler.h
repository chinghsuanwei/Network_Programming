/*
 * CommandHandler.h
 *
 *  Created on: 2011/11/13
 *      Author: Ching-Hsuan Wei
 */

#ifndef COMMANDHANDLER_H_
#define COMMANDHANDLER_H_

#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <error.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/resource.h>

#include "User.h"
#include "UserInfo.h"
#include "Utils.h"

using namespace std;

class CommandHandler {
public:
	CommandHandler();
	void dispatch(int fd, const string& sMsg, UserInfo& oUserInfo);
	void parser(vector<string>& vCmds, const string& msg);
	int calArgc(vector<string>& vCmds, int begin);
	char** makeArgv(vector<string>& vCmds, int begin, int argc);

	bool pipeOut(UserInfo& oUserInfo, int fd, bool STDERR = false);

	void tell(UserInfo& oUserInfo, int myID, vector<string>& vCmds);
	void yell(UserInfo& oUserInfo, int myID, vector<string>& vCmds);
	void name(UserInfo& oUserInfo, int myID, vector<string>& vCmds);
	void who(UserInfo& oUserInfo, int myID);
	void quit(UserInfo& oUserInfo, int myID);

	void resetPathEnv(UserInfo& oUserInfo, int myID); //just for single thread, because users share same env
	void setEnv(UserInfo& oUserInfo, int myID, vector<string>& vCmds);
	void printEnv(vector<string>& vCmds);

	bool isExit();
	~CommandHandler();
public:
	bool FLAG_EXIT;
};

#endif /* COMMANDHANDLER_H_ */
