#pragma once
#include<string>

class WinSDKUtils
{
public:
	static std::wstring GetModulePath();

	//@return ʹ����������DestroyIcon
	static HICON GetFileIcon(const std::wstring& filepath);

	//Ϊ������߷ֱ�Ļ���,���鲻Ҫ�ڹ��캯���е��øú��������ܻᵼ�´�����
	static intptr_t GetAdaptationSizeForDPI(const intptr_t& n);

	static inline void FillSolidRect(const HDC& hdc, const RECT& rect, const COLORREF& color) {
		::SetBkColor(hdc, color);
		::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
	}

	/*
	 * �ж����������Ƿ��ཻ
	 * @return 0-���ཻ��1-�ཻ��-1-���
	 */
	static inline intptr_t IsIntersectRect(const RECT& rc1, const RECT& rc2) {
		if (rc1.left == rc2.left && rc1.top == rc2.top && rc1.right == rc2.right && rc1.bottom == rc2.bottom) {
			return -1;
		}
		return (rc1.left < rc2.right && rc1.top < rc2.bottom && rc2.left < rc1.right && rc2.top < rc1.bottom);
	}
	static inline int RectWidth(const RECT& rect) { return ::abs(rect.right - rect.left); }
	static inline int RectHeight(const RECT& rect) { return ::abs(rect.bottom - rect.top); }
	static void ClientToScreen(const HWND& hwnd, RECT& rect);

protected:
	static std::wstring m_appFolderPath;

	//Ϊ������߷ֱ�Ļ���,ʹ��ʱ�����GetAdaptationSizeForDPI
	static size_t m_zoomScale; //��Ļ���ű���
};

