#include "StdAfx.h"
#include "resource.h"
#include "EditorXFrame.h"
#include "ScintillaEdit.h"
#include "Lexer.h"
#include "StringUtils.h"
#include "./sqlite/sqlite.h"
#include "./config/fileRecord.h"
#include "./config/tempContentRecord.h"

#include <Commdlg.h>
//for ::ShellExecute
#pragma comment(lib,"Shell32.lib")
#include "Shellapi.h"

#include "WinSDKUtils.h"
#include "./directui/uibase.h"
#include "./directui/buttongroup.h"
#include "DlgAbout.h"
#include "DlgFind.h"

//����ģʽ
EditorXFrame EditorXFrame::m_instance;

EditorXFrame::EditorXFrame(void)
{
	m_hInst = NULL;
	m_hWnd = NULL;
	m_hScintillaDll = NULL;

	m_pUIFrame = nullptr;
	m_pMainTab = nullptr;
}


EditorXFrame::~EditorXFrame(void)
{
	if (::IsWindow(m_hWnd)) {
		::DestroyWindow(m_hWnd);
		m_hWnd = NULL;
	}
	if (m_hScintillaDll) {
		FreeLibrary(m_hScintillaDll);
		m_hScintillaDll = NULL;
	}
}

const std::wstring EditorXFrame::MAINWND_CLASSNAME = TEXT("lcc.EditorX5.Frame");
BOOL EditorXFrame::InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	m_hScintillaDll = ::LoadLibrary(_T(".\\bin\\SciLexer.dll"));
	if (m_hScintillaDll == NULL) {
		int err = GetLastError();
		assert(false);
		MessageBox(TEXT("����SciLexer.dllʧ��"));
		return FALSE;
	}

	//����sqlite
	if (!SQLite::LoadSqliteDll(TEXT(".\\bin\\sqlite3.dll"))) {
		MessageBox(TEXT("����sqlite3.dllʧ��"));
		return FALSE;
	}

	//ע�ᴰ����
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_BYTEALIGNWINDOW|CS_DBLCLKS; //CS_HREDRAW|CS_VREDRAW
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EDITORX_MAIN));
	wcex.hIconSm        = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EDITORX_MAIN));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName = NULL;//MAKEINTRESOURCE(IDC_EDITORX);
	wcex.lpszClassName	= MAINWND_CLASSNAME.c_str();
	RegisterClassEx(&wcex);

	//��������ܴ���
    m_hInst = hInstance;
    m_hWnd = CreateWindowEx(WS_EX_ACCEPTFILES
	  , wcex.lpszClassName, _T("EditorX5")
	  , WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN
	  , CW_USEDEFAULT, 0, CW_USEDEFAULT, 0
	  , NULL, NULL, hInstance, NULL);
    if (!m_hWnd)
    {
		int err = GetLastError();
	    assert(false);
        return FALSE;
    }

	//��ʼ���˵������������ʷ�ļ���¼�����ؿ�ݷ�ʽ�˵���
	m_menuFileHistory.Create(m_hInst, m_hWnd);
	m_menuShorcut.Create(m_hInst, m_hWnd);
	m_menuSymbol.Create(m_hInst, m_hWnd);

	m_pUIFrame = &(lcc_direct_ui::UIFrame::GetInstance());
	InitUIFrame();

	//����tabbar���Ҽ��˵�
	//m_tabBar.SetContextMenu(GetTabContextMenu());

    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);

	//������ʱ����
	TempContentRecord::GetInstance().ShowRecords();

	//����һ���ձ༭��
	CreateEditor();

    return TRUE;
}

