#pragma once
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <vector>
#include <vector>
#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <src/udt.h>
#include <stdlib.h>
#include  <stdio.h> 
#include <json/json.h>
#include "util.h"
#include "Files.h"
#include "Logger.h"

class FileServer
{
public:
	FileServer();
	~FileServer();

	bool start();

	void work_thread(UDTSOCKET, string, string);

protected:
	vector<vector<Proj> > getProjects(const string& email, const string& token);

private:
	UDTSOCKET serv;
	Files f;
	Logger* logger;
};

