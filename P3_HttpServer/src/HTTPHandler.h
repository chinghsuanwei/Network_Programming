/*
 * HTTPHandler.h
 *
 *  Created on: 2011/12/16
 *      Author: Ching-Hsuan Wei
 */

#ifndef HTTPHANDLER_H_
#define HTTPHANDLER_H_

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>

using namespace std;

class HTTPHandler {
public:
	HTTPHandler();
	void dispatch(string& sMsg);
	void parser(string& sMsg);
	void setCGIEnv();
	~HTTPHandler();
public:
	string QUERY_STRING;
	string REQUEST_URI;
	string REQUEST_URI_TYPE;
};

#endif /* HTTPHANDLER_H_ */
