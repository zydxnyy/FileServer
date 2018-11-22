#include "util.h"
#include "stdlib.h"
#include <iostream>
#include <stdio.h>
#include "HttpOp.h"
using namespace std;

string&   replace_all(string&   str, const   string&   old_value, const   string&   new_value)
{
	while (true) {
		string::size_type   pos(0);
		if ((pos = str.find(old_value)) != string::npos)
			str.replace(pos, old_value.length(), new_value);
		else   break;
	}
	return   str;
}

string&   replace_all_distinct(string&   str, const   string&   old_value, const   string&   new_value)
{
	for (string::size_type pos(0); pos != string::npos; pos += new_value.length()) {
		if ((pos = str.find(old_value, pos)) != string::npos)
			str.replace(pos, old_value.length(), new_value);
		else   break;
	}
	return   str;
}

size_t get_file_size(const string& path) {
	ifstream f(path, ios::binary);
	if (!f) {
		return 0;
	}
	f.seekg(0, f.end);
	size_t size = f.tellg();
	return size;
}

string get_file_name(const string& path) {
	int off = -1;
	for (int i = path.size() - 1; i >= 0; --i) {
		if (path[i] == '/') {
			off = i+1;
			break;
		}
	}
	if (off == -1) return "";
	return path.substr(off);
}

bool hasSuffix(const string & src, const string & suffix)
{
	if (src.substr(src.size() - suffix.size()) == suffix) return true;
	return false;
}

long long int atol(string num)
{
	long long int n = 0;
	for (int i = 0; i < num.size(); ++i) {
		n *= 10;
		n += num[i] - '0';
	}
	return n;
}

void check_token(const string& email, const string& token, int& rtn) {
	rtn = checkToken(email, token);
	return;
}

Py_Ret get_all_project(const string& email, const string& token) {
	return getAllProject(email, token);
}

bool json_parse(const string& s, Json::Value& v) {
	Json::Reader reader;
	return reader.parse(s, v);
}

void json_write(string &s, Json::Value& v) {
	Json::FastWriter writer;
	s = writer.write(v);
}

int gSend(UDTSOCKET usock, const char* buf, int size, int) {
	int ssize = 0;
	int ss;
	while (ssize < size)
	{
		if (UDT::ERROR == (ss = UDT::send(usock, buf + ssize, size - ssize, 0)))
		{
			Trace << "send:" << UDT::getlasterror().getErrorMessage() << endl;
			return UDT::ERROR;
		}

		ssize += ss;
	}
	return ssize;
}

int gRecv(UDTSOCKET usock, char * buf, int size, int)
{
	int rsize = 0;
	int rs;
	while (rsize < size)
	{
		if (UDT::ERROR == (rs = UDT::recv(usock, buf + rsize, size - rsize, 0))) {

			Trace << "recv:" << UDT::getlasterror().getErrorMessage() << endl;
			return UDT::ERROR;
			break;
		}
		rsize += rs;
	}
	return rsize;
}

string hashFile(const char* filename) {
	string s;
	ifstream ifs(filename, ios::binary);
	MD5 m;
	if (ifs.is_open())
	{
		const int buffer_size = 8192;
		char* buffer = new char[buffer_size];
		while (ifs.read(buffer, buffer_size))
		{
			m.update(buffer, buffer_size);
		}
		int remain = ifs.gcount();
		if (remain) {
			m.update(buffer, remain);
		}
		delete[] buffer;
	}
	//qDebug() << "fileStr: " << s.c_str() << s.size() << endl;

	return m.finalize().hexdigest();
}

int getProjId(const string & pname)
{
	int pos = pname.find('_');
	if (pos == -1) return atol(pname.c_str());
	else return atol(pname.substr(pos + 1).c_str());
}

string getProjName(int pid)
{
	string ret = "Proj_" + itoa(pid);
	return ret;
}

string itoa(int num)
{
	string n;
	while (num) {
		n += (num % 10) + '0';
		num /= 10;
	}
	string ret(n);
	for (int i = 0; i < n.size(); ++i) {
		ret[i] = n[n.size() - 1 - i];
	}
	return ret;
}
