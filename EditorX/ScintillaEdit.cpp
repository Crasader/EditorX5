#include "StdAfx.h"
#include "ScintillaEdit.h"
#include "EditorXFrame.h"
#include "Lexer.h"
#include <Commdlg.h>
#include "WinSDKUtils.h"
#include "DlgFind.h"
#include "./config/fileRecord.h"

int ScintillaEdit::m_ref_count = 0;
std::vector<HMENU> ScintillaEdit::m_arrHPopupMenu;

ScintillaEdit::ScintillaEdit(void):BaseWnd()
{
	m_ref_count++;

	m_pScintillaFunc = NULL;
	m_pScintillaObj = NULL;

	m_containBOM = true;
	m_charset = CHARSET_ANSI;
	m_file_charset = CHARSET_ANSI;
	m_langType = Lexer::LANG_NONE;
}

ScintillaEdit::~ScintillaEdit(void)
{
	BaseWnd::~BaseWnd();

	m_ref_count--;
	if (m_ref_count < 1) {
		for (int i=0; i<m_arrHPopupMenu.size(); i++) {
			::DestroyMenu(m_arrHPopupMenu[i]);
		}
		m_arrHPopupMenu.clear();
	}
}

HMENU ScintillaEdit::GetContextMenu()
{
	if (m_arrHPopupMenu.size() < 1) { //����Ҽ��˵���δ�������򴴽�֮
		HMENU hContenxtMenu(NULL);

		//������һ���˵�
		hContenxtMenu = ::CreatePopupMenu();
		m_arrHPopupMenu.push_back(hContenxtMenu);

		//����һ���˵��е�����
		::AppendMenu(hContenxtMenu,MF_STRING,IDM_EDITOR_UNDO,TEXT("����  Ctrl+Z"));
		::AppendMenu(hContenxtMenu,MF_STRING,IDM_EDITOR_REDO,TEXT("�ָ�  Ctrl+Y"));
		::AppendMenu(hContenxtMenu,MF_SEPARATOR,0,0);
		::AppendMenu(hContenxtMenu,MF_STRING,IDM_EDITOR_SELALL,TEXT("ȫѡ  Ctrl+A"));
		::AppendMenu(hContenxtMenu,MF_STRING,IDM_EDITOR_COPY,TEXT("����  Ctrl+C"));
		::AppendMenu(hContenxtMenu,MF_STRING,IDM_EDITOR_CUT,TEXT("����  Ctrl+X"));
		::AppendMenu(hContenxtMenu,MF_STRING,IDM_EDITOR_PASTE,TEXT("ճ��  Ctrl+V"));
		::AppendMenu(hContenxtMenu,MF_STRING,IDM_EDITOR_DELETE,TEXT("ɾ��"));
		::AppendMenu(hContenxtMenu,MF_SEPARATOR,0,0);
		::AppendMenu(hContenxtMenu,MF_STRING,IDM_EDITOR_CASEUPPER,TEXT("�ַ���д Ctrl+Shift+A"));
		::AppendMenu(hContenxtMenu,MF_STRING,IDM_EDITOR_CASELOWER,TEXT("�ַ�Сд Ctrl+Shift+Z"));
		::AppendMenu(hContenxtMenu,MF_STRING,IDM_EDITOR_CLEARNULLROW,TEXT("�������"));
		::AppendMenu(hContenxtMenu,MF_STRING,IDM_EDITOR_CLEARROWNUM,TEXT("����к�"));
	}
	return m_arrHPopupMenu[0];
}

