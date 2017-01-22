/*
 * CmdHandler.h
 *
 *  Created on: 2011/10/27
 *      Author: Ching-Hsuan Wei
 */

#ifndef CMDHANDLER_H_
#define CMDHANDLER_H_
#include <vector>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <map>
#include "Utils.h"

using namespace std;

class CmdHandler {
public:
	CmdHandler();
	void dispatch(int newsockfd, char input[], map<unsigned int, FwdPipe>& fwdmap, unsigned int cmdno);
	int checkOutputType(char input[]);
	int caculatePipeNum(char input[]);
	void tokenCmdLine(char* pch, vector<string>& vCmd);
	ForwardType examineForwardTypeAndCut(char input[], unsigned int* fwdjump);
	virtual ~CmdHandler();

public:
	//vector<ForwardPackage> vForward;
	ForwardType forwardType;
};

#endif /* CMDHANDLER_H_ */
