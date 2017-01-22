/*
 * CommandHandler.cpp
 *
 *  Created on: 2011/11/20
 *      Author: Ching-Hsuan Wei
 */

#include "CommandHandler.h"

CommandHandler::CommandHandler(int _myID, const char* _myname, int _fd)
{
	// TODO Auto-generated constructor stub
	FLAG_EXIT = false;
	myID = _myID;
	fd = _fd;
	m_iCmdno = 0;
	strcpy(myname, _myname);
}

void CommandHandler::dispatch(string& sMsg, UserInfo* pUserInfo, map<unsigned int, struct ForwardPipe>& mForwardPipe)
{
	vector<string> vCmds;
	dup2(fd, STDIN_FILENO); //always reset STDIN
	dup2(fd, STDOUT_FILENO); //always reset STDOUT
	dup2(fd, STDERR_FILENO); //always reset STDERR
	parser(vCmds, sMsg);


	if(vCmds.empty()) return;
	else if(vCmds[0] == "exit") quit(pUserInfo);
	else if(vCmds[0] == "who") who(pUserInfo);
	else if(vCmds[0] == "tell") tell(pUserInfo, vCmds); // tell <ID> <message>
	else if(vCmds[0] == "yell") yell(pUserInfo, vCmds);
	else if(vCmds[0] == "name") name(pUserInfo, vCmds);
	else if(vCmds[0] == "setenv") setEnv(vCmds);
	else if(vCmds[0] == "printenv") printEnv(vCmds);
	else{
		//ls -S bin|cat|cat|cat|number > ls.txt
		//ls -S bin|cat|cat|cat|number >|
		//cat <7 >|
		string sCmd;
		int fdpipe[2];
		char** argv;
		bool FLAG_RECV_PIPE = false;
		bool FLAG_SEND_PIPE = false;
		//bool FLAG_EXEC_FAILURE = false;
		int senderID;
		int begin;
		int argc;
		for(unsigned int index=0; index<vCmds.size(); index++)
		{
			dup2(fd, STDOUT_FILENO); //always reset STDOUT
			dup2(fd, STDERR_FILENO); //always reset STDERR
			sCmd = vCmds[index];
			begin = index;
			argc = calArgc(vCmds, begin);

			//not first command
			if(index==0)
			{
				map<unsigned int, struct ForwardPipe>::iterator iter = mForwardPipe.find(m_iCmdno);
				if(iter!=mForwardPipe.end())
				{
					dup2(iter->second.rpipe, STDIN_FILENO);
					close(iter->second.rpipe);
					close(iter->second.wpipe);
					mForwardPipe.erase(iter);
				}
			}
			else
			{
				dup2(fdpipe[0], STDIN_FILENO);
				close(fdpipe[0]);
			}

			while(index<vCmds.size())
			{
				if(vCmds[index].at(0)=='|' || vCmds[index].at(0)=='!')//pipe
				{
					if(vCmds[index].size()==1){
						pipe(fdpipe);

						//dup & close wpipe of current pipe
						dup2(fdpipe[1], STDOUT_FILENO);
						close(fdpipe[1]);
						break;
					}
					else{
						int iJump;
						string sJump = vCmds[index].substr(1);
						iJump = Utils::string2Int(sJump);

						map<unsigned int, struct ForwardPipe>::iterator iter = mForwardPipe.find(m_iCmdno + iJump);
						if(iter==mForwardPipe.end()){
							int relayPipe[2];
							struct ForwardPipe fwdpipe;
							pipe(relayPipe);
							fwdpipe.rpipe = relayPipe[0];
							fwdpipe.wpipe = relayPipe[1];
							mForwardPipe.insert(pair<int, struct ForwardPipe>(iJump + m_iCmdno, fwdpipe));

							dup2(fwdpipe.wpipe, STDOUT_FILENO);
							if(vCmds[index].at(0)=='!') dup2(fwdpipe.wpipe, STDERR_FILENO);
						}else{
							dup2(iter->second.wpipe, STDOUT_FILENO);
							if(vCmds[index].at(0)=='!') dup2(iter->second.wpipe, STDERR_FILENO);
						}
					}

				}
				else if(vCmds[index]==">|")
				{
					bool result = pipeOut(pUserInfo);
					if(result) FLAG_SEND_PIPE = true;
					else return;
				}
				else if(vCmds[index]==">!")
				{
					bool FLAG_STDERR = true;
					bool result = pipeOut(pUserInfo, FLAG_STDERR);
					if(result) FLAG_SEND_PIPE = true;
					else return;
				}
				else if(vCmds[index]==">")//write to files
				{
					index++;
					int fileno = open(vCmds[index].c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);

				    if (fileno < 0) {
				      perror("open error");
				      exit(1);
				    }

					dup2(fileno, STDOUT_FILENO);
					close(fileno);
					break;
				}
				else if(vCmds[index].at(0)=='<') //take from other ones pipe
				{
					string sReadFrom = vCmds[index].substr(1);
					senderID = Utils::string2Int(sReadFrom); //substr() cut the character '<'
					if(senderID < 0){
						string msg = "*** Error: syntax error '<N' ***\n";
						speak(msg);
						return;
					}

					if( !pUserInfo->isUserPipeOccupied(senderID) ){
						string msg = "*** Error: the pipe from #";
						msg += Utils::int2String(senderID);
						msg += " does not exist yet. ***\n";
						speak(msg);
						return;
					}

					char fifoname[16];
					pUserInfo->getUserFIFONameByID(senderID, fifoname);
					int fifo = open(fifoname, O_RDONLY);

				    if (fifo < 0) {
				      perror("open error");
				      exit(1);
				    }

					dup2(fifo, STDIN_FILENO);
					close(fifo);
					pUserInfo->setUserFlagPipeOccupiedByID(senderID, false);
					FLAG_RECV_PIPE = true;
				}

				index++;
			}

			if(fork() > 0)
			{
				int status;
				waitpid(-1, &status, 0);
			}
			else
			{
				argv = makeArgv(vCmds, begin, argc);
				//if(execlp(sCmd.c_str(), sCmd.c_str(), 0)==-1)
				if(execvp(sCmd.c_str(), argv)==-1)
				{
					string msg = "Unknown command: [";
					msg += sCmd;
					msg += "].\n";

					speak(msg);
					exit(EXIT_FAILURE);
				}
			}
		}
		/*
		7. When a client receive message from other client's pipe and then redirect the stdout into its pipe using ">|",
		   both messages are to be boardcast to all clients. For example:

		   eg. Assume my name is 'IamUser' and client id = 3.
			   Assume there is another client named 'student7' with client id = 7.
		   % cat <7 >|
		   % *** IamUser (#3) just received the pipe from student7 (#7) by 'cat <7 >|' ***
		   % *** IamUser (#3) just piped 'cat <7 >|' into his/her pipe. ***
		*/

		if(FLAG_RECV_PIPE){
			string msg;

			char senderName[16];
			pUserInfo->getUserNameByID(senderID, senderName);

			msg = "*** ";
			msg += myname;
			msg += " (#";
			msg += Utils::int2String(myID);
			msg += ") just received the pipe from ";
			msg += senderName;
			msg += " (#";
			msg += Utils::int2String(senderID);
			msg += ") by '";
			msg += sMsg.substr(0, sMsg.size()-2); // cut '/r/n'
			msg += "' ***\n";
			pUserInfo->broadcast(msg);
		}

		if(FLAG_SEND_PIPE){
			string msg = "*** ";
			msg += myname;
			msg += " (#";
			msg += Utils::int2String(myID);
			msg += ") just piped '";
			msg += sMsg.substr(0, sMsg.size()-2); // cut '/r/n'
			msg += "' into his/her pipe. ***\n";
			pUserInfo->broadcast(msg);
		}

	}
}