BOOL ScintillaEdit::Create(HINSTANCE hInstance,HWND hParent,const RECT *p_rect/*=NULL*/)
{
	RECT rc = {0};
	if (p_rect) {
		::memcpy(&rc,p_rect,sizeof(RECT));
	}
	m_hWnd = ::CreateWindow(TEXT("Scintilla"), TEXT("editor")
		, WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN | WS_EX_RTLREADING | WS_VISIBLE
		,rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top
		,hParent,NULL,hInstance,NULL);
	if (m_hWnd) {
		//���Զ������Ϣ���������tab����Ĭ�ϵĴ�����
		::SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
		m_pDefaultWndProc = reinterpret_cast<WNDPROC>(::SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));

		m_pScintillaFunc = (SCINTILLA_FUNC *)::SendMessage(m_hWnd, SCI_GETDIRECTFUNCTION, 0, 0);
		m_pScintillaObj = (SCINTILLA_PTR)::SendMessage(m_hWnd, SCI_GETDIRECTPOINTER, 0, 0);

		//ͳһ�ַ�����ΪUTF8
		ExecuteScintilla(SCI_SETCODEPAGE,CP_UTF8);

		//��TAB�����Ĭ�ϵ�8��Ϊ4
		ExecuteScintilla(SCI_SETTABWIDTH, 4);

		//�����к��еĿ��
		ExecuteScintilla(SCI_SETMARGINTYPEN,0,SC_MARGIN_NUMBER);
		ExecuteScintilla(SCI_SETMARGINWIDTHN,0,LPARAM(WinSDKUtils::GetAdaptationSizeForDPI(40)));

		//��ֹ�༭����ʾ�����ʾһ�����������
		ExecuteScintilla(SCI_SETSCROLLWIDTH, WinSDKUtils::GetAdaptationSizeForDPI(900));

		/* ����Ĭ������
		 * Courier New �� Windows ��ȱʡ�ȿ�����
		 * �ȿ�������������ʱ�ܹ�׼ȷ����ÿ���ַ����Լ����ڶ�����������
		 */
		ExecuteScintilla(SCI_STYLESETFONT, STYLE_DEFAULT,(LPARAM)(TEXT("Courier New")));
		ExecuteScintilla(SCI_STYLESETSIZE, STYLE_DEFAULT,12);

		//���õ�ǰ�б���ɫ
		ExecuteScintilla(SCI_SETCARETLINEVISIBLE, TRUE);
		ExecuteScintilla(SCI_SETCARETLINEBACK, RGB(232,242,254));
	}
	assert(m_hWnd);

	return m_hWnd != NULL;
}

LRESULT CALLBACK ScintillaEdit::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ScintillaEdit *pMe = (ScintillaEdit *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);	

	switch (msg) {
	case WM_CONTEXTMENU: //�����Ҽ��˵�
		::TrackPopupMenu(GetContextMenu()
			,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON
			,LOWORD(lParam),HIWORD(lParam)
			,0,pMe->m_hWnd,NULL);
		return 0;
	case WM_COMMAND: //�����Ҽ��˵���Ϣ
		if (lParam == 0 && pMe->OnContentMenuClick(wParam)) {
			return 0;
		}
		break;
	case WM_SYSKEYDOWN:
		//29λ	The context code. The value is 1 if the ALT key is down; otherwise, it is 0.
		if (lParam&0x20000000) {//����Scintilla��alt���Ĵ���
			return 1;
		}
		break;
	case WM_KEYDOWN:
		if (pMe->OnKeyDown(wParam,lParam)) {
			return 0;
		}
		break;
	case WM_CHAR://����Scintilla��ctrl��ϼ��Ĵ���
		if (::GetKeyState(VK_CONTROL) & 0Xf0) {
			return 0;
		}
		break;
	default:
		break;
	}

	//����tab����Ĭ�ϵ���Ϣ������
	return ::CallWindowProc(pMe->m_pDefaultWndProc, hwnd, msg, wParam, lParam);
}

//����true-��ʾ�������Դ����˸���Ϣ��������Ҫ���������߼�
BOOL ScintillaEdit::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	if (::GetKeyState(VK_CONTROL) & 0Xf0) { //ctrl��������
		if ('f' == wParam || 'F' == wParam || 'h' == wParam || 'H' == wParam) { //����&�滻
			DlgFind::GetInstance().ShowInModeless(GetSelectedText());
		} else if ('s' == wParam || 'S' == wParam) {//����
			SaveFile();
		} else if ((::GetKeyState(VK_SHIFT) & 0Xf0) && ('a' == wParam || 'A' == wParam)) { //�ַ���д
			::PostMessage(m_hWnd,WM_COMMAND,IDM_EDITOR_CASEUPPER,0);
		} else if ((::GetKeyState(VK_SHIFT) & 0Xf0) && ('z' == wParam || 'Z' == wParam)) { //�ַ�Сд
			::PostMessage(m_hWnd,WM_COMMAND,IDM_EDITOR_CASELOWER,0);
		} else if ('a' == wParam || 'A' == wParam) {//ȫѡ
			::PostMessage(m_hWnd,WM_COMMAND,IDM_EDITOR_SELALL,0);
		} else if ('c' == wParam || 'C' == wParam) {//����
			::PostMessage(m_hWnd,WM_COMMAND,IDM_EDITOR_COPY,0);
		} else if ('x' == wParam || 'X' == wParam) {
			::PostMessage(m_hWnd,WM_COMMAND,IDM_EDITOR_CUT,0);
		} else if ('v' == wParam || 'V' == wParam) {
			::PostMessage(m_hWnd,WM_COMMAND,IDM_EDITOR_PASTE,0);
		} else if ('z' == wParam || 'Z' == wParam) {//����
			::PostMessage(m_hWnd,WM_COMMAND,IDM_EDITOR_UNDO,0);
		} else if ('y' == wParam || 'Y' == wParam) {//�ָ�
			::PostMessage(m_hWnd,WM_COMMAND,IDM_EDITOR_REDO,0);
		}

		//������������ctrl��ϼ�
		return TRUE;
	} else if (VK_F3 == wParam || VK_F4 == wParam) {//��������
		std::string search = GetSeledText_UTF8();
		if (search.length() > 0) {
			FindByUTF8(search.c_str(),true,true,VK_F3==wParam);
		}
		return TRUE;
	}

	return FALSE;
}

