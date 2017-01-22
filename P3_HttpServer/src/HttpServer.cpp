//============================================================================
// Name        : HttpServer.cpp
// Author      : Ching-Shung Wei
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "HTTPHandler.h"


#define BUFSIZE 1024

using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main() {
	int sockfd;
	int newsockfd;
	int portno;
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;

	portno = 8080;

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


		if(fork() > 0){
			close(newsockfd);
		}
		else{
			HTTPHandler httpHandler;
			string sMsg = "";
			char buf[BUFSIZE];
			int cc;
			do{
				cc = recv(newsockfd, buf, BUFSIZE-1, 0);
				buf[cc] = '\0';
				if(cc < (BUFSIZE-1)) break;
				else sMsg.append(buf);
			}while(cc>0);

			sMsg.append(buf);
			cout << buf;

			chdir("./public_html");
			dup2(newsockfd, STDOUT_FILENO);

			httpHandler.dispatch(sMsg);
			exit(0);
		}

	}
	return 0;
}
