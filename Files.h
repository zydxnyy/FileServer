#pragma once
#include <iostream>
#include <time.h>
#include <vector>
#include <stdio.h>
#include "MyDB.h"
#include "util.h"
using namespace std;

static string host = "localhost", user = "root", pwd = "zhf123456", db_name = "MFTP";

struct myfile {
	string projname;
	string filename;
	size_t filesize;
	time_t uploadtime;
	int state;
	size_t offset;
	string path;
	string email;
	string fileHash;
	myfile(const string& projname, const string& filename, size_t filesize, time_t uploadtime, int state, size_t offset, string path, string email, string fileHash)
		: projname(projname), filename(filename), filesize(filesize), uploadtime(uploadtime), state(state), offset(offset), path(path), email(email), fileHash(fileHash) {}

	myfile() { offset = 0; }

	myfile(const myfile& ano) {
		projname = ano.projname;
		filename = ano.filename;
		filesize = ano.filesize;
		uploadtime = ano.uploadtime;
		state = ano.state;
		offset = ano.offset;
		path = ano.path;
		email = ano.email;
		fileHash = ano.fileHash;
	}

	bool valid() {
		return !projname.empty();
	}

	friend ostream& operator<<(ostream& os, const myfile& ano) {
		if (ano.projname.empty()) os << "Invalid file";
		else os << ano.projname << " " << ano.filename << " " << ano.filesize << " " << ano.uploadtime << " " << ano.state << " " << ano.offset << " " << ano.path;
		return os;
	}
};

class Files
{
public:
	Files();
	~Files();
	bool insertFile(string projname, string filename, size_t filesize, int state, size_t offset, string path, string email, string fileHash);
	bool deleteFile(string projname, string filename);
	bool update(string projname, string filename, int state, size_t offset);
	vector<myfile> queryAll();
	myfile queryFile(string projname, string filename);
	vector<myfile> queryProject(string projname);
private:
	MyDB db;
};

