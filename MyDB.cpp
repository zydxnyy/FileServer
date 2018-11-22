#include "MyDB.h"

MyDB::MyDB() {
	logger = Logger::getLogger();
	connection = mysql_init(NULL); // 初始化数据库连接变量
	result = nullptr;
	if (connection == NULL)
	{
		Trace << "Error:" << mysql_error(connection);
		exit(1);
	}
}

MyDB::~MyDB() {
	if (result) mysql_free_result(result);
	if (connection != NULL)  // 关闭数据库连接
	{
		mysql_close(connection);
	}
}


bool MyDB::initDB(string host, string user, string pwd, string db_name) {
	// 函数mysql_real_connect建立一个数据库连接
	// 成功返回MYSQL*连接句柄，失败返回NULL
	char value = 1;
	mysql_options(connection, MYSQL_OPT_RECONNECT, (char *)&value);
	connection = mysql_real_connect(connection, host.c_str(),
		user.c_str(), pwd.c_str(), db_name.c_str(), 0, NULL, 0);
	if (connection == NULL)
	{
		Trace << "Error:" << mysql_error(connection);
		logger->log(mysql_error(connection), ERROR);
		exit(1);
	}
	if (!execSQL("SET NAMES 'utf8';")) {
		Trace << "Set utf8 failed" << endl;
		logger->log("Set utf8 failed", ERROR);
	}
	return true;
}

bool MyDB::execSQL(string sql) {
	// mysql_query()执行成功返回0，失败返回非0值。与PHP中不一样
	if (mysql_query(connection, sql.c_str()))
	{
		Trace << "Query Error:" << mysql_error(connection);
		logger->log(mysql_error(connection), ERROR);
		return false;
	}
	else
	{
		return true;
	}
}

MYSQL_RES* MyDB::querySQL(string sql) {
	if (result) mysql_free_result(result);
	// mysql_query()执行成功返回0，失败返回非0值。与PHP中不一样
	if (mysql_query(connection, sql.c_str()))
	{
		Trace << "Query Error:" << mysql_error(connection);
		logger->log(mysql_error(connection), ERROR);
		return nullptr;
	}
	else
	{
		result = mysql_store_result(connection); // 获取结果集
		//mysql_field_count()返回connection查询的列数
		if (mysql_num_rows(result)) return result;
		else return nullptr;
		//for (int i = 0; i < mysql_field_count(connection); ++i)
		//{
		//	// 获取下一行
		//	row = mysql_fetch_row(result);
		//	if (row <= 0)
		//	{
		//		break;
		//	}
		//	// mysql_num_fields()返回结果集中的字段数
		//	for (int j = 0; j < mysql_num_fields(result); ++j)
		//	{
		//		cout << row[j] << " ";
		//	}
		//	cout << endl;
		//}
	}
	//return true;
}