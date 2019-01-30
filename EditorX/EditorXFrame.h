#pragma once
#include <vector>
#include "./directui/tab.h"
#include "./directui/UIFrame.h"
#include "MenuShorcut.h"
#include "MenuFileHistory.h"
#include "MenuSymbol.h"

//dor dropfiels
#include "Shellapi.h"
#pragma comment(lib,"Shell32.lib")

class ScintillaEdit;
class EditorXFrame
{
public:
	~EditorXFrame(void);

	//����ģʽ
	static EditorXFrame& GetInstance() {return m_instance;}
	static std::wstring GetMainWndClassName() {return MAINWND_CLASSNAME;}
	static std::wstring ParseCmdLine(LPCTSTR lpCmdLine);

	HINSTANCE GetAppInstance() const {return m_hInst;}
	HWND GetHWnd() const {return this==NULL ? NULL : m_hWnd;}

	//ϵͳ��ʼ��
	BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);

	int MessageBox(LPCTSTR lpText,LPCTSTR lpCaption=NULL,UINT uType=MB_OK);
	BOOL Confirm(LPCTSTR lpText,LPCTSTR lpCaption=NULL);

	void SetTitle(LPCTSTR title=NULL);
	BOOL OpenFile(LPCTSTR file=NULL);

	//����Ӧ�ñ�����
	void UpdateAppTitle(ScintillaEdit *pEditor = NULL);
	void UpdateTabTitle(ScintillaEdit *pEditor = NULL);
	//���ݵ�ǰ����ڸ��²˵����״̬
	BOOL UpdateMainMenuStatus(ScintillaEdit *pEditor = NULL);

	//ȷ�����ٴ���һ��tabҳ
	void EnsureOneTabAtLeast();

	//�½�һ���༭��tabҳ���ڣ�new�ı༭�����ڶ������m_tabBar.DeleteTab()���Զ�ע��
	ScintillaEdit* CreateEditor(LPCTSTR title=NULL);
	ScintillaEdit* GetActiveEditor();
	/**
     * ͨ���༭����HWND��ȡ�����Ϣ
     * @param hWnd[in] �����ҵı༭����HWND
     * @param ppEditor[out] hWnd��Ӧ�ı༭���Ķ���ʵ��ָ�룬�������
     * @param tabIndex[out] hWnd��Ӧ�ı༭��������tabҳ���������������
     * @return �����ɹ�������TRUE
     */
	BOOL GetEditorFromHWND(const HWND& hWnd,ScintillaEdit** ppEditor,int& tabIndex);

	void ShowEditor(bool show);

protected:
	//UI����ID����
	enum :size_t {
		UIOBJ_ID_RDOGROUPLEXER = 1 //�﷨��ɫ��ѡ����
		, UIOBJ_ID_RDOGROUPCODE    //�ı����뵥ѡ����
		, UIOBJ_ID_CHKGROUP        //��ѡ���飺"��BOM","�Զ�����","������ǰ"��
		, UIOBJ_ID_RIGHTBTNGROUP   //�Ҳఴť���ID,����ݡ��������ڡ��Ȱ�ť
		, UIOBJ_ID_MAINTAB         //��tab
	};

	//����ģʽ
	static EditorXFrame m_instance;
	EditorXFrame(void);

	static const std::wstring MAINWND_CLASSNAME;
	
	HINSTANCE m_hInst; //Ӧ�õ���ʵ�����	
	HWND m_hWnd;       //Ӧ�õ������ڵľ��	
	HMODULE m_hScintillaDll; //�༭��DLL�ľ��

	//directUI��ܣ�����ע����ָ��
	lcc_direct_ui::UIFrame *m_pUIFrame;
	lcc_direct_ui::Tab *m_pMainTab;

	//��ݷ�ʽ�˵�
	MenuShorcut m_menuShorcut;
	//�ļ���ʷ�˵�
	MenuFileHistory m_menuFileHistory;
	//����ѡ��
	MenuSymbol m_menuSymbol;

	void InitUIFrame();
	void OnResize(WPARAM wParam, LPARAM lParam);
    //@return TRUE-���Լ����رգ�FALSE-�����Թر�
    BOOL OnClose();
	//�����ܵ�����Ϣ��Ӧ
	static LRESULT CALLBACK	WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void GetEditorRect(RECT& rect);

	/**
	 * ����˵���Ϣ
	 * @param menu_id �˵�ID
	 * @return ����ʶ��ò˵�ID�����˴����򷵻�true�����򷵻�false
	 */
	bool DealMenuMsg(int menu_id);

	//tabҳ�ر�ǰ����Ӧ�¼���������μ�TabCtrl::m_pCallbackBeforeCloseTab��ע��
	static intptr_t OnBeforeCloseTab(intptr_t index, intptr_t userData);
	//tabҳ�رպ����Ӧ�¼���������μ�TabCtrl::m_pCallbackAfterCloseTab��ע��
	static intptr_t OnAfterCloseTab(intptr_t index, intptr_t userData);
	//tabҳ�л�����Ӧ�¼���������μ�TabCtrl::m_pCallbackAfterChangeTab��ע��
	static intptr_t OnChangeTab(intptr_t active, intptr_t preActive);
	//Tab˫���¼��Ļص�����������˫���հ�������Tab�ʱ�Ż���øú�����������μ�TabCtrl::m_pCallbackDblClk��ע��
	static intptr_t OnTabDblClk(intptr_t pPoint, intptr_t reserve);
	void OnDropFiles(HDROP hDropInfo);
};