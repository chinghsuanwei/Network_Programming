/*
 * CmdHandler.cpp
 *
 *  Created on: 2011/10/27
 *      Author: Ching-Hsuan Wei
 */

#include "CmdHandler.h"

CmdHandler::CmdHandler() {
	// TODO Auto-generated constructor stub

}

void CmdHandler::dispatch(int newsockfd, char* input, map<unsigned int, FwdPipe>& fwdmap, unsigned int cmdno)
{
	char* pch;
	vector<string> vCmd;
	int outputType = NONE;
	int pipefd[2];
	int fwdpipefd[2];
	int step = 0;
	unsigned int fwdjump;

	FILE* fw;
	FILE* fr;
	string filename = "";

	forwardType = examineForwardTypeAndCut(input, &fwdjump); //ex: ls|cat|1  return FLAG_FORWARD_STDOUT & cut "|1"
	char inputcopy[strlen(input)+1];
	strcpy(inputcopy, input);
	int pipenum = caculatePipeNum(inputcopy);
#ifdef DEBUG
    cout << "ForwardType = " << (int)forwardType << endl;
	cout << "pipenum = " << pipenum << endl;
	fflush(stdout);
#endif
	pch = strtok(input, "!|\r\n");
	while(pch!=NULL)
	{
	    bool FLAG_OUTPUT_FILE = false;
	    bool FLAG_INPUT_FILE = false;
	    bool FLAG_ARGV_END = false;
#ifdef DEBUG
	    cout << "---------Step " <<  step << "---------" << endl;
#endif
		//TODO cmd or filename should not include '!' & '|'
		if(step > 0)
		{
            dup2(pipefd[0], STDIN_FILENO);
#ifdef DEBUG
            //cout << "********Parent********" << endl;
            //cout << "close pipefd[0]:" << pipefd[0] << endl;
            //cout << "close pipefd[1]:" << pipefd[1] << endl;
            //fflush(stdout);
#endif
            close(pipefd[1]);
            close(pipefd[0]);
		}
		else
		{
		    // examine forward pipe for input & free
            map<unsigned int, FwdPipe>::iterator iter;
            iter = fwdmap.find(cmdno);
            if(iter!=fwdmap.end())
            {
#ifdef DEBUG
                cout << "fwdmap find cmdno " << cmdno << endl;
                fflush(stdout);
#endif
                dup2(iter->second.rpipe, STDIN_FILENO);
                close(iter->second.rpipe);
                close(iter->second.wpipe);

                fwdmap.erase(iter);
            }
		}

		if(pipenum > 0) pipe(pipefd);
		else
		{
		    if(forwardType!=NULL)
		    {
                //examine forward pipe is exist?
                map<unsigned int, FwdPipe>::iterator iter;
                iter = fwdmap.find(cmdno+fwdjump);
                if(iter!=fwdmap.end())
                {
                    fwdpipefd[0] = iter->second.rpipe;
                    fwdpipefd[1] = iter->second.wpipe;
                }
                else
                {
#ifdef DEBUG
                    cout << "Insert FwdPipe to fwdmap, cmdno + fwdjump = " << cmdno+fwdjump << endl;
                    fflush(stdout);
#endif
                    pipe(fwdpipefd);
                    FwdPipe fwdPipe;
                    fwdPipe.rpipe = fwdpipefd[0];
                    fwdPipe.wpipe = fwdpipefd[1];
                    fwdmap.insert(pair<unsigned int, FwdPipe>(cmdno+fwdjump, fwdPipe));
                }
		    }
		}

//*******************************fork*******************************
		if(fork() > 0)
		{
		    pch = strtok(NULL, "!|\r\n");
		    pipenum--;
		    step++;
		}
		else
		{
//*******************************make Argv*******************************
		    vCmd.clear();
		    tokenCmdLine(pch, vCmd);
#ifdef DEBUG
            cout << "Param:" << endl;
            for(unsigned int ui=0; ui<vCmd.size(); ui++)
            {
                cout << vCmd[ui] << endl;
            }
            cout << endl;
#endif
            char** argv;
            argv = (char**)malloc( sizeof(char**) * (vCmd.size()+1) ); // +1 for NULL
            for(unsigned int ui=0; ui<vCmd.size(); ui++)
            {
                char* arg = (char*)malloc(sizeof(char*)*(vCmd[ui].length()+1));
                strcpy(arg, vCmd[ui].c_str());
                if(strcmp(arg, ">")==0)
                {
#ifdef DEBUG
                    cout << "detect '>' character" << endl;
#endif
                    FLAG_OUTPUT_FILE = true;
                    if(FLAG_ARGV_END!=true)
                    {
                        argv[ui] = NULL;
                        FLAG_ARGV_END = true;
                    }
                    ui++;
                    filename = vCmd[ui];
                    //TODO filename error
                    fw = fopen(filename.c_str(), "w");
                    if(fw==NULL)
                    {
                        cout << "error: filename isn't exist." << endl;
                        exit(0);
                    }
                    continue;
                }
                else if(strcmp(arg, "<")==0)
                {
#ifdef DEBUG
                    cout << "detect '<' character" << endl;
#endif
                    FLAG_INPUT_FILE = true;
                    if(FLAG_ARGV_END!=true)
                    {
                        argv[ui] = NULL;
                        FLAG_ARGV_END = true;
                    }
                    ui++;
                    filename = vCmd[ui];
                    fr = fopen(filename.c_str(), "r");
                    if(fr==NULL)
                    {
                        cout << "error: filename isn't exist." << endl;
                        exit(0);
                    }
                    continue;
                }
                argv[ui] = arg;
            }
            argv[vCmd.size()] = NULL;



            if(pipenum > 0)
            {
#ifdef DEBUG
            //cout << "********Child********" << endl;
            //cout << "close pipefd[0]:" << pipefd[0] << endl;
            //cout << "close pipefd[1]:" << pipefd[1] << endl;
#endif

                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
                close(pipefd[0]);
            }
            else
            {
                if(forwardType==NONE)
                {
                    dup2(newsockfd, STDOUT_FILENO);
                }
                else
                {
                    dup2(fwdpipefd[1], STDOUT_FILENO);
                    if(forwardType==FORWARD_STDERR) dup2(fwdpipefd[1], STDERR_FILENO);
                    close(fwdpipefd[0]);
                    close(fwdpipefd[1]);
                }
            }

            if(FLAG_OUTPUT_FILE)
            {
                dup2(fileno(fw), STDOUT_FILENO);
                fclose(fw);
            }

            if(FLAG_INPUT_FILE)
            {
                dup2(fileno(fr), STDIN_FILENO);
                fclose(fr);
            }

            if(execvp(vCmd[0].c_str(), argv)==-1)
            {
                dup2(newsockfd, STDOUT_FILENO);
                cout << "Unknown command: [" << vCmd[0] << "]." << endl;
                exit(0);
            }
		}

        int status;
        waitpid(-1, &status, 0);
	}
}

