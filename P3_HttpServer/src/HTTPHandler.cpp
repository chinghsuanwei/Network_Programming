/*
 * HTTPHandler.cpp
 *
 *  Created on: 2011/12/16
 *      Author: Ching-Hsuan Wei
 */

#include "HTTPHandler.h"

HTTPHandler::HTTPHandler() {
	// TODO Auto-generated constructor stub
	REQUEST_URI = "";
	REQUEST_URI_TYPE = "";
	QUERY_STRING = "";
}

void HTTPHandler::dispatch(string& sMsg)
{
	//GET /form_get.htm?a=1 HTTP/1.1
	parser(sMsg);
	setCGIEnv();

	//execlp("ls", "ls", "public_html", 0);

	string sFilePath = ".";
	sFilePath += REQUEST_URI;
	if(REQUEST_URI_TYPE == "htm" || REQUEST_URI_TYPE == "html"){

		ifstream infile(sFilePath.c_str(), ios::in);
		if(infile.fail()){
			cout << "HTTP/1.0 404 Not Found";
		}
		else{
			string sContent = "";
			string sTmp = "";
			while(!infile.eof()){
				getline(infile, sTmp);
				sContent += sTmp;
				sContent += "\n";
			}
			cout << sContent;
			infile.close();
		}
	}else if(REQUEST_URI_TYPE =="cgi"){
		execlp(sFilePath.c_str(),  0);
	}else{
		cout << "HTTP/1.0 404 Not Found";
	}

}

void HTTPHandler::setCGIEnv()
{
	if(REQUEST_URI=="") REQUEST_URI = "index.html";
	setenv("REQUEST_URI", REQUEST_URI.c_str(), 1);
	setenv("QUERY_STRING", QUERY_STRING.c_str(), 1);
}

void HTTPHandler::parser(string& sMsg)
{
	size_t start;
	size_t end;
	size_t found = sMsg.find("GET");
	if(found==string::npos) found = sMsg.find("HEAD");

	string link;
	if(found!=string::npos){
		start = sMsg.find_first_of(" ", found+1);
		end = sMsg.find_first_of(" ", start+1);
		link = sMsg.substr(start+1, end-start-1);
	}

	if(link!="/"){
		found = link.find_first_of("?");
		REQUEST_URI = link.substr(0, found);
		if(found!=string::npos){
			QUERY_STRING = link.substr(found+1);
		}

		found = REQUEST_URI.find_last_of(".");
		REQUEST_URI_TYPE = REQUEST_URI.substr(found+1);
	}

	cerr << "REQUEST_URI = " << REQUEST_URI << endl;
	cerr << "QUERY_STRING = " << QUERY_STRING << endl;
	cerr << "REQUEST_URI_TYPE = " << REQUEST_URI_TYPE << endl;
}

HTTPHandler::~HTTPHandler() {
	// TODO Auto-generated destructor stub
}

