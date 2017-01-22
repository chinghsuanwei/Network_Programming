//============================================================================
// Name        : SOCKServer.cpp
// Author      : Ching-Hsuan Wei
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "SOCKPackage.h"
#include "Utils.h"

using namespace std;

#define DEBUG 1
#define BUFSIZE 8196
#define INT_MAX 0x7fffffff

#define CONNECT_MODE 1
#define BIND_MODE 2

#define REQUEST_GRANTED 90
#define REQUEST_FAIL 91

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, portno;
    int newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    char cli_ip[16];
    unsigned short port;


    int cc;
    char buf[BUFSIZE];
    SOCKPackage oSOCKPackage;

    portno = 3200;

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
			newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
			if (newsockfd < 0) error("ERROR on accept");
			if(fork() > 0)
			{
				close(newsockfd);
			}else{



				//VN CD PORT IP USERID NULL

				cc = recv(newsockfd, buf, BUFSIZE-1, 0);

#ifdef DEBUG
				for(int i=0; i<8; i++) cout << (int)buf[i] << endl;
#endif

				oSOCKPackage.VN = buf[0];
				oSOCKPackage.CD = buf[1];
				oSOCKPackage.port = (buf[2] << 8) + (unsigned char)buf[3];

				if(oSOCKPackage.CD == CONNECT_MODE){
					//CONNECT

					bzero((char *) &oSOCKPackage.serv_addr, sizeof(oSOCKPackage.serv_addr));
					oSOCKPackage.serv_addr.sin_family = AF_INET;
					oSOCKPackage.serv_addr.sin_addr.s_addr = (buf[7] << 24) + (buf[6] << 16) + (buf[5] << 8) + buf[4];
					oSOCKPackage.serv_addr.sin_port = htons(oSOCKPackage.port);

				    inet_ntop(AF_INET, &oSOCKPackage.serv_addr.sin_addr, oSOCKPackage.ip, INET_ADDRSTRLEN);

					for(int i=8; buf[i]!='\0'; i++) oSOCKPackage.userID += buf[i];

					cout << "VN: " << (int)oSOCKPackage.VN << ", ";
					cout << "CD: " << (int)oSOCKPackage.CD << ", ";
					cout << "DST IP: " << oSOCKPackage.ip << ", ";
					cout << "DST PORT: " << oSOCKPackage.port << ", ";
					cout << "USERID: " << oSOCKPackage.userID << ", ";
					cout << endl;

					oSOCKPackage.sock = socket(AF_INET, SOCK_STREAM, 0);
				    if (oSOCKPackage.sock < 0) error("ERROR opening socket");

	                if( connect( oSOCKPackage.sock, (struct sockaddr *)&oSOCKPackage.serv_addr,
	                    sizeof(oSOCKPackage.serv_addr)) < 0) cout << "error : connect fail" << endl;

	                cout << "SOCKS_CONNECT GRANTED......" << endl;

					//REPLY SUCCESS
					buf[0] = 0;
					buf[1] = REQUEST_GRANTED;

					//port
					buf[2] = oSOCKPackage.port >> 8;
					buf[3] = oSOCKPackage.port & 0xff;

					int ip;
					bcopy(&oSOCKPackage.serv_addr.sin_addr.s_addr, &ip , 4);

					buf[4] = ip & 0xff;
					buf[5] = (ip >> 8) & 0xff;
					buf[6] = (ip >> 16) & 0xff;
					buf[7] = (ip >> 24) & 0xff;

#ifdef DEBUG
					for(int i=0; i<8; i++) cout << (int)buf[i] << endl;
#endif

					send(newsockfd, buf, 8, 0);
					break; //exit
				}else if(oSOCKPackage.CD == BIND_MODE){
					//BIND

					bzero((char *) &oSOCKPackage.serv_addr, sizeof(oSOCKPackage.serv_addr));
					oSOCKPackage.serv_addr.sin_family = AF_INET;
					oSOCKPackage.serv_addr.sin_addr.s_addr = INADDR_ANY;
					oSOCKPackage.serv_addr.sin_port = htons(0);

					oSOCKPackage.socklen = sizeof(oSOCKPackage.serv_addr);
					getsockname(oSOCKPackage.sock, (struct sockaddr *)&oSOCKPackage.serv_addr, &oSOCKPackage.socklen);

				    inet_ntop(AF_INET, &oSOCKPackage.serv_addr.sin_addr, oSOCKPackage.ip, INET_ADDRSTRLEN);

					for(int i=8; buf[i]!='\0'; i++) oSOCKPackage.userID += buf[i];

					oSOCKPackage.sock = socket(AF_INET, SOCK_STREAM, 0);
				    if (oSOCKPackage.sock < 0) error("ERROR opening socket");

				    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
				    	//REPLY FAIL

				    	buf[0] = 0;
				    	buf[1] = REQUEST_FAIL;
				    	buf[2] = 0;
				    	buf[3] = 0;
				    	buf[4] = 0;
				    	buf[5] = 0;
				    	buf[6] = 0;
				    	buf[7] = 0;

				    	send(newsockfd, buf, 8, 0);

				    	return 0;
				    }else{
				    	//REPLY SUCCESS

				    	unsigned short usPort;
				    	bcopy(&oSOCKPackage.serv_addr.sin_port, &usPort , 2);

				    	buf[0] = 0;
				    	buf[1] = REQUEST_GRANTED;
				    	buf[2] = usPort >> 8;
				    	buf[3] = usPort & 0xff;
				    	buf[4] = 0;
				    	buf[5] = 0;
				    	buf[6] = 0;
				    	buf[7] = 0;

				    	send(newsockfd, buf, 8, 0);

						listen(oSOCKPackage.sock, 1);

						accept(oSOCKPackage.sock, (struct sockaddr *)&oSOCKPackage.serv_addr, &oSOCKPackage.socklen);

						cout << "SOCKS_BIND GRANTED......" << endl;

						//SOCKS4_REPLY
				    	buf[0] = 0;
				    	buf[1] = REQUEST_GRANTED;
				    	buf[2] = 0;
				    	buf[3] = 0;
				    	buf[4] = 0;
				    	buf[5] = 0;
				    	buf[6] = 0;
				    	buf[7] = 0;

						send(newsockfd, buf, 8, 0);
						break; //exit*/
				    }
				}


			}
    }

	fd_set rfds; /* read file description set */
	fd_set afds; /* active file description set */
	int nfds;

	nfds = getdtablesize();
	FD_ZERO(&afds);
	FD_SET(newsockfd, &afds);
	FD_SET(oSOCKPackage.sock, &afds);
	string sMsg;

    cc = INT_MAX; //init

    while(cc > 0)
    {
    	memcpy(&rfds, &afds, sizeof(rfds));

    	if(select(nfds, &rfds, (fd_set*)0, (fd_set*)0, (struct timeval*)0) < 0)
    			cout << "ERROR on select";

    	if(FD_ISSET(newsockfd, &rfds)){
    		cc = recv(newsockfd, buf, BUFSIZE-1, 0);
    		if(cc == 0) return 0;

    		send(oSOCKPackage.sock, buf, cc, 0);
    	}

    	if(FD_ISSET(oSOCKPackage.sock, &rfds)){
    		cc = recv(oSOCKPackage.sock, buf, BUFSIZE-1, 0);
    		if(cc == 0) return 0;

    		send(newsockfd, buf, cc, 0);
    	}
    }


	return 0;
}
