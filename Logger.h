#pragma once
#include <iostream>
#include <time.h>
#include <mutex>
#include <string.h>
#include <fstream>
using namespace std;


enum LOG_LEVEL {
	INFO,
	WARN,
	ERROR,
};

class Logger
{
	static Logger* pLogger;
	//д��־�ļ��Ļ�����
	recursive_timed_mutex m_mutex;
	//��־�ļ�
	ofstream m_file;

	Logger();
protected:
	std::string getCurrentTime();

public:
	~Logger();

	static Logger* getLogger();

	void log(const char* str, LOG_LEVEL level);
};

