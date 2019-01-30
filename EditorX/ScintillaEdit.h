#pragma once
#include "BaseWnd.h"
#include <string>
#include <map>
#include <vector>
#include "StringUtils.h"
#include "scintilla/Scintilla.h"
#include "scintilla/SciLexer.h"

/**
 * ʹ�ø���֮ǰ��Ҫ�ڳ����ʼ��ʱ����::LoadLibrary(_T(".\\bin\\SciLexer.dll"))
 */
class ScintillaEdit : public BaseWnd
{
public:
	enum CHARSET{
		CHARSET_ANSI = 0
		, CHARSET_UTF8
		, CHARSET_UTF16LE
		, CHARSET_UTF16BE
	};

public:
	ScintillaEdit(void);
	virtual ~ScintillaEdit(void);

	virtual BOOL Create(HINSTANCE hInstance,HWND hParent,const RECT *p_rect=NULL);
	virtual BOOL SetFont(HFONT hFont);

	/**
	 * ���ļ�
	 * @param file �ļ��ľ���·��,���ΪNULL,�򽫵����ļ�����Ի�����ѡ���ļ�����·��
	 * @param charset ��ʾ�Ժ����ַ�����򿪣����Ϊ-1�����ʾ�����Զ��жϣ�����������ScintillaEdit::CHARSET
	 * @return ����ɹ��򷵻�0�����򷵻�GetLastError()��ֵ
	 */
	DWORD OpenFile(LPCTSTR file,int charset=-1);
	DWORD SaveFile(BOOL saveAs=FALSE);
	BOOL IsModified();

	virtual std::wstring GetToolTip() const {return m_strFilePath.length()<1 ? TEXT("") : m_strFilePath;}
	inline std::wstring GetFilePath() const { return m_strFilePath; }
	std::wstring GetFileName() const;
	std::wstring GetFileExt() const; //��ȡ�ļ��ĺ�׺��

	/**
	 * �����ı�����
	 * @param clearModified �Ƿ�����޸ı�ǣ��������Ϊtrue,���ʾ�ñ༭��������δ���޸Ĺ�
	 */
	void SetText(LPCTSTR text,bool clearModified=FALSE);
	//��ȡUTF8������ı�����
	std::string GetText_UTF8();
	std::string GetSeledText_UTF8();
	StringUtils::T_STRING GetSelectedText();
	void ReplaceSel(const std::wstring& text);

	inline void MakeUpperCase() { ExecuteScintilla(SCI_UPPERCASE); }
	inline void MakeLowerCase() { ExecuteScintilla(SCI_LOWERCASE); }

	//�������
	void ClearNullLine();
	//����кţ�����վ�Ͽ�������ʱ����������ÿ�еĿ�ͷЯ���кţ�
	void ClearLineNumber();

	//�Ե�ǰ���λ��Ϊ���(���ѡ�������֣�����ѡ��Ľ�����Ϊ���)����ʼѭ��������������������λ�ã����򷵻�-1
	int FindByUTF8(const char* pUTF8Text,bool matchcase=true,bool wholeword=true,bool next=true);
	int Find(const StringUtils::T_CHAR *pText,bool matchcase=true,bool wholeword=true,bool next=true);
	/**
	 * �������滻�ı�
	 * @param pText �����滻���ı�
	 * @param pReplaceText �滻���ı�
	 * @type 0-�滻���У�1-�滻��������һ����2-�滻��������һ��
	 * @return �����滻���ı�����
	 */
	int Replace(const StringUtils::T_CHAR *pText,const StringUtils::T_CHAR *pReplaceText,bool matchcase,bool wholeword,int type);

	/*
	 * ����ͳ��
	 * @param result[out] 0-�ַ�����(ScintillaEdit::CHARSET_...)�� 1-�ַ����� 2-�ֽ���
	 */
	void WordStat(std::vector<intptr_t> &result);

	CHARSET GetCharset() const {return m_charset;}
	CHARSET SetCharset(const int& charset);
	bool IsContainBOM() const {return m_containBOM;}
	void SetContainBOM(bool containBOM) {m_containBOM=containBOM;}

	//�����﷨��ɫ�����سɹ����õ��������ͣ�����޸��������ͣ�����Lexer::LANG_NONE
	int SetLanguage(const int& lang_type);
	int GetLanguage() const {return m_langType;}

	//�����Զ�����
	void SetWordWrap(bool worderap) {ExecuteScintilla(SCI_SETWRAPMODE,worderap ? SC_WRAP_WORD : SC_WRAP_NONE);}
	bool GetWordWrap() {return (ExecuteScintilla(SCI_GETWRAPMODE) == SC_WRAP_WORD);}

	inline sptr_t ExecuteScintilla(UINT msg, uptr_t wParam=0, sptr_t lParam=0) {
		return m_pScintillaFunc(m_pScintillaObj, msg, wParam, lParam);
	}

protected:
	//����ִ��scintilla�ķ���
	typedef sptr_t (SCINTILLA_FUNC) (void*, UINT, uptr_t, sptr_t);
	typedef void* SCINTILLA_PTR;
	SCINTILLA_FUNC *m_pScintillaFunc;
	SCINTILLA_PTR m_pScintillaObj;

	//�ļ������������
	std::wstring m_strFilePath;
	CHARSET m_charset;      //��ǰѡ����ַ�����
	CHARSET m_file_charset; //�ļ���ԭʼ�ַ�����
	bool m_containBOM; //�ļ��Ƿ�Я��BOM��Ϣ
	//����������ͣ�Ĭ��ֵLexer::LANG_NODE
	int m_langType;

	//ʵ������
	static int m_ref_count;

	/////////////////////////////////////////////////
	//�Ҽ��˵���ش���
	enum {
		IDM_EDITOR_UNDO = WM_USER+1
		, IDM_EDITOR_REDO
		, IDM_EDITOR_SELALL
		, IDM_EDITOR_COPY
		, IDM_EDITOR_CUT
		, IDM_EDITOR_PASTE
		, IDM_EDITOR_DELETE
		, IDM_EDITOR_CLEARNULLROW
		, IDM_EDITOR_CLEARROWNUM
		, IDM_EDITOR_CASEUPPER
		, IDM_EDITOR_CASELOWER
	};
	static std::vector<HMENU> m_arrHPopupMenu; //��һ��Ԫ��Ϊ�Ҽ��˵��ľ����������ο�GetContextMenu()
	static HMENU GetContextMenu();
	BOOL OnContentMenuClick(WPARAM wParam);

	//�ؼ�Ĭ�ϵ���Ϣ�������ĵ�ַ
	WNDPROC m_pDefaultWndProc;
	//�Զ������Ϣ������
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL OnKeyDown(WPARAM wParam, LPARAM lParam);
};

