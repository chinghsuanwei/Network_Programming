/*
 * CommandHandler.cpp
 *
 *  Created on: 2011/11/13
 *      Author: Ching-Hsuan Wei
 */

#include "CommandHandler.h"

CommandHandler::CommandHandler() {
	// TODO Auto-generated constructor stub
	FLAG_EXIT = false;
}

void CommandHandler::dispatch(int fd, const string& sMsg, UserInfo& oUserInfo)
{
	vector<string> vCmds;
	int myID = oUserInfo.getUserIDByFD(fd);
	resetPathEnv(oUserInfo, myID);
	map<unsigned int, struct ForwardPipe>* pForwardPipe = oUserInfo.getUserForwardPipePointerByID(myID);
	int cmdno = oUserInfo.increaseUserCommandNo(myID);

	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO); //always reset STDOUT
	dup2(fd, STDERR_FILENO); //always reset STDERR
	//[who], [tell], [yell], [name]
	//cat <5
	//ls >|
	//cat <7 >|
	parser(vCmds, sMsg);

	if(vCmds.empty()) return;
	else if(vCmds[0] == "exit") quit(oUserInfo, myID);
	else if(vCmds[0] == "who") who(oUserInfo, myID);
	else if(vCmds[0] == "tell") tell(oUserInfo, myID, vCmds); // tell <ID> <message>
	else if(vCmds[0] == "yell") yell(oUserInfo, myID, vCmds); // yell <message>
	else if(vCmds[0] == "name") name(oUserInfo, myID, vCmds);
	else if(vCmds[0] == "setenv") setEnv(oUserInfo, myID, vCmds);
	else if(vCmds[0] == "printenv") printEnv(vCmds);
	else
	{
		//ls -S bin|cat|cat|cat|number > ls.txt
		//ls -S bin|cat|cat|cat|number >|
		//cat <7 >|
		string sCmd;
		int fdpipe[2];
		char** argv;
		bool FLAG_RECV_PIPE = false;
		bool FLAG_SEND_PIPE = false;
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
				map<unsigned int, struct ForwardPipe>::iterator iter = pForwardPipe->find(cmdno);
				if(iter!=pForwardPipe->end())
				{
					dup2(iter->second.rpipe, STDIN_FILENO);
					close(iter->second.rpipe);
					close(iter->second.wpipe);
					pForwardPipe->erase(iter);
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

						map<unsigned int, struct ForwardPipe>::iterator iter = pForwardPipe->find(cmdno + iJump);
						if(iter==pForwardPipe->end()){
							int relayPipe[2];
							struct ForwardPipe fwdpipe;
							pipe(relayPipe);
							fwdpipe.rpipe = relayPipe[0];
							fwdpipe.wpipe = relayPipe[1];
							pForwardPipe->insert(pair<int, struct ForwardPipe>(iJump + cmdno, fwdpipe));

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
					bool result = pipeOut(oUserInfo, fd);
					if(result) FLAG_SEND_PIPE = true;
					else return;
				}
				else if(vCmds[index]==">!")
				{
					bool FLAG_STDERR = true;
					bool result = pipeOut(oUserInfo, fd, FLAG_STDERR);
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
						cout << "*** Error: syntax error '<N' ***" << endl;
						fflush(stdout);
						return;
					}

					if( !oUserInfo.isUserPipeOccupied(senderID) ){
						cout << "*** Error: the pipe from #" << senderID << " does not exist yet. *** " << endl;
						fflush(stdout);
						return;
					}

					int rpipe = oUserInfo.getUserRPipeByID(senderID);
					dup2(rpipe, STDIN_FILENO);
					close(rpipe);
					oUserInfo.setFlagPipeOccupied(senderID, false);
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
					dup2(fd, STDOUT_FILENO);
					cout << "Unknown command: [" << sCmd << "]." << endl;
					fflush(stdout);
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

			msg = "*** ";
			msg += oUserInfo.getUserNameByID(myID);
			msg += " (#";
			msg += Utils::int2String(myID);
			msg += ") just received the pipe from ";
			msg += oUserInfo.getUserNameByID(senderID);
			msg += " (#";
			msg += Utils::int2String(senderID);
			msg += ") by '";
			msg += sMsg.substr(0, sMsg.size()-2);
			msg += "' ***\n";
			oUserInfo.broadcast(msg);
		}

		if(FLAG_SEND_PIPE){
			string msg;

			msg = "*** ";
			msg += oUserInfo.getUserNameByID(myID);
			msg += " (#";
			msg += Utils::int2String(myID);
			msg += ") just piped '";
			msg += sMsg.substr(0, sMsg.size()-2);
			msg += "' into his/her pipe. ***\n";
			oUserInfo.broadcast(msg);
		}

	}//end else









//   <ID>	<nickname>	<IP/port>	<indicate me>
//   1	IamStudent	140.113.215.62/1201	<-me
//   2	(No Name)	140.113.215.63/1013
//   3	student3	140.113.215.64/1302
// *** <sender's name> told you ***: <message>
}

void CommandHandler::parser(vector<string>& vCmds, const string& msg)
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
		if(begin!=string::npos) vCmds.push_back( msg.substr(begin, end-begin) ); //<message>
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

	if(argc < 1)
	{
		cout << "*** Error: argc < 1 ***" << endl;
		fflush(stdout);
		return -1;
	}

	return argc;
}

