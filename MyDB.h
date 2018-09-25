#pragma once
#include<iostream>
#include<mysql/mysql.h>
using namespace std;

class MyDB
{
public:
	MyDB();
	~MyDB();
	bool initDB(string host, string user, string pwd, string db_name);
	bool execSQL(string sql);
	MYSQL_RES * querySQL(string sql);
	MYSQL *c() { return connection; }
private:
	MYSQL *connection;
	MYSQL_RES *result;
	MYSQL_ROW row;
};
