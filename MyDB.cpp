#include "MyDB.h"

MyDB::MyDB() {
	logger = Logger::getLogger();
	connection = mysql_init(NULL); // ��ʼ�����ݿ����ӱ���
	result = nullptr;
	if (connection == NULL)
	{
		Trace << "Error:" << mysql_error(connection);
		exit(1);
	}
}

MyDB::~MyDB() {
	if (result) mysql_free_result(result);
	if (connection != NULL)  // �ر����ݿ�����
	{
		mysql_close(connection);
	}
}


bool MyDB::initDB(string host, string user, string pwd, string db_name) {
	// ����mysql_real_connect����һ�����ݿ�����
	// �ɹ�����MYSQL*���Ӿ����ʧ�ܷ���NULL
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
	// mysql_query()ִ�гɹ�����0��ʧ�ܷ��ط�0ֵ����PHP�в�һ��
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
	// mysql_query()ִ�гɹ�����0��ʧ�ܷ��ط�0ֵ����PHP�в�һ��
	if (mysql_query(connection, sql.c_str()))
	{
		Trace << "Query Error:" << mysql_error(connection);
		logger->log(mysql_error(connection), ERROR);
		return nullptr;
	}
	else
	{
		result = mysql_store_result(connection); // ��ȡ�����
		//mysql_field_count()����connection��ѯ������
		if (mysql_num_rows(result)) return result;
		else return nullptr;
		//for (int i = 0; i < mysql_field_count(connection); ++i)
		//{
		//	// ��ȡ��һ��
		//	row = mysql_fetch_row(result);
		//	if (row <= 0)
		//	{
		//		break;
		//	}
		//	// mysql_num_fields()���ؽ�����е��ֶ���
		//	for (int j = 0; j < mysql_num_fields(result); ++j)
		//	{
		//		cout << row[j] << " ";
		//	}
		//	cout << endl;
		//}
	}
	//return true;
}