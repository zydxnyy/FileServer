#pragma once
#include <iostream>
//#include <python2.7/Python.h>
#include <fstream>
#include <vector>
#include <cstring>
#include <stdio.h>
#include <json/json.h>
#include <functional>
#include <stdlib.h>
#include <src/udt.h>
#include "md5.h"
using namespace std;

#define SERVER_IP "10.132.100.180"
//#define SERVER_IP "192.168.135.131"
#define SERVER_PORT "9091"

#define ZERO(x) {memset((x), 0, sizeof(x));}

#define OP_TYPE int
//#define GETDIR 1
//#define UPLOAD 2
//#define DOWNLOAD 3
//
//#define MY_NO_ERROR 0
//#define MY_ERROR 1
//#define TOKEN_EXPIRE 2
//#define FILE_EXISTS 3
//#define SERVER_INTERNAL_ERROR 4
//#define FILE_NAME_DULPLICTAION 5
//#define FILE_NOT_EXISTS 6

enum OP {
	GETDIR,
	UPLOAD,
	DOWNLOAD,
	DEL,
};

//#define MY_ERROR 1
//#define TOKEN_EXPIRE 2
//#define FILE_EXISTS 3
//#define SERVER_INTERNAL_ERROR 4
//#define FILE_NAME_DULPLICTAION 5

enum FILE_TRANS_ERROR {
	MY_NO_ERROR = 0,
	MY_ERROR,
	TOKEN_EXPIRE,
	FILE_EXISTS,
	FILE_NOT_EXISTS,
	SERVER_INTERNAL_ERROR,
	FILE_NAME_DULPLICTAION,
	FILE_OPEN_FAILED,
	INCORRECT_ADDR,
	CONNECT_FAILED,
	SEND_REQ_FAILED,
	RECV_REPLY_FAILED,
	REPLY_ERROR,
	STOP_BY_USER,
	RECV_FILE_ERROR,
	SEND_FILE_ERROR,
	USER_AUTH_FAILED,
	FILE_HASH_FAULT,
};
#define File_Container vector<File>
#define Proj_Container vector<Proj*>
const int CHUNK_SIZE = 1 * 1 << 10;

struct FileRequest {
	char op;
	char email[50];
	char token[50];
	char type[50];
	char project_name[50];
	char file_name[100];
	size_t file_size;
	size_t offset;
	char fileHash[130];
	FileRequest() {
		ZERO(this->email); ZERO(this->token); ZERO(this->project_name); ZERO(this->file_name); ZERO(this->type);
	}
	FileRequest(char op, const char* email, const char* token, const char* type, const char* project_name, const char* file_name, size_t file_size, char* fileHash = 0, size_t offset = 0) {
		this->op = op;
		this->file_size = file_size;
		this->offset = offset;
		memset((char*)fileHash, 0, sizeof(fileHash));
		if (fileHash != NULL) memcpy(this->fileHash, fileHash, sizeof(fileHash));
		ZERO(this->email); ZERO(this->token); ZERO(this->project_name); ZERO(this->file_name); ZERO(this->type);
		memcpy(this->email, email, strlen(email));
		memcpy(this->token, token, strlen(token));
		memcpy(this->type, type, strlen(type));
		memcpy(this->project_name, project_name, strlen(project_name));
		memcpy(this->file_name, file_name, strlen(file_name));
	}
};

struct FileReply {
	char op;
	char error_flag;
	size_t file_size;
	size_t offset;
	char fileHash[130];
	char extra[100];
	FileReply() {
		error_flag = MY_NO_ERROR;
		ZERO(extra);
		file_size = 0;
		offset = 0;
		ZERO(fileHash);
	}
};



struct File {
	string name;
	long long size;
	File(){}
	File(const string& n, long long s) : size(s), name(n) {}
	File(const File& f) {
		name = f.name; size = f.size;
	}
};


struct Proj {
	string name;
	int uid;
	File_Container files;
	Proj(){}
	Proj(const Proj& ano) {
		name = ano.name;
		uid = ano.uid;
	}
};

struct Py_Ret {
	int status;
	string str;
	int extra;
	Py_Ret(int status, const string& str, int extra) : status(status), str(str), extra(extra) {}
	Py_Ret() {}
	Py_Ret(const Py_Ret& ano) {
		status = ano.status;
		str = ano.str;
		extra = ano.extra;
	}
};


//class PyThreadStateLock
//{
//public:
//	PyThreadStateLock(void)
//	{
//		state = PyGILState_Ensure();
//	}
//
//	~PyThreadStateLock(void)
//	{
//		PyGILState_Release(state);
//	}
//private:
//	PyGILState_STATE state;
//};

bool json_parse(const string& s, Json::Value& v);

void json_write(string &s, Json::Value& v);

void* get_py_func(const string& module_name, const string& func_name);

void replace_cstr(string& str, char target, char r_char);

string&   replace_all_distinct(string&   str, const   string&   old_value, const   string&   new_value);

string&   replace_all(string&   str, const   string&   old_value, const   string&   new_value);

size_t get_file_size(const string& path);

string get_file_name(const string& path);

void check_token(const string& email, const string& token, int& rtn);

//void pFunc_init();

bool hasSuffix(const string& src, const string& suffix);

long long int atol(string num);

int gSend(UDTSOCKET usock, const char* buf, int size, int);
int gRecv(UDTSOCKET usock, char* buf, int size, int);
string readFileIntoString(const char * filename);
string hashFile(const char* filename);

int getProjId(const string& pname);
string getProjName(int pid);
string itoa(int num);

Py_Ret get_all_project(const string& email, const string& token);