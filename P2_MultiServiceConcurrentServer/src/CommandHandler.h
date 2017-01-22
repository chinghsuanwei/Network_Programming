/*
 * CommandHandler.h
 *
 *  Created on: 2011/11/20
 *      Author: Ching-Hsuan Wei
 */

#ifndef COMMANDHANDLER_H_
#define COMMANDHANDLER_H_

#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <error.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <map>

#include "User.h"
#include "UserInfo.h"
#include "Utils.h"

using namespace std;

class CommandHandler {
public:
	CommandHandler(int _myID, const char* _myname, int _fd);
	void dispatch(string& sMsg, UserInfo* pUserInfo, map<unsigned int, struct ForwardPipe>& mForwardPipe);
	void parser(vector<string>& vCmds, string& msg);
	char** makeArgv(vector<string>& vCmds, int begin, int argc);
	int calArgc(vector<string>& vCmds, int begin);
	void quit(UserInfo* pUserInfo);
	void who(UserInfo* pUserInfo);
	void yell(UserInfo* pUserInfo, vector<string>& vCmds);
	void tell(UserInfo* pUserInfo, vector<string>& vCmds);
	void name(UserInfo* pUserInfo, vector<string>& vCmds);
	void setEnv(vector<string>& vCmds);
	void printEnv(vector<string>& vCmds);
	bool pipeOut(UserInfo* pUserInfo, bool STDERR = false);
	void speak(const char* str);
	void speak(const string& str);
	bool isExit();
	~CommandHandler();
public:
	int fd;
	int myID;
	char myname[16];
	bool FLAG_EXIT;
	unsigned int m_iCmdno;
};

#endif /* COMMANDHANDLER_H_ */
