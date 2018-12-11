#pragma once
#include <iostream>
#include <time.h>
#include <vector>
#include <stdio.h>
#include "MyDB.h"
#include "util.h"
using namespace std;

static string host = "localhost", user = "root", pwd = "zhf123456", db_name = "ddix_test4";
//static string host = "localhost", user = "root", pwd = "5288", db_name = "ddix_test3";

struct myfile {
	string type;
	string email;
	string filename;
	size_t filesize;
	time_t uploadtime;
	string filepath;
	string fileHash;
	int status;
	size_t offset;
	int projId;
	myfile(const string& type, const string& email, const string& filename, size_t filesize, time_t uploadtime, const string& filepath, const string fileHash, 
		int status, size_t offset, int projId):
		type(type), email(email), filename(filename), filesize(filesize), uploadtime(uploadtime), filepath(filepath), fileHash(fileHash), status(status), offset(offset), projId(projId)
	{}
	myfile() { offset = 0; }

	myfile(const myfile& ano) {
		type = ano.type;
		email = ano.email;
		filename = ano.filename;
		filesize = ano.filesize;
		uploadtime = ano.uploadtime;
		filepath = ano.filepath;
		fileHash = ano.fileHash;
		status = ano.status;
		offset = ano.offset;
		projId = ano.projId;
	}

	bool valid() {
		return !filename.empty();
	}

	friend ostream& operator<<(ostream& os, const myfile& ano) {
		if (ano.filename.empty()) os << "Invalid file";
		else os << ano.projId << " " << ano.filename << " " << ano.filesize << " " << ano.uploadtime << " " << ano.status << " " << ano.offset << " " << ano.filepath;
		return os;
	}
};

class Files
{
public:
	Files();
	~Files();
	//myfile(string& type, string& email, string& filename, size_t filesize, time_t uploadtime, string& filepath, string fileHash, int status, size_t offset, int projId) :
	bool insertFile(string type, string email, string filename, size_t filesize, string path, string fileHash, int status, size_t offset, int projId);
	bool deleteFile(string type, int projId, string filename);
	bool update(string type, int projId, string filename, int state, size_t offset);
	vector<myfile> queryAll();
	myfile queryFile(string type, int projId, string filename);
	vector<myfile> queryProject(string type, int projId);
private:
	MyDB db;
};

