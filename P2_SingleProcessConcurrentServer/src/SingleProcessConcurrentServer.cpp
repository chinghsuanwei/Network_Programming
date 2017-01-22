//============================================================================
// Name        : SingleProcessConcurrentServer.cpp
// Author      : Ching-Hsuan Wei
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <iostream>
#include <string>
#include <vector>

#include "User.h"
#include "UserInfo.h"
#include "CommandHandler.h"

using namespace std;

#define QLEN 5
#define BUFSIZE 1024
#define WELCOMEMESSAGE "****************************************\n** Welcome to the information server. **\n****************************************\n"


int main(int argc, char** argv)
{
	int _STDIN = 1021;
	int _STDOUT = 1022;
	int _STDERR = 1023;

	dup2(STDIN_FILENO, _STDIN);
	dup2(STDOUT_FILENO, _STDOUT);
	dup2(STDERR_FILENO, _STDERR);

	struct sockaddr_in fsin;
	int msock;
	fd_set rfds; /* read file description set */
	fd_set afds; /* active file description set */
	socklen_t alen;
	int portno;
	int fd, nfds;
	char buf[BUFSIZE];
	string sMsg = "";
	UserInfo m_oUserInfo;
	CommandHandler cmdHandler;
    chdir("./ras");

    portno = 5005;
	if(argc>1) portno = atoi(argv[1]);

	msock = socket(AF_INET, SOCK_STREAM, 0);
	if (msock < 0) cout << "ERROR opening socket";

	bzero((char *) &fsin, sizeof(fsin));
	fsin.sin_family = AF_INET;
	fsin.sin_addr.s_addr = INADDR_ANY;
	fsin.sin_port = htons(portno);
	if (bind(msock, (struct sockaddr *) &fsin,sizeof(fsin)) < 0) cout << "ERROR on binding";

	listen(msock,5);

	nfds = getdtablesize();
	FD_ZERO(&afds);
	FD_SET(msock, &afds);

    while(true)
    {
		memcpy(&rfds, &afds, sizeof(rfds));

		if(select(nfds, &rfds, (fd_set*)0, (fd_set*)0, (struct timeval*)0) < 0) cout << "ERROR on select";

		if(FD_ISSET(msock, &rfds))
		{
			int ssock;
			alen = sizeof(fsin);
			ssock = accept(msock, (struct sockaddr *)&fsin, &alen);

#ifdef DEBUG
			string info = "socket = ";
			info += Utils::int2String(ssock);
			info += '\n';
			Utils::write2Concole(info);
#endif

#ifdef DEMO
			User user = m_oUserInfo.addUser(ssock, "nctu", 5566);
#else
			char ip[16];
			inet_ntop(AF_INET, &fsin.sin_addr, ip, INET_ADDRSTRLEN);
			User user = m_oUserInfo.addUser(ssock, ip, fsin.sin_port);
#endif

			string msg = "*** User '";
			msg += user.name;
			msg += "' entered from ";
			msg += user.ip;
			msg += "/";
			msg += Utils::int2String(user.port);
			msg += ". ***";
			msg += "\n";

			send(ssock, WELCOMEMESSAGE, strlen(WELCOMEMESSAGE), 0);
/*
7. When a new client connect to the server, broadcast as follows.
   % *** User '(no name)' entered from <IP/port>. ***

   eg.
   [terminal of all clients]
   % *** User '(no name)' entered from 140.113.215.63/1013. ***
*/
			m_oUserInfo.broadcast(msg);
			send(ssock, "% ", 2, 0); // welcome message

			FD_SET(ssock, &afds);
		}

		for(fd=0; fd<nfds; ++fd)
			if(fd != msock && FD_ISSET(fd, &rfds))
			{

				int cc;
				sMsg = "";
				do{
					cc = recv(fd, buf, BUFSIZE-1, 0);
				    buf[cc] = '\0';
					if(cc < (BUFSIZE-1)) break;
					else if( buf[BUFSIZE-2]=='\n') break;
					else sMsg.append(buf);
				}while(cc > 0);

				sMsg.append(buf);
#ifdef DEBUG
				Utils::write2Concole(sMsg);
#endif

				//[who], [tell], [yell], [name]
				cmdHandler.dispatch(fd, sMsg, m_oUserInfo);
				if(cmdHandler.isExit())
				{
#ifdef DEBUG
					Utils::write2Concole("close socket\n");
#endif
					// close stdin stdout stderr that we used for pipe
					dup2(_STDIN, STDIN_FILENO);
					dup2(_STDOUT, STDOUT_FILENO);
					dup2(_STDERR, STDERR_FILENO);
					(void) close(fd);
					FD_CLR(fd, &afds);
				}

				send(fd, "% ", 2, 0);
			}


    }
}

