//============================================================================
// Name        : MultiServiceConcurrentServer.cpp
// Author      : Ching-Hsuan Wei
// Version     :
// Copyright   : Your copyright notice
// Description : If you change the directory
//				please notice where "chdir" & "fifo" is right
//============================================================================

#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "CommandHandler.h"
#include "User.h"
#include "UserInfo.h"
#include "Utils.h"

#define SHMKEY 5677
#define BUFSIZE 1024
#define WELCOMEMESSAGE "****************************************\n** Welcome to the information server. **\n****************************************\n"
using namespace std;

UserInfo* pUserInfo;
int myID;
int newsockfd;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void reaper(int temp){
	int status;
	while(wait3(&status, WNOHANG, NULL) > 0);
}

void recvMsg(int temp)
{
	char msg[1024];
	pUserInfo->getUserMessageByID(myID, msg);
	send(newsockfd, msg, strlen(msg), 0);
#ifdef DEBUG
	Utils::write2Concole("myID = " + Utils::int2String(myID) + "recv!\n");
#endif
}

int main(int argc, char *argv[])
{
#ifdef DEBUG
	Utils::DebugOn();
#endif
	//***************** shared memory *****************
    int shmid;
    void *shm;

    /*
     * Create the segment.
     */
    if ((shmid = shmget(SHMKEY, sizeof(UserInfo), IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    /*
     * Now we attach the segment to our data space.
     */
    if ((shm = shmat(shmid, NULL, 0)) == (void *) -1) {
        perror("shmat");
        exit(1);
    }

    pUserInfo = (UserInfo*)shm;
    pUserInfo->init();

    //***************** end shared memory *****************

    int sockfd, portno;
    socklen_t clilen;
    pid_t pid;
    struct sockaddr_in serv_addr, cli_addr;

    portno = 7004;
    switch(argc)
    {
        case 1: break;
        case 2:
            portno = atoi(argv[1]);
            break;
        default: break;
    }

    signal(SIGCHLD, reaper);
    signal(SIGUSR1, recvMsg);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
            error("ERROR on binding");

    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    while(true)
    {
			newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
			if (newsockfd < 0) error("ERROR on accept");
			if(fork() > 0)
			{
				close(newsockfd);
			}
			else
			{
				pid = getpid();
				char ip[16];
				inet_ntop(AF_INET, &cli_addr.sin_addr, ip, INET_ADDRSTRLEN);
				int port = cli_addr.sin_port;
				User user = pUserInfo->addUser(pid, ip, port);

				myID = user.ID;
				if(!user.isUser()) exit(0);
/*
7. When a new client connect to the server, broadcast as follows.
   % *** User '(no name)' entered from <IP/port>. ***

   eg.
   [terminal of all clients]
   % *** User '(no name)' entered from 140.113.215.63/1013. ***
*/
				string msg = "*** User '";
				msg += user.name;
				msg += "' entered from ";
				msg += user.ip;
				msg += "/";
				msg += Utils::int2String(user.port);
				msg += ". ***";
				msg += "\n";

#ifdef DEBUG
    cout << "sockfd = " << sockfd << endl;
    cout << "newsockfd = " << newsockfd << endl;
#endif

			    close(sockfd);
			    dup2(newsockfd, STDOUT_FILENO);
			    dup2(newsockfd, STDERR_FILENO);

			    send(newsockfd, WELCOMEMESSAGE, strlen(WELCOMEMESSAGE), 0);
				pUserInfo->broadcast(msg);
				break;
			}
    }



    //******************Fork Server******************
    int cc;
    string sMsg = "";
    char buf[BUFSIZE];
    CommandHandler cmdHandler(myID, "(no name)", newsockfd);
    map<unsigned int, struct ForwardPipe> mForwardPipe;

    chdir("./ras");
    setenv("PATH", "bin:.", 1);

    while(true)
    {
    	send(newsockfd, "% ", 2, 0);
    	sMsg = "";

		do{
			cc = recv(newsockfd, buf, BUFSIZE-1, 0);
			buf[cc] = '\0';
			if(cc < (BUFSIZE-1)) break;
			else if( buf[BUFSIZE-2]=='\n') break;
			else sMsg.append(buf);
		}while(cc>0);

		sMsg.append(buf);

#ifdef DEBUG
				Utils::write2Concole(sMsg);
#endif

		cmdHandler.dispatch(sMsg, pUserInfo, mForwardPipe);
		if(cmdHandler.isExit())
		{
#ifdef DEBUG
					Utils::write2Concole("close socket\n");
#endif
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);
			close(newsockfd);
			break;
		}

		cmdHandler.m_iCmdno++;
    }

    if (shmdt(shm) == -1) {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