void EditorXFrame::InitUIFrame()
{
	RECT rect;

	//����UI���
	::GetClientRect(m_hWnd, &rect);
	rect.bottom = (LONG)WinSDKUtils::GetAdaptationSizeForDPI(107);
	m_pUIFrame->Create(m_hInst, m_hWnd, rect);

	lcc_direct_ui::ButtonGroup *pBtnGroup = nullptr;
	lcc_direct_ui::Separator *pSeparator = nullptr;

	//�������ܰ�ť
	rect = { (int)(m_pUIFrame->GetRectLeft() + WinSDKUtils::GetAdaptationSizeForDPI(10))
		, (int)(m_pUIFrame->GetRectTop() + WinSDKUtils::GetAdaptationSizeForDPI(2))
		,0 ,0 };
	pBtnGroup = new lcc_direct_ui::ButtonGroup();
	pBtnGroup->Create(m_hWnd, rect, false, false
		, WinSDKUtils::GetAdaptationSizeForDPI(40)
		, WinSDKUtils::GetAdaptationSizeForDPI(40)
		, WinSDKUtils::GetAdaptationSizeForDPI(2)
	);
	pBtnGroup->AddButton(TEXT("btn_history.png"),TEXT("�ļ�����ʷ"), IDM_TOOL_FILEHISTORY);
	pBtnGroup->AddButton(TEXT("btn_open.png"), TEXT("���ļ�"), IDM_OPENFILE);
	pBtnGroup->AddButton(TEXT("btn_new.png"), TEXT("�� ��"), IDM_NEWFILE);
	pBtnGroup->AddButton(TEXT("btn_save.png"), TEXT("���浱ǰ�༭���е�����"), IDM_SAVEFILE);
	pBtnGroup->AddButton(TEXT("btn_saveas.png"), TEXT("��浱ǰ�༭���е�����"), IDM_SAVEASFILE);
	pBtnGroup->AddButton(TEXT("btn_reload.png"), TEXT("���¼��ص�ǰ�ļ�"), IDM_RELOADFILE);
	pBtnGroup->AddButton(TEXT("btn_search.png"), TEXT("����\\�滻"), IDM_FIND_REPLACE);
	pBtnGroup->AddButton(TEXT("btn_copypath.png"), TEXT("�����ļ�·����������"), IDM_COPYFILEPATH);
	pBtnGroup->AddButton(TEXT("btn_location.png"), TEXT("��λ�ļ�"), IDM_OPENFILEDIR);
	pBtnGroup->AddButton(TEXT("btn_stats.png"), TEXT("����ͳ��"), IDM_TOOL_WORDSTAT);
	pBtnGroup->AddButton(TEXT("btn_case_upper.png"), TEXT("�ַ���д"), IDM_EDIT_UPPERCASE);
	pBtnGroup->AddButton(TEXT("btn_case_lower.png"), TEXT("�ַ�Сд"), IDM_EDIT_LOWERCASE);
	pBtnGroup->AddButton(TEXT("btn_symbol.png"), TEXT("�������"), IDM_EDIT_SYMBOL);
	m_pUIFrame->AddUIObject(pBtnGroup);

	//�����Ҳ๦�ܰ�ť���İ�ť�����ڡ���ݷ�ʽ�ȣ�
	rect = { (int)(m_pUIFrame->GetRectRight() - WinSDKUtils::GetAdaptationSizeForDPI(120))
		, (int)(m_pUIFrame->GetRectTop() + WinSDKUtils::GetAdaptationSizeForDPI(5))
		,0 ,0 };
	pBtnGroup = new lcc_direct_ui::ButtonGroup();
	pBtnGroup->Create(m_hWnd, rect, false, false
		, WinSDKUtils::GetAdaptationSizeForDPI(50)
		, WinSDKUtils::GetAdaptationSizeForDPI(50)
		, WinSDKUtils::GetAdaptationSizeForDPI(2)
		, UIOBJ_ID_RIGHTBTNGROUP
	);
	pBtnGroup->AddButton(TEXT("btn_shortcuts.png"), TEXT("�������"), IDM_TOOL_SHORTCUT);
	pBtnGroup->AddButton(TEXT("btn_about.png"), TEXT("Ӧ����Ϣ"), IDM_ABOUT);
	pBtnGroup->ModifyBtnStyle(lcc_direct_ui::Button::BTN_STYLE::BS_ELLIPSE);
	m_pUIFrame->AddUIObject(pBtnGroup);

	//�����﷨��ɫ��ѡ����
	rect = { (int)(m_pUIFrame->GetRectLeft() + WinSDKUtils::GetAdaptationSizeForDPI(10))
		, (int)(m_pUIFrame->GetRectTop() + WinSDKUtils::GetAdaptationSizeForDPI(45))
		,0 ,0 };
	pBtnGroup = new lcc_direct_ui::ButtonGroup();
	pBtnGroup->Create(m_hWnd, rect, true, false
		, WinSDKUtils::GetAdaptationSizeForDPI(40)
		, WinSDKUtils::GetAdaptationSizeForDPI(25)
		, 0
		, UIOBJ_ID_RDOGROUPLEXER
	);
	pBtnGroup->AddButton(TEXT("�ı�"), IDM_LANG_NONE,0,true);
	pBtnGroup->AddButton(TEXT("C++"), IDM_LANG_C);
	pBtnGroup->AddButton(TEXT("Java"), IDM_LANG_JAVA);
	pBtnGroup->AddButton(TEXT("SQL"), IDM_LANG_SQL);
	pBtnGroup->AddButton(TEXT("XML"), IDM_LANG_XML);
	pBtnGroup->AddButton(TEXT("JSP"), IDM_LANG_JSP);
	pBtnGroup->AddButton(TEXT("HTML"), IDM_LANG_HTML);
	pBtnGroup->AddButton(TEXT("JS"), IDM_LANG_JS);
	pBtnGroup->ModifyBtnColor(Gdiplus::Color(255,70, 184, 218)
		, Gdiplus::Color(255, 49, 176, 213)
		, Gdiplus::Color(255, 50, 50, 50)
		, Gdiplus::Color(255, 0, 0, 0));
	m_pUIFrame->AddUIObject(pBtnGroup);

	//�����ָ��
	pSeparator = new lcc_direct_ui::Separator();
	pSeparator->Create(m_hWnd
		, { (LONG)(pBtnGroup->GetRectRight())
		, (LONG)(pBtnGroup->GetRectTop() + WinSDKUtils::GetAdaptationSizeForDPI(2)) }
		, size_t(pBtnGroup->GetRectHeight() - WinSDKUtils::GetAdaptationSizeForDPI(2) * 2)
	);
	m_pUIFrame->AddUIObject(pSeparator);

	//�����ı����뵥ѡ����
	rect = { (LONG)(pBtnGroup->GetRectRight() + WinSDKUtils::GetAdaptationSizeForDPI(5))
		, (LONG)(pBtnGroup->GetRectTop())
		,0 ,0 };
	pBtnGroup = new lcc_direct_ui::ButtonGroup();
	pBtnGroup->Create(m_hWnd, rect, true, false
		, WinSDKUtils::GetAdaptationSizeForDPI(60)
		, WinSDKUtils::GetAdaptationSizeForDPI(25)
		, 0
		, UIOBJ_ID_RDOGROUPCODE
	);
	pBtnGroup->AddButton(TEXT("ANSI"), IDM_CODE_ANSI, WinSDKUtils::GetAdaptationSizeForDPI(40), true);
	pBtnGroup->AddButton(TEXT("UTF-8"), IDM_CODE_UTF8, WinSDKUtils::GetAdaptationSizeForDPI(40));
	pBtnGroup->AddButton(TEXT("UTF-16LE"), IDM_CODE_UTF16LE);
	pBtnGroup->AddButton(TEXT("UTF-16BE"), IDM_CODE_UTF16BE);
	pBtnGroup->ModifyBtnColor(Gdiplus::Color(255, 70, 184, 218)
		, Gdiplus::Color(255, 49, 176, 213)
		, Gdiplus::Color(255, 50, 50, 50)
		, Gdiplus::Color(255, 0, 0, 0));
	m_pUIFrame->AddUIObject(pBtnGroup);

	//�����ָ��
	pSeparator = new lcc_direct_ui::Separator();
	pSeparator->Create(m_hWnd
		, { (LONG)(pBtnGroup->GetRectRight() + WinSDKUtils::GetAdaptationSizeForDPI(8))
		  , (LONG)(pBtnGroup->GetRectTop() + WinSDKUtils::GetAdaptationSizeForDPI(2)) }
		, size_t(pBtnGroup->GetRectHeight() - WinSDKUtils::GetAdaptationSizeForDPI(2)*2)
	);
	m_pUIFrame->AddUIObject(pSeparator);

	//������ѡ���飺�Զ����У�������ǰ����BOM��
	rect = { (LONG)(pBtnGroup->GetRectRight() + WinSDKUtils::GetAdaptationSizeForDPI(12))
		, (LONG)(pBtnGroup->GetRectTop())
		,0 ,0 };
	pBtnGroup = new lcc_direct_ui::ButtonGroup();
	pBtnGroup->Create(m_hWnd, rect, true, true
		, WinSDKUtils::GetAdaptationSizeForDPI(60)
		, WinSDKUtils::GetAdaptationSizeForDPI(25)
		, WinSDKUtils::GetAdaptationSizeForDPI(1)
		, UIOBJ_ID_CHKGROUP
	);
	pBtnGroup->AddButton(TEXT("��BOM"), IDM_CODE_BOMSIGN);
	pBtnGroup->AddButton(TEXT("�Զ�����"), IDM_WORDWRAP);
	pBtnGroup->AddButton(TEXT("������ǰ"), IDM_TOPMOST);
	pBtnGroup->ModifyBtnColor(Gdiplus::Color(255, 162, 219, 236)
		, Gdiplus::Color(255, 148, 214, 233)
		, Gdiplus::Color(255, 240, 240, 240)
		, Gdiplus::Color(255, 220, 220, 220));
	m_pUIFrame->AddUIObject(pBtnGroup);

	//����tabҳ
	rect = { 0
		,(LONG)(pBtnGroup->GetRectBottom() + WinSDKUtils::GetAdaptationSizeForDPI(2))
		,(LONG)m_pUIFrame->GetRectRight()
		,0 };
	rect.bottom = (LONG)(rect.top + WinSDKUtils::GetAdaptationSizeForDPI(32));
	m_pMainTab = new lcc_direct_ui::Tab();
	m_pMainTab->Create(m_hWnd, &rect, UIOBJ_ID_MAINTAB);
	m_pMainTab->SetCallbackAfterChangeTab(OnChangeTab);
	m_pMainTab->SetCallbackBeforeCloseTab(OnBeforeCloseTab);
	m_pMainTab->SetCallbackAfterCloseTab(OnAfterCloseTab);
	m_pMainTab->SetCallbackDblClk(OnTabDblClk);
	m_pUIFrame->AddUIObject(m_pMainTab);
}