BOOL ScintillaEdit::OnContentMenuClick(WPARAM wParam)
{
	std::string symbol;
	switch (wParam) {
	case IDM_EDITOR_UNDO:
		ExecuteScintilla(SCI_UNDO);
		return TRUE;
	case IDM_EDITOR_REDO:
		ExecuteScintilla(SCI_REDO);
		return TRUE;
	case IDM_EDITOR_SELALL:
		ExecuteScintilla(SCI_SELECTALL);
		return TRUE;
	case IDM_EDITOR_COPY:
		ExecuteScintilla(SCI_COPY);
		return TRUE;
	case IDM_EDITOR_CUT:
		ExecuteScintilla(SCI_CUT);
		return TRUE;
	case IDM_EDITOR_PASTE:
		ExecuteScintilla(SCI_PASTE);
		return TRUE;
	case IDM_EDITOR_DELETE:
		ExecuteScintilla(SCI_CLEAR);
		return TRUE;
	case IDM_EDITOR_CLEARNULLROW:
		ClearNullLine();
		return TRUE;
	case IDM_EDITOR_CLEARROWNUM:
		ClearLineNumber();
		return TRUE;
	case IDM_EDITOR_CASEUPPER:
		ExecuteScintilla(SCI_UPPERCASE);
		return TRUE;
	case IDM_EDITOR_CASELOWER:
		ExecuteScintilla(SCI_LOWERCASE);
		return TRUE;
	default:
		break;
	}
	return FALSE;
}

void ScintillaEdit::ReplaceSel(const std::wstring& text)
{
	std::string utf8_text = StringUtils::UnicodeToUTF8(text.c_str());
	if (utf8_text.length() > 0) {
		ExecuteScintilla(SCI_REPLACESEL, 0, LPARAM(utf8_text.c_str()));
	}
}

BOOL ScintillaEdit::SetFont(HFONT hFont)
{//���ṩ����������,����Scintilla�ؼ�ò�Ʋ�����WM_SETFONT��Ϣ
	assert(false);
	return TRUE;
}

ScintillaEdit::CHARSET ScintillaEdit::SetCharset(const int& charset)
{
	switch (charset) {
	case CHARSET_UTF8:
		m_charset = CHARSET_UTF8;
		break;
	case CHARSET_UTF16LE:
		m_charset = CHARSET_UTF16LE;
		break;
	case CHARSET_UTF16BE:
		m_charset = CHARSET_UTF16BE;
		break;
	default:
		m_charset = CHARSET_ANSI;
		break;
	}
	return m_charset;
}

std::wstring ScintillaEdit::GetFileName() const {
	if (m_strFilePath.length() < 1) {
		return TEXT("");
	}
	std::wstring name = m_strFilePath;
	StringUtils::replace(name,'/','\\');
	size_t pos = name.find_last_of('\\');
	if ((0 < pos) && (pos+1 < name.length())) {
		name = name.substr(pos+1);
	}
	return name;
}

std::wstring ScintillaEdit::GetFileExt() const {
	const std::wstring name = GetFileName();
	if (name.length() < 1) {
		return TEXT("");
	}
	size_t pos = name.find_last_of('.');
	if ((0 <= pos) && (pos+1 < name.length())) {
		return name.substr(pos+1);
	} else {
		return TEXT("");
	}
}

