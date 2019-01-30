#pragma once

/**
 * �ַ��������� StringUtils
 * �ַ���ƴ���� StringBuilder
 * 20180105 lcc ����
 */
#include<string>
#include<vector>
#include<list>

/**
 * �ַ���������
 */
class StringUtils
{
public:
#ifdef _UNICODE
	typedef wchar_t T_CHAR;
	typedef std::wstring T_STRING;
#else
	typedef char T_CHAR;
	typedef std::string T_STRING;
#endif

public:
	/**
	 * ��һ���ַ������Ϊ������ַ�����Ȼ�󽫽����Ϊ�ַ����������
	 * param : str[in] - ���ָ���ַ���
			   seprator  [in] - �ָ���
			   arr_str   [out]- �ָ������ַ�������
	 * return : ����arrString��Ԫ�ظ���������Ϊ1
	 */
	static int split(const T_STRING str,const T_STRING separator,std::vector<T_STRING>& arr_str);

	/**
	 * �ַ����е������滻
	 * @param strSrc[in][out] ԭ�ַ���,����ִ��֮������滻������ַ���
	 * @param oldC[in] �����滻�����ַ���
	 * @param newC[in] ���滻���µ����ַ���
	 * @return �����滻������
	 */
	static int replace(T_STRING& strSrc,const T_STRING& oldC,const T_STRING& newC);
	static int replace(T_STRING& strSrc,const T_CHAR& oldC,const T_CHAR& newC);

	/**
	 * ��ʽ���ַ�����ת���ַ��Ķ�����printf��ͬ
	 */
	static T_STRING format(const T_CHAR* format,...);

	/*
	 * @param sub ���Ϊ���ַ��������ʾɾ���հ��ַ����ո�TAB�ȣ�
	 */
	static T_STRING rtrim(const T_STRING& str,const T_STRING& sub);
	static T_STRING rtrim(const T_STRING& str,const T_CHAR& sub=0);
	static T_STRING ltrim(const T_STRING& str,const T_STRING& sub);
	static T_STRING ltrim(const T_STRING& str,const T_CHAR& sub=0);

	static T_STRING to_lowercase(T_STRING& str);
	static T_STRING to_uppercase(T_STRING& str);

	//�ж��Ƿ�Ϊ�հ׷�
	static inline bool is_whitespace(const T_CHAR& c) {
		//160-&nbsp; 0x3000-unicode�Ŀո����; 0xC2A0-utf8�Ŀո����
		return (' ' == c || '\t' == c || 160 == c || 0x3000 == c);
	}

	/**
	 * ����ַ�����ӦBOM�ֽ�
	 * @param charset[in]     �ַ�������
	 * @param pBomBytes[out]  ����������ַ�����Ӧ��bom,��������Ĵ�С���ڻ����4
	 * @return bom���ֽ��������Ϊ0�����ʾδ�ҵ����ַ�����Ӧ��bom
	 */
	static int GetBOMBytes(LPCTSTR charset,unsigned char* pBomBytes);

	/**
	 * �ж��ַ����ı���
	 * @param str ���жϵ��ַ���
	 * @param size ���ַ����Ĵ�С�����ֽ�Ϊ��λ����������ֹ��
	 * @return 0-�޷���ȷ�ж��ַ����룻
	 *         2-UTF8; 3-UTF16LE; 4-UTF16BE��
	 *         ���С��0�����ʾЯ��BOMͷ��abs()֮��ȡ�õ��������ĺ���ο������ע��
	 */
	static int JudgeCharset(const char* str,const DWORD& size);

	//�ַ�����ת��
	static std::string UnicodeToUTF8(const wchar_t* pText);
	static std::string UnicodeToANSI(const wchar_t* pText);
	static std::wstring UTF8ToUnicode(const char* pText);
	static std::string UTF8ToANSI(const char* pText);
	static std::wstring ANSIToUnicode(const char* pText);
	static std::string ANSIToUTF8(const char* pText);
};

/**
 * �ַ���ƴ��,������java��StringBuilder
 * T ֻ��Ϊstd::string �� std::wstring
 * !!!ģ���಻�ܰѺ�������ŵ�cpp�У������ⲿʹ�øú���ʱ����������Ӵ���
 */
template<class T> class StringBuilder {
public:
	void reset(void) {
		m_lst_str.clear();
	}
	void append(const T& str) {
		m_lst_str.push_back(str);
	}
	size_t length() {
		const int count = (int)m_lst_str.size();
		size_t total_size = 0;
		std::list<T>::iterator iter = m_lst_str.begin();
		for (; iter!=m_lst_str.end(); iter++) {
			total_size += (*iter).length();
		}
		return total_size;
	}
	T toString() {
		T str;
		const size_t total_size = length();
		if (total_size < 1) {
			return str;
		}		
		std::list<T>::iterator iter;
	
		str.resize(total_size);
		if (sizeof(str[0]) == 2) {//std::wstring
			wchar_t *p_temp = (wchar_t*)(str.data());
			for (iter = m_lst_str.begin(); iter!=m_lst_str.end(); iter++) {
				std::memcpy(p_temp,(*iter).c_str(),(*iter).length()*2);
				p_temp += (*iter).length();
			}
		} else { //std::string
			char *p_temp = (char*)(str.data());
			for (iter = m_lst_str.begin(); iter!=m_lst_str.end(); iter++) {
				std::memcpy(p_temp,(*iter).c_str(),(*iter).length());
				p_temp += (*iter).length();
			}
		}

		return str;
	}

private:
	std::list<T> m_lst_str;
};

