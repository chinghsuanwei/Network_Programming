/*
 * SOCKPackage.h
 *
 *  Created on: 2012/1/1
 *      Author: Ching-Hsuan Wei
 */

#ifndef SOCKPACKAGE_H_
#define SOCKPACKAGE_H_

#include <string>
#include <netinet/in.h>

using namespace std;

class SOCKPackage {
public:
	SOCKPackage();
	~SOCKPackage();
public:
	int sock;
	unsigned char VN;
	unsigned char CD;
	unsigned short port;
	struct sockaddr_in serv_addr;
	socklen_t socklen;
	char ip[16];
	string userID;
	string domainName;
};

#endif /* SOCKPACKAGE_H_ */
