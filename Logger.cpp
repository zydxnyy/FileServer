#include "Logger.h"


const int MAX_LOG_LEN = 1024;

const char* INFOPRE = "[INFO] ";
const char* WARNPRE = "[WARNNING] ";
const char* ERRORPRE = "[ERROR] ";

Logger* Logger::pLogger = new Logger();

std::string Logger::getCurrentTime()
{
	time_t tt = time(NULL);//这句返回的只是一个时间cuo
	tm* t = localtime(&tt);
	char buf[MAX_LOG_LEN];
	sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d ",
		t->tm_year + 1900,
		t->tm_mon + 1,
		t->tm_mday,
		t->tm_hour,
		t->tm_min,
		t->tm_sec);
	return std::string(buf);
}

Logger::Logger():m_file("/home/webadmin/projects/linuxServer/logs/FileServer.log", ios::out | ios::binary | ios::app)
{
	if (!m_file) {
		Trace << "Logger open failed..." << endl;
		exit(-1);
	}
	else {
		Trace << "Logger opened" << endl;
		log("Opened", INFO);
	}
}


Logger::~Logger()
{
	m_file.close();
}

Logger * Logger::getLogger()
{
	return pLogger;
}

void Logger::log(const char * str, LOG_LEVEL level)
{
	char buf[MAX_LOG_LEN];
	strcpy(buf, getCurrentTime().c_str());
	switch (level) {
	case INFO:
		strcat(buf, INFOPRE);
		break;
	case WARN:
		strcat(buf, WARNPRE);
		break;
	case ERROR:
		strcat(buf, ERRORPRE);
		break;
	}
	strcat(buf, str);

	m_mutex.lock();
	m_file << buf << endl;
	m_mutex.unlock();
}
