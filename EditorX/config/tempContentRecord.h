#pragma once
#include "configBase.h"
#include<list>

class ScintillaEdit;
class TempContentRecord : public ConfigBase{
public:
	~TempContentRecord();
	static TempContentRecord& GetInstance() {return m_instance;}
	
	/**
	 * �༭���򿪺���øú����Ѵ�֮ǰ����ʱ�ı���¼
	 * @return �������ʱ��¼�ҳɹ����򷵻�true,���򷵻�false
	 */
	bool ShowRecords();

	void AddEditorForSave(ScintillaEdit* pEditor);
	void DeleteTempContent(ScintillaEdit* pEditor);

protected:
	//����ģʽ
	static TempContentRecord m_instance;
	TempContentRecord();

	//��ʱ���ݵ�������
	static const int CONTENT_MAX_COUNT;
	//��Ҫ������ʱ���ݵı༭��
	ScintillaEdit** m_arrEditorForSave;
	//�ٽ�����Ϊ���̰߳�ȫ
	CRITICAL_SECTION m_criticalSection;

	//��ʱ���ݵı����߳�
	HANDLE m_hSaveThread;
	static DWORD WINAPI SaveProcThread(LPVOID lpParam);
};
