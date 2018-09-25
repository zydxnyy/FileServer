#pragma once
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sstream>

using namespace std;

#define DBG cout

class HttpConnect
{
	char* reply;
	int port;
public:
	HttpConnect(int port);
	void socketHttp(string host, string request);
	string postData(std::string host, std::string path, std::string post_content);
	string getData(std::string host, std::string path, std::string get_content);

	~HttpConnect();
};