DWORD ScintillaEdit::OpenFile(LPCTSTR file,int charset/*=-1*/) {
	DWORD err_code(0);
	char *pContent = NULL;
	DWORD file_size(0);

	HANDLE hFile = ::CreateFile(file,GENERIC_READ,FILE_SHARE_READ| FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		err_code = GetLastError();
		assert(false);
		return err_code;
	}
	//��ȡ�ļ���С
	file_size = ::GetFileSize(hFile,NULL);
	if (file_size == INVALID_FILE_SIZE) {
		err_code = GetLastError();
		::CloseHandle(hFile);
		assert(false);
		return err_code;
	}
	//�����ڴ�
	pContent = new char[file_size+2];
	if (pContent == NULL) {
		::CloseHandle(hFile);
		assert(false);
		return -1;
	}
	//��ȡ�ļ�
	if (!::ReadFile(hFile,pContent,file_size,NULL,NULL)) {
		err_code = GetLastError();
		delete[] pContent;
		::CloseHandle(hFile);
		assert(false);
		return err_code;
	}
	//�ر��ļ�
	::CloseHandle(hFile);

	//�����ַ�������ֹ��
	pContent[file_size] = 0;
	pContent[file_size+1] = 0;

	//�ж��ļ����ַ����뼰�Ƿ�Я��BOM
	//0-�޷���ȷ�ж��ַ����룻2-UTF8; 3-UTF16LE; 4-UTF16BE,���С��0�����ʾЯ��BOMͷ
	int bomlen = 0;
	int charsetJuage = StringUtils::JudgeCharset(pContent,file_size);
	if (charsetJuage < 0) {
		m_containBOM = true;
		charsetJuage = 0-charsetJuage;
		if (charsetJuage == 2) bomlen=3;
		if (charsetJuage == 3 || charsetJuage == 4) bomlen=2;
	} else {
		m_containBOM = false;
	}

	if (charset < 0) {//�������-1���������ⲿָ���ļ����ַ�����
		switch (charsetJuage) {
		case 2:
			charset = CHARSET_UTF8;
			break;
		case 3:
			charset = CHARSET_UTF16LE;
			break;
		case 4:
			charset = CHARSET_UTF16BE;
			break;
		default:
			charset = CHARSET_ANSI;
			break;
		}
	}

	//���ļ��������õ��༭����,Scintilla����UTF8��Ϊ����
	if (CHARSET_UTF8 == charset) { //utf8
		m_charset = CHARSET_UTF8;
		ExecuteScintilla(SCI_SETTEXT,0,LPARAM(pContent+bomlen));
		delete pContent;
	} else if (CHARSET_UTF16LE == charset || CHARSET_UTF16BE == charset) {//utf16
		m_charset = (CHARSET_UTF16LE == charset ? CHARSET_UTF16LE : CHARSET_UTF16BE);
		std::string text_utf8 = StringUtils::UnicodeToUTF8((wchar_t*)(pContent+bomlen));
		delete[] pContent;
		ExecuteScintilla(SCI_SETTEXT,0,LPARAM(text_utf8.c_str()));
	} else { //ANSI
		m_charset = CHARSET_ANSI;
		std::string text_utf8 = StringUtils::ANSIToUTF8((char*)(pContent+bomlen));
		delete[] pContent;
		ExecuteScintilla(SCI_SETTEXT,0,LPARAM(text_utf8.c_str()));
	}

	ExecuteScintilla(SCI_EMPTYUNDOBUFFER);
	//This message tells Scintilla that the current state of the document is unmodified
	ExecuteScintilla(SCI_SETSAVEPOINT);

	m_strFilePath = file;
	m_file_charset = m_charset;
	m_langType = Lexer::GetLangByExt(GetFileExt().c_str());

	//�����﷨��ɫ
	Lexer::SetLanguage(this,m_langType);
	
	return err_code;
}

