#include "util.h"
#include "stdlib.h"
#include <iostream>
#include <stdio.h>
#include "HttpOp.h"
using namespace std;

void replace_cstr(string& str, char target, char r_char) {
	for (int i = 0; i < str.size(); ++i) {
		if (str[i] == target) str[i] = r_char;
	}
}

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

//PyObject* pFunc = NULL;//全局可调用的python程序中的函数
//PyObject *f_login, *f_all, *f_self;
//void pFunc_init()
//{
//	PyRun_SimpleString("import sys");
//	PyRun_SimpleString("sys.path.append('../../../')");
//	PyObject* pyModule = PyImport_Import(PyString_FromString("httpops"));
//	if (pyModule == NULL)
//	{
//		cout << "Load module failed" << endl;
//		exit(0);
//	}
//	PyObject* pDict = PyModule_GetDict(pyModule);
//	pFunc = PyDict_GetItemString(pDict, "check_token");
//	f_login = PyDict_GetItemString(pDict, "verify_account");
//	f_all = PyDict_GetItemString(pDict, "get_all_project");
//	f_self = PyDict_GetItemString(pDict, "get_self_project");
//
//}

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

	//PyThreadStateLock PyThreadLock;
	///**************************以下加入需要调用的python脚本代码  Begin***********************/

	//PyObject* args = PyTuple_New(2);   // 2个参数
	//PyObject* arg1 = PyString_FromString(email.c_str());
	//PyObject* arg2 = PyString_FromString(token.c_str());
	//PyTuple_SetItem(args, 0, arg1);
	//PyTuple_SetItem(args, 1, arg2);
	//PyObject *pRet;

	//// 调用函数
	//pRet = PyObject_CallObject(pFunc, args);
	//if (pRet) {
	//	rtn = _PyInt_AsInt(pRet);
	//}
}

Py_Ret get_all_project(const string& email, const string& token) {
	return getAllProject(email, token);

	//PyThreadStateLock lock;
	//Py_Ret rtn(-1, "Error", -1);
	///**************************以下加入需要调用的python脚本代码  Begin***********************/
	//// 加载模块
	//PyObject *args, *arg1, *arg2, *pRet;
	//args = PyTuple_New(2);   // 2个参数
	//arg1 = PyString_FromString(email.c_str());
	//arg2 = PyString_FromString(token.c_str());
	//PyTuple_SetItem(args, 0, arg1);
	//PyTuple_SetItem(args, 1, arg2);

	//// 调用函数
	//pRet = PyObject_CallObject(f_all, args);
	//// 获取参数
	//if (pRet)  // 验证是否调用成功
	//{
	//	char* ret_str;
	//	int status;
	//	//解析元组
	//	PyArg_ParseTuple(pRet, "is", &status, &ret_str);
	//	rtn.status = status;
	//	rtn.str = ret_str;
	//	// << rtn.status << " " << rtn.str.c_str() << endl;
	//}
	//else {
	//	rtn.status = -1;
	//	rtn.str = "调用失败1";
	//}
	///**************************以下加入需要调用的python脚本代码  End***********************/
	//return rtn;
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
			cout << "send:" << UDT::getlasterror().getErrorMessage() << endl;
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

			cout << "recv:" << UDT::getlasterror().getErrorMessage() << endl;
			return UDT::ERROR;
			break;
		}
		rsize += rs;
	}
	return rsize;
}


string readFileIntoString(const char * filename)
{
	string s;
	FILE *fp = fopen(filename, "rb");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		int len = ftell(fp);
		//cout << "Len = " << len << endl;
		fseek(fp, 0, SEEK_SET);
		s.resize(len);
		fread((void*)s.data(), 1, len, fp);
		fclose(fp);
	}
	else
	{
		printf("fopen error\n");
	}
	return s;
}

string hashFile(const char* filename) {
	cout << "Hashing " << filename << endl;
	string fileStr = readFileIntoString(filename);
	//cout << "fileStr : " << fileStr << " " << fileStr.size() << endl;
	return MD5(fileStr).md5();
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
