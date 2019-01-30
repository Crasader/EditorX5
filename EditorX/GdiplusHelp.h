#pragma once
#include<string>

class GdiplusHelp
{
public:
	//�����Gdiplus::GdiplusShutdown(m_gdiplusToken);��delete Gdiplus����; ����������
#define DELETE_GDIPLUS_OBJ(pObj) if (GdiplusHelp::IsGdiplusValid()) delete (pObj);

	/* �ɽ���ж�Gdiplus�����Ƿ���Ч
	* ���� �����Gdiplus::GdiplusShutdown(m_gdiplusToken);��delete Gdiplus����; ����������
	* ������Ҫ�Ƚ�˺����ж�
	*/
	static inline bool IsGdiplusValid() { return m_gdiplusToken != 0; }
	//��ʼ��Gdiplus����,���ܱ�����һ�Σ���
	static void LoadGdiplus() { Gdiplus::GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL); }
	//��ʼ��Gdiplus����,���ܱ�����һ�Σ���
	static void UnloadGdiplus() { Gdiplus::GdiplusShutdown(m_gdiplusToken); m_gdiplusToken = 0; }

	//����Բ�Ǿ���
	static bool GetRoundRectPath(const INT& x, const INT& y, const INT& width, const INT& height, const INT& radius
		, Gdiplus::GraphicsPath& path);

	//���Բ�Ǿ��εı���
	static void FillRoundRect(Gdiplus::Graphics& graphic, const Gdiplus::Brush *pBrushBK
		, const INT& x, const INT& y, const INT& width, const INT& height, const INT& radius);
	static void FillRoundRect(Gdiplus::Graphics& graphic, const Gdiplus::Brush *pBrushBK
		, const Gdiplus::RectF& rectf, const INT& radius);

	//����Բ�Ǿ���
	static void DrawRoundRect(Gdiplus::Graphics& graphic, const Gdiplus::Pen *pPen
		, const INT& x, const INT& y, const INT& width, const INT& height, const INT& radius
	);
	static void DrawRoundRect(Gdiplus::Graphics& graphic, const Gdiplus::Pen *pPen
		, const Gdiplus::RectF& rectf, const INT& radius
	);

private:
	static ULONG_PTR m_gdiplusToken;
	static Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;
};

