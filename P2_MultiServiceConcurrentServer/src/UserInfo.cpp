/*
 * UserInfo.cpp
 *
 *  Created on: 2011/11/19
 *      Author: Ching-Hsuan Wei
 */

#include "UserInfo.h"

UserInfo::UserInfo() {
	// TODO Auto-generated constructor stub

}

void UserInfo::init()
{
	for(int i=1; i<=30; i++)
	{
		m_aUser[i].init(i);
	}
}

User UserInfo::addUser(pid_t pid, char ip[], int port)
{
	for(int i=1; i<=30; i++)
	{
		if(m_aUser[i].notUsed())
		{
			m_aUser[i].ID = i;
			m_aUser[i].pid = pid;
			strcpy(m_aUser[i].name, "(no name)");
			strcpy(m_aUser[i].ip, ip);
			m_aUser[i].port = port;
			bzero(m_aUser[i].msg, sizeof(m_aUser[i].msg));
			return m_aUser[i];
		}
	}

	cout << "*** Error: UserInfo::addUser() Number of all users Exceed 30. ***" << endl;
	return User(-1);
}

void UserInfo::delUser(int ID)
{
	m_aUser[ID].init(ID);
}

void UserInfo::setUserNameByID(int ID, const string& name)
{
	strcpy(m_aUser[ID].name, name.c_str());
}

void UserInfo::setUserFlagPipeOccupiedByID(int ID, bool flag)
{
	m_aUser[ID].FLAG_PIPE_OCCUPIED = flag;
}

void  UserInfo::getUserNameByID(int ID, char _name[])
{
	strcpy(_name, m_aUser[ID].name);
}

void UserInfo::getUserFIFONameByID(int ID, char _fifoname[])
{
	strcpy(_fifoname, m_aUser[ID].fifo);
}

void UserInfo::getUserMessageByID(int ID, char _msg[])
{
	strcpy(_msg, m_aUser[ID].msg);
}

void UserInfo::getUserIPByID(int ID, char _ip[])
{
	strcpy(_ip, m_aUser[ID].ip);
}

int UserInfo::getUserPortByID(int ID)
{
	return m_aUser[ID].port;
}

bool UserInfo::hasUser(int ID)
{
	if( !m_aUser[ID].isUser() ) return false;
	else return true;
}

bool UserInfo::isUserPipeOccupied(int ID)
{
	return m_aUser[ID].FLAG_PIPE_OCCUPIED;
}

string UserInfo::getUserListString(int myID)
{
/*
	1	IamStudent	140.113.215.62/1201	<-me
	2	(No Name)	140.113.215.63/1013
	3	student3	140.113.215.64/1302
*/
	string str = "<ID>\t<nickname>\t<IP/port>\t<indicate me>\n";
	for(int i=1; i<=30; i++)
	{
		if(m_aUser[i].isUser())
		{
			str += Utils::int2String(m_aUser[i].ID);
			str += "\t";
			str += m_aUser[i].name;
			str += "\t";
			str += m_aUser[i].ip;
			str += "/";
			str += Utils::int2String(m_aUser[i].port);
			if(myID == m_aUser[i].ID)
			{
				str += "\t";
				str += "<-me";
			}
			str += "\n";
		}
	}
	return str;
}

void UserInfo::broadcast(string& msg)
{
	for(int i=1; i<=30; i++)
	{
		if(m_aUser[i].isUser())
		{
			usleep(5000);
			send(m_aUser[i].ID, msg);
		}
	}
}

void UserInfo::send(int ID, string& str)
{
	bzero(m_aUser[ID].msg, sizeof(m_aUser[ID].msg));
	strcpy(m_aUser[ID].msg, str.c_str());

	kill(m_aUser[ID].pid, SIGUSR1);
}

UserInfo::~UserInfo() {
	// TODO Auto-generated destructor stub
}

