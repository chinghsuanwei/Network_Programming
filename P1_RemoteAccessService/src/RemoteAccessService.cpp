//============================================================================
// Name        : RemoteAccessService.cpp
// Author      : Ching-Hsuan Wei
// Version     :
// Copyright   :
// Description : I'm handsome man
//============================================================================

#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <map>

#include "CmdHandler.h"
#include "Utils.h"

using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void reaper(int temp){
	int status;
	while(wait3(&status,WNOHANG,NULL) >= 0);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno;
    bool FLAG_EXIT = false;
    pid_t pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    portno = 7009;
    switch(argc)
    {
        case 1: break;
        case 2:
            portno = atoi(argv[1]);
            break;
        default: break;
    }

    signal(SIGCHLD, reaper);

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
         if((pid = fork()) > 0)
         {
             close(newsockfd);
         }
         else
         {
             break;
         }
    }

#ifdef DEBUG
    cout << "sockfd = " << sockfd << endl;
    cout << "newsockfd = " << newsockfd << endl;
#endif
    //******************Child******************
    close(sockfd);
#ifdef SOCKET_COMMUTICATE
    dup2(newsockfd, STDOUT_FILENO);
    dup2(newsockfd, STDERR_FILENO);
#endif
    //Set enviorment
    chdir("./ras");
    char buf[BUFSIZE];
    char input[INPUTSIZE];
    setenv("PATH", "bin:.", 1);
#ifdef DEBUG
    //cout << getenv("PATH") << endl;
    //fflush(stdout);
#endif

    CmdHandler cmdHandler;
    unsigned int cmdno = 0;
    map<unsigned int, FwdPipe> fwdmap;

    //Welcome Message
    bzero(buf, BUFSIZE);
    strcpy(buf, "****************************************\n** Welcome to the information server. **\n****************************************\n");
    send(newsockfd, buf, strlen(buf), 0);
    while(FLAG_EXIT==false)
    {
        bzero(buf, BUFSIZE);
        strcpy(buf, "% ");
        send(newsockfd, buf, strlen(buf), 0);
        bzero(buf, BUFSIZE);
        bzero(input, INPUTSIZE);
        int n = 0 ;
        while (true)
        {
          n = read(newsockfd, buf, BUFSIZE-1);
          buf[n] = '\0';
          strcat( input, buf ) ;
          if ( buf[n-1] =='\n') break ;
        }
#ifdef DEBUG
        cout << input << endl;
#endif
        //TODO if user add something after string "exit", system will exit
        if(strncmp(input, "exit", 4)==0)
        {
            FLAG_EXIT = true;
            close(newsockfd);
        }
        else if(strncmp(input, "setenv", 6)==0)
        {
            strtok(buf, " \r\n"); //cut "setenv"
            char* env = strtok(NULL, " \r\n");
            char * value = strtok(NULL, " \r\n");
            if(env == NULL || value == NULL) cout << "error: setenv [ENV] [VALUE] ." << endl;
            else setenv(env, value, 1);
        }
        else if(strncmp(input, "printenv", 6)==0)
        {
            strtok(input, " \r\n"); //cut "printenv"
            char* env = strtok(NULL, " \t\r\n");
            if(env == NULL) cout << "error: printenv [ENV] ." << endl;
            else cout << getenv(env) << endl;
        }
        else
        {
            cmdHandler.dispatch(newsockfd, input, fwdmap, cmdno);
            cmdno++;
            //execlp("ls", "ls", NULL);
        }
    }
    return 0;
}
