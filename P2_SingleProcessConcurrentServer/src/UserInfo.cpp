/*
 * UserInfo.cpp
 *
 *  Created on: 2011/11/13
 *      Author: Ching-Hsuan Wei
 */

#include "UserInfo.h"

UserInfo::UserInfo() {
	// TODO Auto-generated constructor stub

}

User UserInfo::addUser(int fd, const char ip[], int port)
{
	for(int i=1; ; i++)
	{
		map<int, User>::iterator iter = m_mUser.find(i);
		if(iter==m_mUser.end())
		{
			User user;
			user.ID = i;
			user.fd = fd;
			strcpy(user.ip, ip);
			user.port = port;
			m_mUser.insert(pair<int, User>(i, user));
			return user;
		}
	}
}

void UserInfo::delUser(int ID)
{
	map<int, User>::iterator iter = m_mUser.find(ID);

	if(iter==m_mUser.end()) cout << "*** Error: UserInfo::delUser() userID:" << ID << " not found. ***";
	else m_mUser.erase(iter);
}

bool UserInfo::hasUser(int ID)
{
	map<int, User>::iterator iter = m_mUser.find(ID);
	if(iter==m_mUser.end()) return false;
	else return true;
}

int UserInfo::getUserIDByFD(int fd)
{
	map<int, User>::iterator iter;
	for(iter = m_mUser.begin(); iter != m_mUser.end(); iter++)
	{
		if(iter->second.fd==fd) return iter->second.ID;
	}

	return -1;
}

int UserInfo::getUserFDByID(int ID)
{
	map<int, User>::iterator iter = m_mUser.find(ID);
	return iter->second.fd;
}

string UserInfo::getUserNameByID(int ID)
{
	map<int, User>::iterator iter = m_mUser.find(ID);
	return iter->second.name;
}

User UserInfo::getUserByID(int ID)
{
	map<int, User>::iterator iter = m_mUser.find(ID);
	if(iter==m_mUser.end()) return User(-1);
	else return iter->second;
}

void UserInfo::setUserNameByID(int ID, const string& name)
{
	map<int, User>::iterator iter = m_mUser.find(ID);
	iter->second.name = name;
}

User UserInfo::getUserByFD(int fd)
{
	map<int, User>::iterator iter;
	for(iter = m_mUser.begin(); iter != m_mUser.end(); iter++)
	{
		if(iter->second.fd==fd) return iter->second;
	}

	return User(-1);
}

void UserInfo::getUserIPByID(int ID, char _ip[])
{
	map<int, User>::iterator iter = m_mUser.find(ID);
	strcpy(_ip, iter->second.ip);
}

int UserInfo::getUserPortByID(int ID)
{
	map<int, User>::iterator iter = m_mUser.find(ID);
	return iter->second.port;
}

int UserInfo::getUserRPipeByID(int ID)
{
	map<int, User>::iterator iter = m_mUser.find(ID);
	return iter->second.fdpipe[0];
}


map<unsigned int, struct ForwardPipe>* UserInfo::getUserForwardPipePointerByID(int ID)
{
	map<int, User>::iterator iter = m_mUser.find(ID);
	return &(iter->second.mForwardPipe);
}

string UserInfo::getUserPathEnvByID(int ID)
{
	map<int, User>::iterator iter = m_mUser.find(ID);
	return iter->second.pathEnv;
}


void UserInfo::setUserPathEnvByID(int ID, const string& pathEnv)
{
	map<int, User>::iterator iter = m_mUser.find(ID);
	iter->second.pathEnv = pathEnv;
}

void UserInfo::setFlagPipeOccupied(int ID, bool flag)
{
	m_mUser[ID].FLAG_PIPE_OCCUPIED = flag;
}

void UserInfo::setUserPipeByID(int ID, int fdpipe[2])
{
	m_mUser[ID].fdpipe[0] = fdpipe[0];
	m_mUser[ID].fdpipe[1] = fdpipe[1];
}

bool UserInfo::isUserPipeOccupied(int ID)
{
	map<int, User>::iterator iter = m_mUser.find(ID);
	if(iter->second.FLAG_PIPE_OCCUPIED) return true;
	else return false;
}

int UserInfo::increaseUserCommandNo(int ID)
{
	map<int, User>::iterator iter = m_mUser.find(ID);
	return ++iter->second.cmdno;
}

void UserInfo::broadcast(string& msg)
{
	map<int, User>::iterator iter;
	for(iter = m_mUser.begin(); iter != m_mUser.end(); iter++)
	{
		send(iter->second.fd, msg.c_str() , msg.size(), 0);
	}
}

string UserInfo::getUserListStringByID(int myID)
{
/*
	1	IamStudent	140.113.215.62/1201	<-me
	2	(No Name)	140.113.215.63/1013
	3	student3	140.113.215.64/1302
*/
	string str = "<ID>\t<nickname>\t<IP/port>\t<indicate me>\n";
	map<int, User>::iterator iter;
	for(iter = m_mUser.begin(); iter != m_mUser.end(); iter++)
	{
		str += Utils::int2String(iter->first);
		str += "\t";
		str += iter->second.name;
		str += "\t";
		str += iter->second.ip;
		str += "/";
		str += Utils::int2String(iter->second.port);
		if(myID == iter->second.ID)
		{
			str += "\t";
			str += "<-me";
		}
		str += "\n";
	}

	return str;
}

UserInfo::~UserInfo() {
	// TODO Auto-generated destructor stub
}