char** CommandHandler::makeArgv(vector<string>& vCmds, int begin, int argc)
{
	int len = argc+1; //one for NULL
	char** argv = (char**)malloc(sizeof(char)*len);


	for(int i=begin, v=0; v<argc; i++, v++)
	{
		int size = vCmds[i].size()+1;
		char* arg = (char*)malloc(sizeof(char)*size);
		strcpy( arg, vCmds[i].c_str() );
		argv[v] = arg;
	}
	argv[len-1] = NULL;
	return argv;

}

int CommandHandler::calArgc(vector<string>& vCmds, int begin)
{
	int argc = 0;
	for(unsigned int i=begin; i < vCmds.size()  ;i++)
	{
		if(vCmds[i].at(0)=='|' || vCmds[i].at(0)=='!' || vCmds[i]==">" || vCmds[i].at(0)=='<' || vCmds[i]==">|" || vCmds[i]==">!") break;
		else argc++;
	}
	if(argc < 1) speak( "*** Error: argc < 1 ***\n");
	return argc;
}

void CommandHandler::parser(vector<string>& vCmds, string& msg)
{
	size_t begin = 0;
	size_t end = 0;
	vCmds.clear();

	begin = msg.find_first_not_of(" \r\n", end);
	end = msg.find_first_of(" \r\n", begin);
	if(begin!=string::npos) vCmds.push_back( msg.substr(begin, end-begin) );
	else return;

	if(vCmds[0]=="tell"){
		begin = msg.find_first_not_of(" \r\n", end);
		end = msg.find_first_of(" \r\n", begin);
		if(begin!=string::npos) vCmds.push_back( msg.substr(begin, end-begin) ); //<ID>
		else return;

		begin = msg.find_first_not_of(" \r\n", end);
		end = msg.find_first_of("\r\n", begin);
		if(begin!=string::npos) vCmds.push_back( msg.substr(begin, end-begin) ); //<message>
		else return;
	}
	else if(vCmds[0]=="yell"){
		begin = msg.find_first_not_of(" \r\n", end);
		end = msg.find_first_of("\r\n", begin);
		if(begin!=string::npos) vCmds.push_back( msg.substr(begin, end-begin)); //<message>
		else return;
	}
	else{
		begin = msg.find_first_not_of(" \r\n", end);
		end = msg.find_first_of(" \r\n", begin);

		while(begin!=string::npos)
		{
			vCmds.push_back( msg.substr(begin, end-begin) );
			begin = msg.find_first_not_of(" \r\n", end);
			end = msg.find_first_of(" \r\n", begin);
		}
	}

}

