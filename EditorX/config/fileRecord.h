#pragma once
#include <string>
#include <vector>
#include "configBase.h"

class FileRecord : public ConfigBase{
public:
	~FileRecord();

	/**
	 * ��ȡ�����򿪵��ļ���¼
	 * @param arrRecord[out] �����򿪵��ļ���¼
	 * @return �������ļ�������
	 */
	int GetRecords(std::vector<std::wstring>& arrRecord);

	//ɾ���ļ���ʷ��¼
	bool DeleteRecord(std::wstring file);

	//��¼�����򿪵��ļ�·��
	bool AddRecord(std::wstring file);
};