/*int CmdHandler::checkOutputType(char* input)
{
	char* pch;
	pch = strtok(input, "!");
	if(pch!=NULL) return PIPE_STDERR;
}*/

int CmdHandler::caculatePipeNum(char* input)
{
    int count = 0;
    char* pch = strtok(input, "!|\r\n");
    while(pch!=NULL)
    {
        if(atoi(pch)>0) break;
        count++;
        pch = strtok(NULL, "!|\r\n");
    }

    return count-1;
}

ForwardType CmdHandler::examineForwardTypeAndCut(char* input, unsigned int* fwdjump)
{
    int len = strlen(input);
    char* pch;
    for(int i = len-1; i>=0; i--)
    {
        if(input[i]=='|' || input[i]=='!')
        {
            char ch = input[i];
            pch = &input[i+1];
            if((*fwdjump = atoi(pch)) > 0)
            {
#ifdef DEBUG
                cout << "fwdjump = " << *fwdjump << endl;
                fflush(stdout);
#endif
                input[i] = '\0'; //cut string
                return  (ch=='|')? FORWARD_STDOUT: FORWARD_STDERR;
            }
            return NONE;
        }
    }

    return NONE;
}

void CmdHandler::tokenCmdLine(char* pch, vector<string>& vCmd)
{
	char* start;
	char* end;
	bool FLAG_TOKENING = false;
	int len = strlen(pch);
	char param[128];
	bzero(param, sizeof(param));
	for(int i=0; i<=len; i++)
	{
		if(!FLAG_TOKENING && pch[i]!=' ')
		{
			start = pch + i;
			FLAG_TOKENING = true;
		}
		else if(FLAG_TOKENING && (pch[i]==' ' || pch[i]=='\0'))
		{
			end = pch + i;
			strncpy(param, start, end - start);
			param[end-start] = '\0';

			string str = "";
			str.assign(param);
			vCmd.push_back(str);

			bzero(param, sizeof(param));
			FLAG_TOKENING = false;
		}
	}
}

CmdHandler::~CmdHandler() {
	// TODO Auto-generated destructor stub
}