DWORD ScintillaEdit::SaveFile(BOOL saveAs/*=FALSE*/)
{
	//���ΪNULL,�򽫵����ļ�����Ի�����ѡ���ļ�����·��
	if (saveAs || m_strFilePath.length() < 1) {
		OPENFILENAME ofn;
		memset(&ofn,0,sizeof(ofn));
		ofn.lStructSize     = sizeof( ofn );
		ofn.hwndOwner       = m_hWnd;
		ofn.lpstrFile       = new TCHAR[1000];
		ofn.lpstrFile[0]    = 0;
		ofn.nMaxFile        = 1000;
		ofn.lpstrFilter     = TEXT("txt file(*.txt)\0*.txt\0sql file(*.sql)\0*.sql\0all files(*.*)\0*.*\0");
		ofn.nFilterIndex    = 0;
		ofn.lpstrFileTitle  = NULL;
		ofn.nMaxFileTitle   = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.lpstrDefExt     = TEXT("");
		ofn.Flags           = OFN_EXPLORER | OFN_ENABLESIZING | OFN_FORCESHOWHIDDEN;
		ofn.lpstrTitle      = TEXT("�����ļ�");
		if (::GetSaveFileName(&ofn)) {
			m_strFilePath = ofn.lpstrFile;
			delete []ofn.lpstrFile;
		} else {
			return 0;
		}
	} else if (!IsModified() && m_file_charset == m_charset) {//BOM��Ϣ��δ�ȶԣ�����BOMʱ�����ܻ᲻��Ч
		return 0;
	}

	DWORD err_code(0);	
	const int utf8_length = (int)ExecuteScintilla(SCI_GETLENGTH);

	//��ȡbom��Ϣ
	unsigned char bom_data[5];
	int bom_len(0);
	if (m_containBOM) {
		if (CHARSET_UTF8 == m_charset) {
			bom_len = StringUtils::GetBOMBytes(TEXT("utf8"),bom_data);
		} else if (CHARSET_UTF16LE == m_charset) {
			bom_len = StringUtils::GetBOMBytes(TEXT("utf16le"),bom_data);
		} else if (CHARSET_UTF16BE == m_charset) {
			bom_len = StringUtils::GetBOMBytes(TEXT("utf16be"),bom_data);
		} else {
			bom_len = 0;
		}
	}

	//�����ļ�
	DWORD numberOfBytesWritten(0);
	HANDLE hFile = ::CreateFile(m_strFilePath.c_str(),GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		err_code = GetLastError();
		assert(false);
		return err_code;
	}	
	
	//д��bom��Ϣ
	if (bom_len > 0) {
		::WriteFile(hFile,bom_data,bom_len,&numberOfBytesWritten,NULL);
	}

	//��ȡ�༭���е�����,��д���ļ�����	
	if (utf8_length > 0) {
		char *pContent = new char[utf8_length+1];
		ExecuteScintilla(SCI_GETTEXT,utf8_length+1,LPARAM(pContent));
		if (CHARSET_ANSI == m_charset) {
			std::string text = StringUtils::UTF8ToANSI(pContent);
			delete[] pContent;
			::WriteFile(hFile,text.c_str(),static_cast<DWORD>(text.length()),&numberOfBytesWritten,NULL);
		} else if (CHARSET_UTF8 == m_charset) {
			//��Scintilla��ȡ�����ı���utf8�����Բ���Ҫ����
			::WriteFile(hFile,pContent,utf8_length,&numberOfBytesWritten,NULL);
			delete []pContent;
		} else if (CHARSET_UTF16LE == m_charset || CHARSET_UTF16BE == m_charset) {
			std::wstring text_u = StringUtils::UTF8ToUnicode(pContent);
			delete[] pContent;
			if (CHARSET_UTF16BE == m_charset) {
				char lo,hi;
				const wchar_t *pU16BE = text_u.data();
				for (int i=0; i<text_u.length(); i++) {
					lo = LOBYTE(pContent[i]);
					hi = HIBYTE(pContent[i]);
					pContent[i] = MAKEWORD(hi,lo);
				}
			}
			::WriteFile(hFile,text_u.c_str(),static_cast<DWORD>(text_u.length()*2),&numberOfBytesWritten,NULL);
		}
	}
	
	::CloseHandle(hFile);

	m_file_charset = m_charset;

	//This message tells Scintilla that the current state of the document is unmodified
	ExecuteScintilla(SCI_SETSAVEPOINT);

	//����Ӧ�õ�������ʾ״̬
	EditorXFrame::GetInstance().UpdateMainMenuStatus(this);
	EditorXFrame::GetInstance().UpdateAppTitle(this);
	EditorXFrame::GetInstance().UpdateTabTitle(this);

	//��¼�ļ�����ʷ
	FileRecord conf;
	conf.AddRecord(m_strFilePath.c_str());
	return 0;
}

void ScintillaEdit::SetText(LPCTSTR text,bool clearModified/*=FALSE*/)
{
	std::string text_utf8;
#ifdef _UNICODE
	text_utf8 = StringUtils::UnicodeToUTF8(text);
#else
	text_utf8 = StringUtils::ANSIToUTF8(text);
#endif
	ExecuteScintilla(SCI_SETTEXT,0,LPARAM(text_utf8.c_str()));

	if (clearModified) {//����޸ı��
		ExecuteScintilla(SCI_EMPTYUNDOBUFFER);
		//This message tells Scintilla that the current state of the document is unmodified
		ExecuteScintilla(SCI_SETSAVEPOINT);
	}
}

