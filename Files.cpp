#include "Files.h"
#include <time.h>
//string host = "localhost", user = "root", pwd = "zhf123456", db_name = "MFTP";

Files::Files()
{
	if (!db.initDB(host.c_str(), user.c_str(), pwd.c_str(), db_name.c_str())) {
		cout << "DB connection failed ..." << endl;
		exit(-1);
	}
}


Files::~Files()
{
}


bool Files::insertFile(string type, string email, string filename, size_t filesize, string path, string fileHash, int status, size_t offset, int projId) {
	time_t uploadtime = time(NULL);
	if (filename.empty()) {
		cout << "Filename is empty" << endl;
		return false;
	}
	char sql[500];
	sprintf(sql, "insert into dms_files (type, email, filename, filesize, created,filepath, fileHash, status, offset, projname_id) \
	values('%s', '%s', '%s', %lld, %lld, '%s', '%s', %d, %lld, %d);", type.c_str(), email.c_str(), filename.c_str(),
		filesize, uploadtime, path.c_str(), fileHash.c_str(), status, offset, projId);
	cout << "Sql = " << sql << endl;
	return db.execSQL(sql);
}

bool Files::deleteFile(string type, int projId, string filename) {
	char sql[500];
	sprintf(sql, "delete from dms_files where type = '%s' and projname_id = %d and filename = '%s'", type.c_str(), projId, filename.c_str());
	return db.execSQL(sql);
}

bool Files::update(string type, int projId, string filename, int state, size_t offset)
{
	if (filename.empty()) return false;
	char sql[500];
	sprintf(sql, "update dms_files set status=%d, offset=%lld, created=%lld where type='%s' and projname_id = %d and filename='%s';", state, offset, time(NULL), type.c_str(), projId, filename.c_str());
	//cout << "SQl = " << sql << endl;
	return db.execSQL(sql);
}

vector<myfile> Files::queryAll() {
	vector<myfile> vFile;
	vFile.clear();
	char sql[100] = "select * from dms_files where status = 1;";
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
		vFile.push_back(myfile(row[1], row[2], row[3], atol(row[4]), atol(row[5]),
			row[6], row[7], atol(row[8]),atol(row[9]), atol(row[10])));
	}
	return vFile;
}

myfile Files::queryFile(string type, int projId, string filename) {
	char sql[500];
	sprintf(sql, "select * from dms_files where type='%s' and projname_id = %d and filename='%s';", type.c_str(), projId, filename.c_str());
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
		return myfile(row[1], row[2], row[3], atol(row[4]), atol(row[5]),
			row[6], row[7], atol(row[8]), atol(row[9]), atol(row[10]));
	}
}

vector<myfile> Files::queryProject(string type, int projId)
{
	vector<myfile> vFile;
	vFile.clear();
	char sql[500];
	sprintf(sql, "select * from dms_files where type='%s' and projname_id = %d;", type.c_str(), projId);
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
		vFile.push_back(myfile(row[1], row[2], row[3], atol(row[4]), atol(row[5]),
			row[6], row[7], atol(row[8]), atol(row[9]), atol(row[10])));
	}
	return vFile;
}