void CommandHandler::quit(UserInfo* pUserInfo)
{
/*
8. When a client disconnect from the server, broadcast as follows.
   % *** User '<name>' left. ***

   eg.
   [terminal of all clients]
   % *** User 'student5' left. ***
*/
	pUserInfo->delUser(myID);

	string msg = "*** User '";
	msg += myname;
	msg += "' left. ***";
	msg += "\n";

	pUserInfo->broadcast(msg);
	FLAG_EXIT = true;
	// close stdout stderr that we used for pipe
}

void CommandHandler::who(UserInfo* pUserInfo)
{
/*
	6. The output format of [who]:
	You have to print a tab between each of tags. Notice that the first column do not print socket fd but client id.

	<ID>[Tab]<nickname>[Tab]<IP/port>[Tab]<indicate me>
	<1st id>[Tab]<1st name>[Tab]<1st IP/port>([Tab]<is me?>)
	<2nd id>[Tab]<2nd name>[Tab]<2nd IP/port>([Tab]<is me?>)
	<3rd id>[Tab]<3rd name>[Tab]<3rd IP/port>([Tab]<is me?>)
	...

	For example:
	% who
	<sockfd>	<nickname>	<IP/port>	<indicate me>
	1	IamStudent	140.113.215.62/1201	<-me
	2	(No Name)	140.113.215.63/1013
	3	student3	140.113.215.64/1302
*/
	string msg;
	msg += pUserInfo->getUserListString(myID); //pass myID for indicating me

	cout << msg;
	fflush(stdout);
}

void CommandHandler::yell(UserInfo* pUserInfo, vector<string>& vCmds)
{
/*
4. Format of command [yell]:
   % yell <message>
   And all the clients will get the message with following format:
   % *** <sender's name> yelled ***: <message>

   eg. Assume my name is 'IamUser'.
   [terminal of mine]
   % yell Hi...

   [terminal of all clients]
   % *** IamUser yelled ***:  Hi...
*/

	if(vCmds.size() < 2)
	{
		speak("*** Error: syntax error 'yell [SAYING]' ***\n");
		return;
	}


	string msg = "*** ";
	msg += myname;
	msg += " yelled ***: ";
	msg += vCmds[1];
	msg += "\n";

	pUserInfo->broadcast(msg);
}