void ScintillaEdit::ClearNullLine()
{
	//��Scintilla�л�ȡ����utf8������ַ���
	StringBuilder<std::string> strb;

	char *pLine = nullptr;
	size_t line_length(0);
	bool has_null_line(false);
	bool is_null_line(false);
	const size_t line_count = ExecuteScintilla(SCI_GETLINECOUNT);
	for (size_t i=0; i<line_count; i++) {
		line_length = ExecuteScintilla(SCI_LINELENGTH,i);
		if (line_length > 0) {
			pLine = new char[line_length+1];
			ExecuteScintilla(SCI_GETLINE,i,LPARAM(pLine));
			pLine[line_length] = 0;

			//�ж��Ƿ�Ϊ����
			is_null_line = true;
			for (int j=0; j<line_length; j++) {
				if (13 == pLine[j]) {//�������з�����ֹ�ж�
					break;
				}
				if (StringUtils::is_whitespace(pLine[j])) {
					continue;
				}
				//0xC2A0��utf8�Ŀո����
				if (j+1<line_length && 0xC2 == pLine[j] && 0xA0 == pLine[j+1]) {
					j++;
					continue;
				}
				is_null_line = false;
				break;
			}

			if (!is_null_line) {
				strb.append(pLine);
				delete []pLine;
			} else {
				has_null_line = true;
			}
		}
	}

	if (has_null_line) {
		ExecuteScintilla(SCI_SETTEXT,0,LPARAM(strb.toString().c_str()));
	}
}

void ScintillaEdit::ClearLineNumber()
{
	//��Scintilla�л�ȡ����utf8������ַ���
	StringBuilder<std::string> strb;

	const size_t sel_index_start = ExecuteScintilla(SCI_GETSELECTIONSTART);
	const size_t sel_index_end = ExecuteScintilla(SCI_GETSELECTIONEND);
	const size_t sel_line_start = ExecuteScintilla(SCI_LINEFROMPOSITION,sel_index_start);
	const size_t sel_line_end = ExecuteScintilla(SCI_LINEFROMPOSITION,sel_index_end);
	const size_t line_count = ExecuteScintilla(SCI_GETLINECOUNT);
	char *pLine = nullptr;
	size_t line_length(0);

	//���������Ƿ�Я���кţ����Я����ɾ������ɾ��������ı������strb��
	bool exists = false;
	for (size_t i=sel_line_start; i<=sel_line_end; i++) {
		line_length = ExecuteScintilla(SCI_LINELENGTH,i);
		if (line_length < 1) {
			continue;
		}
		pLine = new char[line_length+1];
		ExecuteScintilla(SCI_GETLINE,i,LPARAM(pLine));
		pLine[line_length] = 0;

		size_t index(0);
		bool match(false);
		for (; index<line_length; index++) {
			unsigned char c = pLine[index];
			if (13 == c) {//�������з�����ֹ�ж�
				break;
			}
			//0xC2A0��utf8�Ŀո����
			if (StringUtils::is_whitespace(c) || (index+1<line_length && 0xC2 == c && 0xA0 == pLine[index+1])) {
				if (!match) {
					if (!StringUtils::is_whitespace(c)) {
						//���������ͨ�Ŀհ׷�����˵����utf8�Ŀհ׷�������index��Ҫ��1��1λ
						index++;
					}
					continue;
				} else {
					break;
				}
			} else if (c < '0' || '9' < c) {
				break;
			} else {
				match = true;
			}
		}
		if (index<line_length) {
			exists = true;
			strb.append(pLine+index);
		} else {
			strb.append(pLine);
		}

		delete[] pLine;
	}

	//�滻ɾ���кź�����ı�
	if (exists) {
		const size_t start = ExecuteScintilla(SCI_POSITIONFROMLINE,sel_line_start);
		size_t end =  sel_line_end+1 == line_count ? ExecuteScintilla(SCI_GETTEXTLENGTH) : ExecuteScintilla(SCI_LINEFROMPOSITION,sel_index_end+1);
		ExecuteScintilla(SCI_SETSELECTIONSTART,start);
		ExecuteScintilla(SCI_SETSELECTIONEND,end);
		ExecuteScintilla(SCI_REPLACESEL,0,LPARAM(strb.toString().c_str()));
		//ѡ���滻����ı�����ʾ
		ExecuteScintilla(SCI_SETSELECTIONSTART,start);
		ExecuteScintilla(SCI_SETSELECTIONEND,start+strb.length());
	}
}

