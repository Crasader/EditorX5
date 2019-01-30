#pragma once

/**
 * �����﷨��ɫ
 */
class ScintillaEdit;
class Lexer
{
public:
	enum {
		LANG_NONE = 0
		,LANG_C
		,LANG_JAVA
		,LANG_SQL
		,LANG_XML
		,LANG_JSP
		,LANG_HTML
		,LANG_JS
		,_LANG_END //Ϊ�˷��������������
	};

	//��ɫ����
	static const COLORREF COLOR_LINE_BK;     //�б���ɫ
	static const COLORREF COLOR_STRING;      //�ַ���
	static const COLORREF COLOR_CHAR;        //�ַ�
	static const COLORREF COLOR_COMMENTS;    //ע��
	static const COLORREF COLOR_PRECOMPILED; //Ԥ����
	static const COLORREF COLOR_KEYWORD;     //�ؼ���
	static const COLORREF COLOR_CRIMSON;     //���ɫ
	static const COLORREF COLOR_DARKGREEN;   //����ɫ
	static const COLORREF COLOR_ORANGE;      //�ٻ�ɫ
	static const COLORREF COLOR_LIGHTBROWN;  //ǳ��ɫ

public:

	//ͨ���ļ���׺���ж���������
	static int GetLangByExt(LPCTSTR ext);
	//���ñ༭�����﷨��ɫ�����سɹ����õ��������ͣ�����޸��������ͣ�����Lexer::LANG_NONE
	static int SetLanguage(ScintillaEdit* pEditor,const int& lang_type);

protected:
	///////////////////////////////////////////////////////////////
	//��ȡ������ԵĹؼ���
	static char* GetKeyword1_C();
	static char* GetKeyword2_C();

	static char* GetKeyword1_JAVA();
	static char* GetKeyword2_JAVA();

	static char* GetKeyword1_SQL();
	static char* GetKeyword1_JS();
};