int EditorXFrame::MessageBox(LPCTSTR lpText,LPCTSTR lpCaption/*=NULL*/,UINT uType/*=MB_OK*/)
{
	return ::MessageBox(m_hWnd,lpText,(lpCaption ? lpCaption : TEXT("EditorX����")),MB_OK);
}

BOOL EditorXFrame::Confirm(LPCTSTR lpText,LPCTSTR lpCaption/*=NULL*/)
{
	return ::MessageBox(m_hWnd,lpText,(lpCaption ? lpCaption : TEXT("EditorXȷ��")),MB_OKCANCEL) == IDOK;
}

LRESULT CALLBACK EditorXFrame::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	ScintillaEdit *pEditor = NULL;
	int tabIndex(-1);
	POINT pt;

	switch (message)
	{
	case WM_COMMAND:
		if (0 == lParam) {
			if (m_instance.DealMenuMsg(LOWORD(wParam))) {
				return 0;
			} else {
				return DefWindowProc(hWnd, message, wParam, lParam);
			}						
		} else {
			if (SCEN_CHANGE == HIWORD(wParam)) {//�༭���������޸���Ϣ
				if (m_instance.GetEditorFromHWND((HWND)lParam,&pEditor,tabIndex)) {
					if (pEditor->GetFilePath().length() < 1) {
						TempContentRecord::GetInstance().AddEditorForSave(pEditor);
					}
				} else {
					assert(false);
				}
			} else {//ȱʡ��Ϣ����
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}		
		break;
	case WM_NOTIFY://����༭����Ϣ
		switch (((LPNMHDR)lParam)->code) {
		case SCN_SAVEPOINTREACHED: //�༭�������ѱ���
			if (m_instance.GetEditorFromHWND(((LPNMHDR)lParam)->hwndFrom,&pEditor,tabIndex)) {
				std::wstring title = m_instance.m_pMainTab->GetItemTitle(tabIndex);
				if ('*' == title[0]) {
					title.erase(0,2);
					m_instance.m_pMainTab->SetItemTitle(tabIndex,title.c_str());
				}
				TempContentRecord::GetInstance().DeleteTempContent(pEditor);
			} else {
				assert(false);
			}
			break;
		case SCN_SAVEPOINTLEFT: //�༭�����ݴ�����
			if (m_instance.GetEditorFromHWND(((LPNMHDR)lParam)->hwndFrom,&pEditor,tabIndex)) {
				std::wstring title = m_instance.m_pMainTab->GetItemTitle(tabIndex);
				if ('*' != title[0]) {
					title = TEXT("* ")+title;
					m_instance.m_pMainTab->SetItemTitle(tabIndex,title.c_str());
				}
			} else {
				assert(false);
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		lcc_direct_ui::UIFrame::GetInstance().Draw(hdc,ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_SIZE:
		m_instance.OnResize(wParam, lParam);
		break;
	case WM_MOUSEMOVE:
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		if (lcc_direct_ui::UIFrame::GetInstance().PtInArea(pt)
			&& lcc_direct_ui::UIFrame::GetInstance().OnMouseMove(pt)) {
			return 0;
		}
		break;
	case WM_LBUTTONDOWN:
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		if (lcc_direct_ui::UIFrame::GetInstance().PtInArea(pt)
			&& lcc_direct_ui::UIFrame::GetInstance().OnLButtonDown(pt)) {
			return 0;
		}
		break;
	case WM_LBUTTONUP:
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		if (lcc_direct_ui::UIFrame::GetInstance().PtInArea(pt)
			&& lcc_direct_ui::UIFrame::GetInstance().OnLButtonUp(pt)) {
			return 0;
		}
		break;
	case WM_LBUTTONDBLCLK:
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		if (lcc_direct_ui::UIFrame::GetInstance().PtInArea(pt)
			&& lcc_direct_ui::UIFrame::GetInstance().OnLButtonDblClk(pt)) {
			return 0;
		}	
		return 0;
	case WM_RBUTTONDOWN:
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		if (lcc_direct_ui::UIFrame::GetInstance().PtInArea(pt)
			&& lcc_direct_ui::UIFrame::GetInstance().OnRButtonDown(pt)) {
			return 0;
		}
		break;
	case WM_DROPFILES: //�Ϸ��ļ�
		m_instance.OnDropFiles(HDROP(wParam));
		break;
	case WM_COPYDATA: //���ڽ��̼䴫�����ݣ����_tWinMain()/EditorX.cpp
		{
			COPYDATASTRUCT *pData = (COPYDATASTRUCT *)lParam;
			if (1 == pData->dwData) {
				unsigned char *pStr = new unsigned char[pData->cbData+2];
				::memcpy(pStr,pData->lpData,pData->cbData);
				pStr[pData->cbData] = 0;
				pStr[pData->cbData+1] = 0;
				m_instance.OpenFile((wchar_t*)pStr);
				delete[] pStr;
			}
		}
		break;
	case WM_CLOSE:
		if (m_instance.OnClose()) {
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void EditorXFrame::OnDropFiles(HDROP hDropInfo)
{
	//��ȡ�ļ�·��
	std::vector<std::wstring> files;
	const size_t file_count = ::DragQueryFile(hDropInfo,0xFFFFFFFF,nullptr,0);
	wchar_t pFileName[1000];
	for (size_t i=0; i<file_count; i++) {
		::DragQueryFile(hDropInfo,(UINT)i,pFileName,1000);
		files.push_back(pFileName);
	}
	::DragFinish(hDropInfo);

	//���ļ�
	DWORD attr(0);
	for (size_t i=0; i<file_count; i++) {
		attr = ::GetFileAttributes(files[i].c_str());
		if (attr == INVALID_FILE_ATTRIBUTES) {
			DWORD err = ::GetLastError();
			assert(false);
			continue;
		}
		if (attr&FILE_ATTRIBUTE_DEVICE || attr&FILE_ATTRIBUTE_DIRECTORY || attr&FILE_ATTRIBUTE_TEMPORARY) {
			continue;
		}
		OpenFile(files[i].c_str());
	}
}

BOOL EditorXFrame::OnClose()
{
	ScintillaEdit *pEditor;
	if (m_pMainTab->GetItemCount() > 0) {
		for (int i = (int)m_pMainTab->GetItemCount() - 1; i>-1; i--) {
			pEditor = (ScintillaEdit *)(m_pMainTab->GetItemData(i));
			if (pEditor->IsModified() && !(m_instance.Confirm(TEXT("�������޸ĵ���δ���棬ȷ�ϼ����رյ�ǰ�༭����")))) {
				return FALSE;
			}

			/* �ر�Ӧ��ʱ��ɾ��֮ǰ�������ʱ����
			 * OnBeforeCloseTab�л�ɾ����ʱ���ݣ����Խ�confirmBeforeClose����Ϊfalse
			*/
			m_pMainTab->CloseItem(i, false);
		}
	}	

	return TRUE;
}

void EditorXFrame::OnResize(WPARAM wParam, LPARAM lParam)
{
	if (!m_pMainTab) {
		return ;
	}

	switch (wParam) {
	case SIZE_RESTORED:
	case SIZE_MAXSHOW:
	case SIZE_MAXIMIZED:
		if (m_pUIFrame) {
			const intptr_t cx = LOWORD(lParam);
			const intptr_t offsetX = cx - (int)m_pUIFrame->GetRectWidth();
			const size_t minWidth = WinSDKUtils::GetAdaptationSizeForDPI(569);

			m_pUIFrame->Resize(cx, -1,false);
			if (offsetX != 0 && m_pUIFrame->GetRectWidth() > minWidth) {
				//�����Ҳఴť���λ��
				lcc_direct_ui::UIBase* pRightBtnGroup = m_pUIFrame->GetUIObjectByID(UIOBJ_ID_RIGHTBTNGROUP);
				assert(pRightBtnGroup);
				pRightBtnGroup->Offset(offsetX, 0);
				//����Tab��λ��
				m_pMainTab->Resize(cx, -1, false);
				//�ػ洰��
				m_pUIFrame->Redraw();
			}

			//�������б༭����λ��
			ScintillaEdit* pEditor = nullptr;
			RECT rcEditor;
			GetEditorRect(rcEditor);
			for (int i = 0; i < m_pMainTab->GetItemCount(); i++) {
				pEditor = (ScintillaEdit*)m_pMainTab->GetItemData(i);
				pEditor->MoveWindow(&rcEditor);
			}

			//����editor���ڻ�ý���
			pEditor = (ScintillaEdit*)m_pMainTab->GetActiveItemData();
			if (pEditor) {
				//���ø������Ч���²�֮������Ϣ������WM_SETFOCUS������ʹ��PostMessage
				//pEditor->SetFocus();
				::PostMessage(pEditor->GetHWnd(), WM_SETFOCUS, 0, 0);
			}
		}
		break;
	}
}

//����ʶ��ò˵�ID�����˴����򷵻�true�����򷵻�false
bool EditorXFrame::DealMenuMsg(int menu_id) {
	ScintillaEdit *pEditor = NULL;
	std::wstring str_temp;
	int result(0);

	switch (menu_id) {
	//////////////////////////////////////////////////////////////////
	//�ļ�
	case IDM_OPENFILE: //���ļ�
		OpenFile();
		return true;
	case IDM_SAVEASFILE://���Ϊ
		pEditor = (ScintillaEdit *)(m_pMainTab->GetActiveItemData());
		pEditor->SaveFile(TRUE);
		return true;
	case IDM_RELOADFILE://���¼���
		pEditor = (ScintillaEdit *)(m_pMainTab->GetActiveItemData());
		if (pEditor->GetFilePath().length() < 1) {
			m_pUIFrame->Hint(TEXT("��δ���ļ�"));
			return true;
		}
		if (!(pEditor->IsModified())) {//����ļ���δ�޸ģ�����ָ�����ļ�����򿪸��ļ�
			pEditor->OpenFile(pEditor->GetFilePath().c_str(),pEditor->GetCharset());
		} else {
			pEditor->OpenFile(pEditor->GetFilePath().c_str());
		}
		return true;
	case IDM_SAVEFILE: //�����ļ�
		pEditor = (ScintillaEdit *)(m_pMainTab->GetActiveItemData());
		pEditor->SaveFile();
		return true;
	case IDM_NEWFILE:  //�½��ļ�
		CreateEditor();
		return true;
	case IDM_EXIT: //Ӧ���˳�
		::DestroyWindow(m_hWnd);
		return true;
	case IDM_FIND_REPLACE:
		pEditor = (ScintillaEdit *)(m_pMainTab->GetActiveItemData());
		if (pEditor) {
			DlgFind::GetInstance().ShowInModeless(pEditor->GetSelectedText());
		}
		else {
			m_pUIFrame->Hint(TEXT("��ǰ�޻�ı༭��"));
		}
		break;

	//////////////////////////////////////////////////////////////////
	//�鿴
	case IDM_TOPMOST: //������ǰ
		if (::GetWindowLong(m_hWnd,GWL_EXSTYLE)&WS_EX_TOPMOST) {
			::SetWindowPos(m_hWnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		} else {
			::SetWindowPos(m_hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		}
		::CheckMenuItem(GetMenu(m_hWnd),IDM_TOPMOST
			,((::GetWindowLong(m_hWnd,GWL_EXSTYLE)&WS_EX_TOPMOST) ? (MF_BYCOMMAND|MF_CHECKED) : (MF_BYCOMMAND|MF_UNCHECKED)));
		return true;
	case IDM_WORDWRAP: //�Զ�����
		pEditor = (ScintillaEdit *)(m_pMainTab->GetActiveItemData());
		pEditor->SetWordWrap(!(pEditor->GetWordWrap()));
		return true;
	case IDM_COPYFILEPATH://�����ļ�·��
		pEditor = (ScintillaEdit *)(m_pMainTab->GetActiveItemData());
		str_temp = pEditor->GetFilePath();
		if (str_temp.length() < 1) {
			m_pUIFrame->Hint(TEXT("��δ���ļ�"));
		} else {
			if (::OpenClipboard(m_hWnd)) {
				::EmptyClipboard();
				HGLOBAL hContent = GlobalAlloc(GMEM_MOVEABLE, (str_temp.length()+1)*sizeof(TCHAR));
				LPTSTR pConent = (LPTSTR)::GlobalLock(hContent);
				::memcpy(pConent,str_temp.c_str(),str_temp.length()*sizeof(TCHAR));
				pConent[str_temp.length()] = 0;
				::GlobalUnlock(hContent);
				::SetClipboardData(CF_UNICODETEXT,hContent);
				::CloseClipboard();
				::GlobalFree(hContent);
				m_pUIFrame->Hint(TEXT("�ļ�·���Ѹ�����������"));
			} else {
				assert(false);
				m_pUIFrame->Hint(TEXT("�򿪼�����ʧ��"));
			}			
		}
		return true;
	case IDM_OPENFILEDIR: //���ļ�λ��
		pEditor = (ScintillaEdit *)(m_pMainTab->GetActiveItemData());
		str_temp = pEditor->GetFilePath();
		if (str_temp.length() < 1) {
			m_pUIFrame->Hint(TEXT("��δ���ļ�"));
		} else {
			str_temp = TEXT("/e,/select,")+str_temp;
			ShellExecute(m_hWnd,TEXT("open"),TEXT("explorer"),str_temp.c_str(),NULL,SW_SHOW);
		}
		return true;
	case IDM_EDIT_UPPERCASE:
	case IDM_EDIT_LOWERCASE:
		pEditor = (ScintillaEdit *)(m_pMainTab->GetActiveItemData());
		if (pEditor) {
			if (IDM_EDIT_UPPERCASE == menu_id)
				pEditor->MakeUpperCase();
			else
				pEditor->MakeLowerCase();
		}
		else {
			m_pUIFrame->Hint(TEXT("��ǰ�޻�ı༭�����޷�����"));
		}
		return true;
	case IDM_EDIT_SYMBOL: //�������
		m_menuSymbol.Popup();
		return true;

	case IDM_LANG_NONE:
	case IDM_LANG_C:
	case IDM_LANG_JAVA:
	case IDM_LANG_XML:
	case IDM_LANG_SQL:
	case IDM_LANG_JSP:
	case IDM_LANG_HTML:
	case IDM_LANG_JS:   
		//�﷨��ɫ
		pEditor = (ScintillaEdit *)(m_pMainTab->GetActiveItemData());
		assert(pEditor);
		result = pEditor->SetLanguage(menu_id-IDM_LANG_NONE);
		//UpdateMainMenuStatus(pEditor);
		return true;
	//////////////////////////////////////////////////////////////////
	//����
	case IDM_CODE_ANSI:
	case IDM_CODE_UTF8:
	case IDM_CODE_UTF16LE:
	case IDM_CODE_UTF16BE:
		pEditor = (ScintillaEdit *)(m_pMainTab->GetActiveItemData());
		assert(pEditor);
		pEditor->SetCharset(menu_id-IDM_CODE_ANSI);
		//UpdateMainMenuStatus(pEditor);
		return true;
	case IDM_CODE_BOMSIGN:
		pEditor = (ScintillaEdit *)(m_pMainTab->GetActiveItemData());
		assert(pEditor);
		pEditor->SetContainBOM(!(pEditor->IsContainBOM()));
		return true;
	//////////////////////////////////////////////////////////////////
	//����
	case IDM_TOOL_WORDSTAT: //����ͳ��
		pEditor = (ScintillaEdit *)(m_pMainTab->GetActiveItemData());
		if (pEditor) {
			std::vector<intptr_t> result; //0-�ַ����룻 1-�ַ����� 2-�ֽ���
			pEditor->WordStat(result);

			std::wstring msg = StringUtils::format(TEXT("�ַ�����%d\r\n�ֽ�����%d\r\n�ַ����룺")
				, result[1], result[2]);
			if (ScintillaEdit::CHARSET_ANSI == result[0]) {
				msg += TEXT("ANSI");
			}
			else if (ScintillaEdit::CHARSET_UTF8 == result[0]) {
				msg += TEXT("UTF8");
			}
			else if (ScintillaEdit::CHARSET_UTF16LE == result[0]) {
				msg += TEXT("UTF16LE");
			}
			else if (ScintillaEdit::CHARSET_UTF16BE == result[0]) {
				msg += TEXT("UTF16BE");
			}
			else {
				msg += TEXT("δ֪");
			}

			m_pUIFrame->Hint(msg);
		}
		else {
			m_pUIFrame->Hint(TEXT("��ǰ�޻�ı༭�����޷�ͳ��"));
		}
		
		break;
	case IDM_ABOUT: //����about�Ի���
		DlgAbout::GetInstance().ShowInModeless();
		return true;
	case IDM_TOOL_SHORTCUT: //������ݷ�ʽ�˵�
		m_menuShorcut.Popup();
		break;
	case IDM_TOOL_FILEHISTORY://�ļ���ʷ��¼
		m_menuFileHistory.Popup(WinSDKUtils::GetAdaptationSizeForDPI(2)
			, m_pUIFrame->GetRectBottom()+ WinSDKUtils::GetAdaptationSizeForDPI(2));
		break;
	}
	return false;
}

void EditorXFrame::EnsureOneTabAtLeast()
{
	if (m_pMainTab->GetItemCount() < 1) {
		CreateEditor();
	}
}

void EditorXFrame::UpdateAppTitle(ScintillaEdit *pEditor/*=NULL*/)
{
	if (!pEditor) {
		pEditor = (ScintillaEdit*)(m_pMainTab->GetActiveItemData());
	}
	if (!pEditor) {
		SetTitle(TEXT("EditorX5"));
		return ;
	}

	if (pEditor->GetFilePath().length() < 1) {
		SetTitle(TEXT("EditorX5"));
	} else {
		SetTitle(pEditor->GetFilePath().c_str());
	}
}

void EditorXFrame::UpdateTabTitle(ScintillaEdit *pEditor/*=NULL*/)
{
	intptr_t index(-1);
	if (pEditor) {
		index = m_pMainTab->GetItemIndexByUserData((intptr_t)pEditor);
	} else {
		pEditor = (ScintillaEdit *)m_pMainTab->GetActiveItemData();
	}
	if (index > -1 && pEditor && pEditor->GetFilePath().length() > 0) {
		m_pMainTab->SetItemTitle(index,pEditor->GetFileName());
	}
}

BOOL EditorXFrame::UpdateMainMenuStatus(ScintillaEdit *pEditor/*=NULL*/)
{
	if (!pEditor) {
		pEditor = (ScintillaEdit*)(m_pMainTab->GetActiveItemData());
	}
	if (!pEditor) {
		return FALSE;
	}

	const int langType = pEditor->GetLanguage();
	const ScintillaEdit::CHARSET charset = pEditor->GetCharset();
	const bool containBOM = pEditor->IsContainBOM();
	const bool wordWrap = pEditor->GetWordWrap();

	//�����﷨��ɫ�˵�״̬
	lcc_direct_ui::ButtonGroup* pBtnGroup = nullptr;
	pBtnGroup = (lcc_direct_ui::ButtonGroup*)lcc_direct_ui::UIFrame::GetInstance().GetUIObjectByID(UIOBJ_ID_RDOGROUPLEXER);
	pBtnGroup->SetCheckedById(IDM_LANG_NONE+ langType,true);
	//�����ַ�����˵�״̬
	pBtnGroup = (lcc_direct_ui::ButtonGroup*)lcc_direct_ui::UIFrame::GetInstance().GetUIObjectByID(UIOBJ_ID_RDOGROUPCODE);
	pBtnGroup->SetCheckedById(IDM_CODE_ANSI+charset,true);
	//���¡��Ƿ�Я��BOM��״̬
	pBtnGroup = (lcc_direct_ui::ButtonGroup*)lcc_direct_ui::UIFrame::GetInstance().GetUIObjectByID(UIOBJ_ID_CHKGROUP);
	pBtnGroup->SetCheckedById(IDM_CODE_BOMSIGN, containBOM);	
	pBtnGroup->SetCheckedById(IDM_WORDWRAP, wordWrap);//�����Ƿ��Զ�����

	return TRUE;
}

void EditorXFrame::SetTitle(LPCTSTR title/*=NULL*/)
{
	if (::IsWindow(m_hWnd)) {
		::SetWindowText(m_hWnd,title ? title : TEXT("EditorX 5"));
	}
}

BOOL EditorXFrame::OpenFile(LPCTSTR file/*=NULL*/)
{
	std::wstring file_path;
	if (file) {
		file_path = file;
	} else {
		OPENFILENAME ofn;
		memset(&ofn,0,sizeof(ofn));
		ofn.lStructSize     = sizeof( ofn );
		ofn.hwndOwner       = m_hWnd;
		ofn.lpstrFile       = new TCHAR[1000];
		ofn.lpstrFile[0]    = 0;
		ofn.nMaxFile        = 1000;
		ofn.lpstrFilter     = TEXT("All files (*.*)\0*.*\0");
		ofn.nFilterIndex    = 0;
		ofn.lpstrFileTitle  = NULL;
		ofn.nMaxFileTitle   = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.lpstrDefExt     = TEXT("");
		ofn.Flags           = OFN_EXPLORER | OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_FORCESHOWHIDDEN;
		ofn.lpstrTitle      = TEXT("ѡ���ļ�");
		if (::GetOpenFileName(&ofn)) {
			file_path = ofn.lpstrFile;
			delete []ofn.lpstrFile;
		} else {
			int err = CommDlgExtendedError();
			delete []ofn.lpstrFile;
			return FALSE;
		}
	}

	//��ȡ����ʾ�ļ�
	intptr_t editorIndex = m_pMainTab->GetActiveIndex();
	ScintillaEdit *pEditor = editorIndex > -1 ? (ScintillaEdit *)(m_pMainTab->GetItem(editorIndex)->userData) : nullptr;
	if (pEditor
		&& pEditor->GetFilePath().length() < 1 
		&& !pEditor->IsModified() 
		&& m_pMainTab->GetItemTitle(editorIndex).compare(TEXT("new")) == 0) {
	} else {
		pEditor = CreateEditor();
		editorIndex = m_pMainTab->GetActiveIndex();
	}
	DWORD result = pEditor->OpenFile(file_path.c_str());
	if (result != 0) {
		std::wstring hint = StringUtils::format(TEXT("���ļ�ʧ�ܣ��������Ϊ��%d��"),result);
		MessageBox(hint.c_str());
		return FALSE;
	}

	//���³����������tabҳ�ı���
	SetTitle(pEditor->GetFilePath().c_str());
	m_pMainTab->SetItemTitle(editorIndex, pEditor->GetFileName());

	//���²˵�״̬
	UpdateMainMenuStatus();

	//��¼�ļ�����ʷ
	FileRecord conf;
	conf.AddRecord(file_path.c_str());

	return TRUE;
}

void EditorXFrame::GetEditorRect(RECT& rect)
{
	::GetClientRect(m_hWnd, &rect);
	rect.top = (LONG)m_pUIFrame->GetRectBottom();
}

ScintillaEdit* EditorXFrame::CreateEditor(LPCTSTR title/*=NULL*/)
{
	//����һ���༭��
	RECT rc;
	GetEditorRect(rc);
	ScintillaEdit *pEditor = new ScintillaEdit();
	pEditor->Create(m_hInst,m_hWnd,&rc);

	//Ϊ�ñ༭������һ��tabҳ����
	const lcc_direct_ui::Tab::ITEM *pTabItem = m_pMainTab->AddItem(
		NULL == title ? TEXT("new") : title
		, (intptr_t)pEditor
		, false
	);

	//���ǰtab��
	const size_t index = m_pMainTab->GetItemIndexByID(pTabItem->uuid);
	m_pMainTab->SetActiveItem(index);

	return pEditor;
}

ScintillaEdit* EditorXFrame::GetActiveEditor()
{
	return (ScintillaEdit*)m_pMainTab->GetActiveItemData();
}

void EditorXFrame::ShowEditor(bool show)
{
	GetActiveEditor()->ShowWindow(show);
}

BOOL EditorXFrame::GetEditorFromHWND(const HWND& hWnd,ScintillaEdit** ppEditor,int& tabIndex)
{
	ScintillaEdit* pEditor = NULL;
	for (int i = 0; i < m_pMainTab->GetItemCount(); i++) {
		pEditor = (ScintillaEdit*)m_pMainTab->GetItemData(i);
		if (pEditor->GetHWnd() == hWnd) {
			*ppEditor = pEditor;
			tabIndex = i;
			return TRUE;
		}
	}
	*ppEditor = NULL;
	tabIndex = -1;
	return FALSE;
}

intptr_t EditorXFrame::OnBeforeCloseTab(intptr_t index, intptr_t userData)
{
	assert(index >= 0);

	ScintillaEdit *pEditor = (ScintillaEdit *)userData;
	assert(pEditor);
	if (pEditor->IsModified()) {
		std::wstring title = m_instance.m_pMainTab->GetItemTitle(index);
		std::wstring hint = StringUtils::format(TEXT("��%s���������޸ĵ���δ���档\r\n    ȷ�ϼ����رգ�"), title.c_str());
		m_instance.m_pMainTab->SetActiveItem(index);
		if (!(m_instance.Confirm(hint.c_str()))) {
			return FALSE;
		}
	}

	//ɾ��֮ǰ�������ʱ����
	TempContentRecord::GetInstance().DeleteTempContent(pEditor);
	return TRUE;
}

intptr_t EditorXFrame::OnAfterCloseTab(intptr_t index, intptr_t userData)
{
	assert(index >= 0);
	//ע���༭������
	ScintillaEdit *pEditor = (ScintillaEdit *)userData;
	assert(pEditor);
	delete pEditor;
	return TRUE;
}

intptr_t EditorXFrame::OnTabDblClk(intptr_t pPoint, intptr_t reserve)
{
	EditorXFrame::GetInstance().CreateEditor();
	return 0;
}

//tabҳ�л�����Ӧ�¼�
intptr_t EditorXFrame::OnChangeTab(intptr_t active, intptr_t preActive)
{
	ScintillaEdit *pEditor;

	//����֮ǰ�Ļ����
	if (preActive > -1) {
		pEditor = (ScintillaEdit *)m_instance.m_pMainTab->GetItemData(preActive);
		pEditor->ShowWindow(FALSE);
	}

	//��ʾ��ǰtabҳ�µ��Ӵ���
	pEditor = (ScintillaEdit *)m_instance.m_pMainTab->GetItemData(active);	
	pEditor->ShowWindow(TRUE);
	pEditor->SetFocus();

	m_instance.UpdateMainMenuStatus(pEditor);
	m_instance.UpdateAppTitle(pEditor);
	return TRUE;
}

std::wstring EditorXFrame::ParseCmdLine(LPCTSTR lpCmdLine)
{
	std::wstring filepath = lpCmdLine;
	if (filepath.length() > 0) {
		filepath = StringUtils::rtrim(filepath,'"');
		filepath = StringUtils::ltrim(filepath,'"');

		DWORD attr = ::GetFileAttributes(filepath.c_str());
		if (attr == INVALID_FILE_ATTRIBUTES || attr&FILE_ATTRIBUTE_DEVICE || attr&FILE_ATTRIBUTE_DIRECTORY || attr&FILE_ATTRIBUTE_TEMPORARY) {
			filepath.empty();
		}
	}
	return filepath;
}