int ScintillaEdit::Find(const StringUtils::T_CHAR *pText,bool matchcase/*=true*/,bool wholeword/*=true*/,bool next/*=true*/)
{
#ifdef _UNICODE
	std::string search_text = StringUtils::UnicodeToUTF8(pText);
#else
	std::string search_text = StringUtils::ANSIToUTF8(pText);
#endif
	return FindByUTF8(search_text.c_str(),matchcase,wholeword,next);
}

//@param type 0-�滻���У�1-������һ�����滻��2-������һ�����滻
int ScintillaEdit::Replace(const StringUtils::T_CHAR *pText,const StringUtils::T_CHAR *pReplaceText,bool matchcase,bool wholeword,int type)
{
#ifdef _UNICODE
	std::string search_text = StringUtils::UnicodeToUTF8(pText);
	std::string replace_text = StringUtils::UnicodeToUTF8(pReplaceText);
#else
	std::string search_text = StringUtils::ANSIToUTF8(pText);
	std::string replace_text = StringUtils::ANSIToUTF8(pReplaceText);
#endif
	if (search_text.length() < 1) {
		return 0;
	}
	if (type != 0 && type != 1 && type != 2) {
		assert(false);
		return 0;
	}

	if (1 == type || type == 2) {
		int result(0);
		if (search_text.compare(GetSeledText_UTF8()) == 0) {
			ExecuteScintilla(SCI_REPLACESEL,0,LPARAM(replace_text.c_str()));
			result = 1;
		} else {
			result = 0;
		}
		//������һ��ƥ����
		FindByUTF8(search_text.c_str(),matchcase,wholeword,(1 == type));
		return result;
	} else { //�滻����		
		const std::string content = GetText_UTF8();
		StringBuilder<std::string> strb_dest;
		size_t replace_count(0);
		size_t start(0);
		int index(0);
		int flag = 0;
		if (matchcase) flag |= SCFIND_MATCHCASE;
		if (wholeword) flag |= SCFIND_WHOLEWORD;
		ExecuteScintilla(SCI_SETSEARCHFLAGS,flag);
		while (true) {
			ExecuteScintilla(SCI_SETTARGETSTART,start);
			ExecuteScintilla(SCI_SETTARGETEND,content.length());
			index = (int)ExecuteScintilla(SCI_SEARCHINTARGET,search_text.length(),LPARAM(search_text.c_str()));
			if (index > -1) {
				if (index > start) {
					strb_dest.append(content.substr(start,index-start));
				}
				if (replace_text.length() > 0) {
					strb_dest.append(replace_text);
				}
				replace_count++;
				start = index+search_text.length();
				ExecuteScintilla(SCI_SETTARGETSTART,start);
			} else {
				if (replace_count < 1) {
					return 0;
				} else {
					strb_dest.append(content.substr(start));
					break;
				}
			}
		}
		if (replace_count > 0) {
			ExecuteScintilla(SCI_SETTEXT,0,LPARAM(strb_dest.toString().c_str()));
		}
		return (int)replace_count;
	}

	assert(false);
	return 0;
}

