#include "Files.h"
#include <time.h>
//string host = "localhost", user = "root", pwd = "zhf123456", db_name = "MFTP";

Files::Files()
{
	db.initDB(host.c_str(), user.c_str(), pwd.c_str(), db_name.c_str());
}


Files::~Files()
{
}


bool Files::insertFile(string projname, string filename, size_t filesize, int state, size_t offset, string path, string email, string fileHash) {
	time_t uploadtime = time(NULL);
	if (projname.empty() || filename.empty()) return false;
	char sql[500];
	sprintf(sql, "insert into files values('%s', '%s', %lld, %lld, %d, %lld, '%s', '%s', '%s');", projname.c_str(), filename.c_str(), 
		filesize, uploadtime, state, offset, path.c_str(), email.c_str(), fileHash.c_str());
	return db.execSQL(sql);
}

bool Files::deleteFile(string projname, string filename) {
	char sql[500];
	sprintf(sql, "delete from files where projname = '%s' and filename = '%s'", projname.c_str(), filename.c_str());
	return db.execSQL(sql);
}

bool Files::update(string projname, string filename, int state, size_t offset)
{
	if (projname.empty() || filename.empty()) return false;
	char sql[500];
	sprintf(sql, "update files set state=%d, offset=%lld, created=%lld where projname='%s' and filename='%s';", state, offset, time(NULL), projname.c_str(), filename.c_str());
	//cout << "SQl = " << sql << endl;
	return db.execSQL(sql);
}

vector<myfile> Files::queryAll() {
	vector<myfile> vFile;
	vFile.clear();
	char sql[100] = "select * from files;";
	MYSQL_RES* result = db.querySQL(sql);
	if (!result) return vFile;
	for (int i = 0; i < mysql_num_rows(result); ++i)
	{
		// 获取下一行
		MYSQL_ROW row = mysql_fetch_row(result);
		if (row <= 0)
		{
			break;
		}
		// mysql_num_fields()返回结果集中的字段数
		vFile.push_back(myfile(row[0], row[1], atol(row[2]), atol(row[3]), atol(row[4]),atol(row[5]), row[6], row[7], row[8]));
	}
	return vFile;
}

myfile Files::queryFile(string projname, string filename) {
	char sql[500];
	sprintf(sql, "select * from files where projname='%s' and filename='%s';", projname.c_str(), filename.c_str());
	MYSQL_RES* result = db.querySQL(sql);
	if (!result) {
		cout << "Query fail" << endl;
		return myfile();
	}
	for (int i = 0; i < mysql_num_rows(result); ++i)
	{
		// 获取下一行
		MYSQL_ROW row = mysql_fetch_row(result);
		if (row <= 0)
		{
			break;
		}
		// mysql_num_fields()返回结果集中的字段数
		return myfile(row[0], row[1], atol(row[2]), atol(row[3]), atol(row[4]), atol(row[5]), row[6], row[7], row[8]);
	}
}

vector<myfile> Files::queryProject(string projname)
{
	vector<myfile> vFile;
	vFile.clear();
	char sql[500];
	sprintf(sql, "select * from files where projname='%s';", projname.c_str());
	MYSQL_RES* result = db.querySQL(sql);
	if (!result) return vFile;
	for (int i = 0; i < mysql_num_rows(result); ++i)
	{
		// 获取下一行
		MYSQL_ROW row = mysql_fetch_row(result);
		if (row <= 0)
		{
			break;
		}
		// mysql_num_fields()返回结果集中的字段数
		vFile.push_back(myfile(row[0], row[1], atol(row[2]), atol(row[3]), atol(row[4]), atol(row[5]), row[6], row[7], row[8]));
	}
	return vFile;
}
