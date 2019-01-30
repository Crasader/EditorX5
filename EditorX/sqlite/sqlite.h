#pragma once

#include "sqlite3.h"
#include <string>
#include <list>
#include <vector>
#include "../StringUtils.h"

/**
 * ��sqlite�Ĳ�����װ
 * 20180126 lcc
 */
class SQLite {
protected:
	static HINSTANCE m_hInstDll;

public:
	//����sqlite dll����Ҫ�ظ����øú���
	static bool LoadSqliteDll(const StringUtils::T_CHAR* pDllPath);
	static void UnloadSqliteDll();

	SQLite();
	~SQLite();

	bool Connect(const StringUtils::T_CHAR* pDatabasePath);
	void Close();
	inline StringUtils::T_STRING GetLastError() {return m_errmsg;}

	/**
	 * ִ�з�select���
	 * @return ������Ӱ��ļ�¼��
	 */
	int Execute(const StringUtils::T_CHAR* sql);
	//������ִ�У�������֧��UTF8�ַ��� char*
	int ExecuteWithUTF8StrParam(const StringUtils::T_CHAR* sql,...);
	int ExecuteBatch(const std::vector<StringUtils::T_STRING>& sqls);

	/**
	 * ִ��select��ѯ
	 * @param sql[in] select sql
	 * @param rs[out] ��ѯ��������
	 * @return <0��ʾ��ѯʧ�ܣ���ͨ��GetLastError��ȡ������Ϣ��;�ɹ����ؽ����������
	 */
	int Query(const StringUtils::T_CHAR* sql,std::list<std::vector<std::pair<StringUtils::T_STRING,StringUtils::T_STRING>>>& rs);

protected:
	sqlite3 *m_pDB;
	StringUtils::T_STRING m_errmsg;

	/**
	 * ����sqlite3_exec�ĵ���������
	 * @param data ��Ӧsqlite3_exec�ĵ��ĸ�����
	 */
	static int QueryCallback(void* data, int n_columns, char** column_values,char** column_names);

	/**
	 * sqlite3������utf8���룬����Ҫ�Ƚ�unicode��ansiת��Ϊutf8
	 * @return ʹ�������Ҫdelete[]
	 */
	static std::string ConvertToUTF8(const StringUtils::T_CHAR* str);
	//@param str utf8�ַ���
	static StringUtils::T_STRING FromUTF8(const char* str);

////////////////////////////////////////////////
//����sqlite3 dll�ĺ��� 
protected:
	/**
	 * @param filename Database filename (UTF-8)
	 * @param ppDb[OUT] SQLite db handle
	 * @param flags SQLITE_OPEN_NOMUTEX: �������ݿ����������ڶ��߳�ģʽ(û��ָ�����߳�ģʽ�������)
     *              SQLITE_OPEN_FULLMUTEX���������ݿ����������ڴ���ģʽ��
     *              SQLITE_OPEN_SHAREDCACHE�����������ڹ�����ģʽ��
     *              SQLITE_OPEN_PRIVATECACHE�����������ڷǹ�����ģʽ��
     *              SQLITE_OPEN_READWRITE��ָ�����ݿ����ӿ��Զ�д��
     *              SQLITE_OPEN_CREATE��������ݿⲻ���ڣ��򴴽���
	 */
	static int (_stdcall *m_p_sqlite3_open_v2)(const char *filename,sqlite3 **ppDb, int flags,const char *zVfs);

	//�ر����ݿ����ӣ����ر�ʱ��������δ�ύ�����񣬸�������Զ��ع���
	static int (_stdcall *m_p_sqlite3_close_v2)(sqlite3*);

	/**
	 *
	 * @param sqlite3 An open database
	 * @param sql SQL to be evaluated,sql���԰������SQL������֮���ԷֺŸ���
	 * @param callback Callback function
	 * @param data 1st argument to callback
	 * @param errmsg Error msg written here ������sqlite3_free()������
     * @remark sqlite3_exec()����ִ�ж��SQL������Ǻ�������֤���񣬼���ִ�гɹ�����䣬������Ϊ����ִ��ʧ�ܵ������ع���
	 */
	static int (_stdcall *m_p_sqlite3_exec)(sqlite3*,const char *sql,int (*callback)(void*,int,char**,char**),void *data,char **errmsg);
	static void (_stdcall *m_p_sqlite3_free)(void*);

	/**
     * @param zSql SQL statement, UTF-8 encoded
     * @param nByte Maximum length of zSql in bytes
	 * @param ppStmt[out] Statement handle
	 * @param pzTail[out] Pointer to unused portion of zSql
	 */
	static int (_stdcall *m_p_sqlite3_prepare_v2)(sqlite3*,const char * zSql,int nByte,sqlite3_stmt **ppStmt,const char **pzTail);
	static int (_stdcall *m_p_sqlite3_step)(sqlite3_stmt*);
	static int (_stdcall *m_p_sqlite3_finalize)(sqlite3_stmt *pStmt);
	static const char* (_stdcall *m_p_sqlite3_errmsg)(sqlite3*);

	//�õ����Ӱ�������
	static int (_stdcall *m_p_sqlite3_changes)(sqlite3*);

	//��ȡ������sql�еĲ�������
	static int (_stdcall *m_p_sqlite3_bind_parameter_count)(sqlite3_stmt*);
	static int (_stdcall *m_p_sqlite3_bind_null)(sqlite3_stmt*, int);
	static int (_stdcall *m_p_sqlite3_bind_text)(sqlite3_stmt*,int,const char*,int,void(*)(void*));
	static int (_stdcall *m_p_sqlite3_reset)(sqlite3_stmt *pStmt);
};