void CommandHandler::tell(UserInfo* pUserInfo, vector<string>& vCmds)
{
/*
   Format of command [tell]:
   % tell <client id> <message>
   And the client will get the message with following format:
   % *** <sender's name> told you ***: <message>

   eg. Assume my name is 'IamUser'.
   [terminal of mine]
   % tell 3 Hello.

   [terminal of client id 3]
   % *** IamUser told you ***:  Hello.

   If the client you want to send message to is not exist, print the following message:
   % *** Error: user #<client id> does not exist yet. ***
*/
	if(vCmds.size() < 3)
	{
		speak("*** Error: syntax error 'tell [ID] [SAYING]' ***\n");
		return;
	}

	int yourID = Utils::string2Int(vCmds[1]);
	if(!pUserInfo->hasUser(yourID))
	{
		string msg = "*** Error: user #";
		msg += Utils::int2String(yourID);
		msg += " does not exist yet. ***\n";

		speak(msg);
		return;
	}
	else
	{
		string msg = "*** ";
		msg +=  myname ;
		msg += " told you ***: ";
		msg += vCmds[2];
		msg += "\n";

		pUserInfo->send(yourID, msg);
	}
}
void CommandHandler::name(UserInfo* pUserInfo, vector<string>& vCmds)
{
/*
5. Format of command [name]:
   % name <name>
   And all the clients will get the message with following format:
   % *** User from <IP/port> is named '<name>'. ***

   eg.
   [terminal of mine]
   % name IamUser

   [terminal of all clients]
   % *** User from 140.113.215.62/1201 is named 'IamUser'. ***
*/

	if(vCmds.size() < 2)
	{
		speak("*** Error: syntax error 'name [NAME]' ***\n");
		return;
	}

	pUserInfo->setUserNameByID(myID, vCmds[1]); /*Now, name of user is changed, but oUserMe is old name */
	strcpy(myname, vCmds[1].c_str());

	char ip[16];
	pUserInfo->getUserIPByID(myID, ip);

	string msg = "*** User from ";
	msg += ip;
	msg += "/";
	msg += Utils::int2String( pUserInfo->getUserPortByID(myID) );
	msg += " is named '";
	msg += myname;
	msg += "'. ***";
	msg += "\n";

	pUserInfo->broadcast(msg);
}

void CommandHandler::setEnv(vector<string>& vCmds)
{
	if(vCmds.size() < 3)
	{
		speak("*** Error: syntax error 'setenv [ENVNAME] [VALUE]' ***\n");
		return;
	}
	setenv(vCmds[1].c_str(), vCmds[2].c_str(), 1);
}

void CommandHandler::printEnv(vector<string>& vCmds)
{
	if(vCmds.size() < 2)
	{
		speak("*** Error: syntax error 'printenv [ENVNAME]' ***\n");
		return;
	}

	string msg = vCmds[1];
	msg += "=";
	msg += getenv(vCmds[1].c_str());
	msg += '\n';
	speak(msg);
}

bool CommandHandler::pipeOut(UserInfo* pUserInfo, bool STDERR)
{
	if( pUserInfo->isUserPipeOccupied(myID) ){
		speak("*** Error: your pipe already exists. ***\n");
		return false;
	}
	else
	{
		pUserInfo->setUserFlagPipeOccupiedByID(myID, true);

		char fifoname[16];
		pUserInfo->getUserFIFONameByID(myID, fifoname);
	    int fifo = open(fifoname,  O_WRONLY | O_CREAT| O_TRUNC, 0666);

	    dup2(fifo ,STDOUT_FILENO);
		if(STDERR == true) dup2(fifo, STDERR_FILENO);
		close(fifo);
		return true;
	}
}

bool CommandHandler::isExit()
{
	if(FLAG_EXIT){
		FLAG_EXIT = false;
		return true;
	}else return false;
}

void CommandHandler::speak(const char* str)
{
	send(fd, str, strlen(str), 0);
}

void CommandHandler::speak(const string& str)
{
	send(fd, str.c_str(), str.size(), 0);
}

CommandHandler::~CommandHandler() {
	// TODO Auto-generated destructor stub
}