int ScintillaEdit::FindByUTF8(const char* pUTF8Text,bool matchcase/*=true*/,bool wholeword/*=true*/,bool next/*=true*/)
{
	const size_t search_text_length = strlen(pUTF8Text);
	if (search_text_length < 1) {
		return -1;
	}

	const size_t sel_index_start = ExecuteScintilla(SCI_GETSELECTIONSTART);
	const size_t sel_index_end = ExecuteScintilla(SCI_GETSELECTIONEND);
	const size_t total_char_count = ExecuteScintilla(SCI_GETTEXTLENGTH);

	int result(-1);
	int flag = 0;
	if (matchcase) flag |= SCFIND_MATCHCASE;
	if (wholeword) flag |= SCFIND_WHOLEWORD;
	ExecuteScintilla(SCI_SETSEARCHFLAGS,flag);

	if (next) { //��������
		ExecuteScintilla(SCI_SETTARGETSTART,sel_index_end);
		ExecuteScintilla(SCI_SETTARGETEND,total_char_count);
		result = (int)ExecuteScintilla(SCI_SEARCHINTARGET,search_text_length,LPARAM(pUTF8Text));
		if (result < 0 && search_text_length <= sel_index_start) { //δ����������ѭ������
			ExecuteScintilla(SCI_SETTARGETSTART,0);
			ExecuteScintilla(SCI_SETTARGETEND,sel_index_start);
			result = (int)ExecuteScintilla(SCI_SEARCHINTARGET,search_text_length,LPARAM(pUTF8Text));
		}
	} else {//��ǰ����
		ExecuteScintilla(SCI_SETTARGETSTART,sel_index_start);
		ExecuteScintilla(SCI_SETTARGETEND,0);
		result = (int)ExecuteScintilla(SCI_SEARCHINTARGET,search_text_length,LPARAM(pUTF8Text));
		if (result < 0) { //δ����������ѭ������
			ExecuteScintilla(SCI_SETTARGETSTART,total_char_count);
			ExecuteScintilla(SCI_SETTARGETEND,sel_index_end);
			result = (int)ExecuteScintilla(SCI_SEARCHINTARGET,search_text_length,LPARAM(pUTF8Text));
		}
	}

	if (result > -1) {//ѡ�����������ı�	
		ExecuteScintilla(SCI_SETSELECTIONSTART,result);
		ExecuteScintilla(SCI_SETSELECTIONEND, result + search_text_length);

		//��������ǰ������λ��
		ExecuteScintilla(SCI_SCROLLCARET);
	}
	return result;
}

std::string ScintillaEdit::GetText_UTF8()
{
	std::string str;
	int size = (int)ExecuteScintilla(SCI_GETLENGTH);
	if (size > 0) {
		str.resize(size);
		ExecuteScintilla(SCI_GETTEXT,size+1,LPARAM(str.data()));
	}
	return str;
}

std::string ScintillaEdit::GetSeledText_UTF8()
{
	std::string str;
	int size = (int)ExecuteScintilla(SCI_GETSELTEXT);
	if (size > 1) {//size������null��ֹ��
		str.resize(size-1);
		ExecuteScintilla(SCI_GETSELTEXT,0,LPARAM(str.data()));
	}	
	return str;
}

StringUtils::T_STRING ScintillaEdit::GetSelectedText()
{
	std::string str = GetSeledText_UTF8();
	if (str.length() < 1) {
		return TEXT("");
	} else {
#ifdef _UNICODE
		return StringUtils::UTF8ToUnicode(str.c_str());
#else
		return StringUtils::UTF8ToANSI(str.c_str());
#endif
	}
}

void ScintillaEdit::WordStat(std::vector<intptr_t> &result)
{
	std::string str_utf8 = GetSeledText_UTF8();
	if (str_utf8.length() < 1) { //���û��ѡ����Ĭ��ͳ�������ļ�
		str_utf8 = GetText_UTF8();
	}

	int char_count(0);
	if (str_utf8.length() > 0) {
		char_count = MultiByteToWideChar(CP_UTF8, 0, str_utf8.c_str(), -1, 0, 0)-1; //-1����Ϊ������null��ֹ��
	}

	result.clear();//0-�ַ����룻 1-�ַ����� 2-�ֽ���
	result.push_back(GetCharset());
	result.push_back(char_count);
	if (GetCharset() == ScintillaEdit::CHARSET_ANSI) {
		if (str_utf8.length() > 0) {
			std::string str = StringUtils::UTF8ToANSI(str_utf8.c_str());
			result.push_back(str.length());
		}
		else {
			result.push_back(0);
		}
	}
	else if (GetCharset() == ScintillaEdit::CHARSET_UTF8) {
		result.push_back(str_utf8.length());
	}
	else if (GetCharset() == ScintillaEdit::CHARSET_UTF16LE || GetCharset() == ScintillaEdit::CHARSET_UTF16BE) {
		result.push_back(char_count * 2);
	}
}

BOOL ScintillaEdit::IsModified()
{
	return !!ExecuteScintilla(SCI_GETMODIFY);
}

int ScintillaEdit::SetLanguage(const int& lang_type) {
	if (lang_type == m_langType) {
		return m_langType;
	}

	if (Lexer::LANG_NONE <= lang_type && lang_type < Lexer::_LANG_END) {
		m_langType = Lexer::SetLanguage(this,lang_type);
	} else {
		assert(false);
		m_langType = Lexer::SetLanguage(this,Lexer::LANG_NONE);
	}
	return m_langType;
}