void CommandHandler::tell(UserInfo& oUserInfo, int myID, vector<string>& vCmds)
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
		cout << "*** Error: syntax error 'tell [ID] [SAYING]' ***" << endl;
		fflush(stdout);
		return;
	}

	int yourID = Utils::string2Int(vCmds[1]);
	if(!oUserInfo.hasUser(yourID))
	{
		cout << "*** Error: user #" << yourID << " does not exist yet. ***" << endl;
		fflush(stdout);
		return;
	}
	else
	{
		int yourfd = oUserInfo.getUserFDByID(yourID);

		string msg = "*** ";
		msg += oUserInfo.getUserNameByID(myID);
		msg += " told you ***: ";
		msg += vCmds[2];
		msg += "\n";

		send(yourfd, msg.c_str(), msg.size(), 0);
	}
}

void CommandHandler::yell(UserInfo& oUserInfo, int myID, vector<string>& vCmds)
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
		cout << "*** Error: syntax error 'yell [SAYING]' ***" << endl;
		fflush(stdout);
		return;
	}


	string msg = "*** ";
	msg += oUserInfo.getUserNameByID(myID);
	msg += " yelled ***: ";
	msg += vCmds[1];
	msg += "\n";

	oUserInfo.broadcast(msg);
}

void CommandHandler::name(UserInfo& oUserInfo, int myID, vector<string>& vCmds)
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
		cout << "*** Error: syntax error 'name [NAME]' ***" << endl;
		fflush(stdout);
		return;
	}

	oUserInfo.setUserNameByID(myID, vCmds[1]); /*Now, name of user is changed, but oUserMe is old name */

	char ip[16];
	oUserInfo.getUserIPByID(myID, ip);
	string msg = "*** User from ";
	msg += ip;
	msg += "/";
	msg += Utils::int2String(oUserInfo.getUserPortByID(myID));
	msg += " is named '";
	msg += vCmds[1];
	msg += "'. ***";
	msg += "\n";

	oUserInfo.broadcast(msg);
}

void CommandHandler::who(UserInfo& oUserInfo, int myID)
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
   <sockd>	<nickname>	<IP/port>	<indicate me>
   1	IamStudent	140.113.215.62/1201	<-me
   2	(No Name)	140.113.215.63/1013
   3	student3	140.113.215.64/1302
*/
	string msg;
	msg = oUserInfo.getUserListStringByID(myID);

	cout << msg;
	fflush(stdout);
}

void CommandHandler::quit(UserInfo& oUserInfo, int myID)
{
/*
8. When a client disconnect from the server, broadcast as follows.
   % *** User '<name>' left. ***

   eg.
   [terminal of all clients]
   % *** User 'student5' left. ***
*/
	string msg = "*** User '";
	msg += oUserInfo.getUserNameByID(myID);
	msg += "' left. ***";
	msg += "\n";

	oUserInfo.delUser(myID);

	oUserInfo.broadcast(msg);

	FLAG_EXIT = true;
}

void CommandHandler::resetPathEnv(UserInfo& oUserInfo, int myID)
{
	string pathEnv = oUserInfo.getUserPathEnvByID(myID);
	setenv("PATH", pathEnv.c_str(), 1);
}

void CommandHandler::setEnv(UserInfo& oUserInfo, int myID, vector<string>& vCmds)
{
	if(vCmds.size() < 3)
	{
		cout << "*** Error: syntax error 'setenv [ENVNAME] [VALUE]' ***" << endl;
		fflush(stdout);
		return;
	}

	oUserInfo.setUserPathEnvByID(myID, vCmds[2]);
}


void CommandHandler::printEnv(vector<string>& vCmds)
{
	if(vCmds.size() < 2)
	{
		cout << "*** Error: syntax error 'printenv [ENVNAME]' ***" << endl;
		fflush(stdout);
		return;
	}

#ifdef DEBUG
	Utils::write2Concole(vCmds[1]);
#endif
	string msg = vCmds[1];
	msg += "=";
	msg += getenv(vCmds[1].c_str());
	msg += '\n';

	cout << msg;
	fflush(stdout);
}

bool CommandHandler::pipeOut(UserInfo& oUserInfo, int fd, bool STDERR)
{
	User user = oUserInfo.getUserByFD(fd);
	if(user.FLAG_PIPE_OCCUPIED)
	{
		cout << "*** Error: your pipe already exists. ***" << endl;
		fflush(stdout);
		return false;
	}
	else
	{
		int fdpipe[2];
		oUserInfo.setFlagPipeOccupied(user.ID, true);
		pipe(fdpipe);
		oUserInfo.setUserPipeByID(user.ID, fdpipe);
		dup2(fdpipe[1] ,STDOUT_FILENO);
		if(STDERR == true) dup2(fdpipe[1], STDERR_FILENO);
		close(fdpipe[1]);
		return true;
	}
}

bool CommandHandler::isExit()
{
	if(FLAG_EXIT)
	{
		FLAG_EXIT = false; //recover for use next command
		return true;
	}
	else return false;
}

CommandHandler::~CommandHandler() {
	// TODO Auto-generated destructor stub
}
