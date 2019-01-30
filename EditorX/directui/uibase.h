#pragma once
#include<string>
#include<vector>
#include "../StringUtils.h"

/* ��DirectUI��ʽ���ɵĽ�����������������UIBaseΪ����,����UIFrame�������н������
 * ����Gdiplu���ƽ���
 * ���ڳ�������ʱ�����lcc_direct_ui::UIBase::LoadGdiplus������Gdiplus����
 * ����ر�ʱ����lcc_direct_ui::UIBase::UnloadGdiplus�Թر�Gdiplus����
 * ��� 201802
 */
namespace lcc_direct_ui {
	//�����־λ
	enum :size_t {
		BTN_STATE_DEFAULT = 0
		, BTN_STATE_HOVER = 0x0001
		, BTN_STATE_CHECKED = 0x0002
	};

	/**
	 * UI����Ļ���
	 */
	class UIBase {
	public:
		UIBase();
		virtual ~UIBase();

		bool Create(const HWND& hWnd, const RECT* pRect=nullptr, const size_t& id = 0);

		inline HWND GetHWnd() const { return m_hWnd; }
		inline size_t GetId() const { return m_id; }
		inline UIBase* GetParent() const { return m_pParent; }
		inline void SetParent(UIBase* pParent) { m_pParent = pParent; }

		inline RECT GetRect() const { return {(int)m_x,(int)m_y,(int)(m_x+m_width),(int)(m_y+m_height)}; }
		inline Gdiplus::RectF GetRectF() const { return { (Gdiplus::REAL)m_x,(Gdiplus::REAL)m_y,(Gdiplus::REAL)m_width,(Gdiplus::REAL)m_height }; }
		inline intptr_t GetRectLeft() const { return m_x; }
		inline intptr_t GetRectTop() const { return m_y; }
		inline intptr_t GetRectRight() const { return m_x+m_width; }
		inline intptr_t GetRectBottom() const { return m_y + m_height; }
		inline size_t GetRectWidth() const { return m_width; }
		inline size_t GetRectHeight() const { return m_height; }
		inline size_t GetZOrder() const { return m_z; }
		inline void SetZOrder(const size_t& z) { m_z = z; }
		inline bool PtInArea(const POINT& pt) const { return m_x < pt.x&&pt.x < (m_x + m_width) && m_y < pt.y&&pt.y < (m_y+m_height); }

		virtual bool ModifyFont(const Gdiplus::FontStyle& style,const Gdiplus::REAL& emSize=0);
		SIZE GetTextSize(const std::wstring& text
			, const Gdiplus::Font* pFont = nullptr
			, const Gdiplus::StringFormat* pStringFormat = nullptr);
		static SIZE GetTextSize(Gdiplus::Graphics& graphic
			,const std::wstring& text
			, const Gdiplus::Font* pFont
			, const Gdiplus::StringFormat* pStringFormat=nullptr);

		//���û����Զ�������
		inline void SetUserData(intptr_t data) { m_userData = data; }
		inline intptr_t GetUserData() const { return m_userData; }

		//����32λUUID
		static StringUtils::T_STRING GenerateUUID32();

		static Gdiplus::Image *HICONToGdiplusImage(HICON hIcon);

		static inline intptr_t Min(const intptr_t& x1, const intptr_t& x2) { return x1 < x2 ? x1 : x2; }
		static inline intptr_t Max(const intptr_t& x1, const intptr_t& x2) { return x1 > x2 ? x1 : x2; }
		static bool PtInLine(const intptr_t& x, const intptr_t& y
			, const intptr_t& line_start_x, const intptr_t& line_start_y
			, const intptr_t& line_end_x, const intptr_t& line_end_y
		);

		/*
		 * ���������߶εĽ�������
		 * ֱ��1 [(x1,y1),(x2,y2)] ֱ��2[(x3,y3),(x4,y4)]
		 * @param lpIntersectPointX[out] �����X������ཻ�Ļ�,���ڷ���ֵΪ1������£����������Ч
		 * @param lpIntersectPointY[out] �����Y������ཻ�Ļ�,���ڷ���ֵΪ1������£����������Ч
		 * @return 0-�����߶β��ཻ��-1-�����߶��غϣ�1�����߶��ཻ
		 */
		static intptr_t GetIntersectPointByLines(const intptr_t& x1, const intptr_t& y1
			, const intptr_t& x2, const intptr_t& y2
			, const intptr_t& x3, const intptr_t& y3
			, const intptr_t& x4, const intptr_t& y4
			, intptr_t* lpIntersectPointX=nullptr
			, intptr_t* lpIntersectPointY=nullptr
		);
		static bool PtInRegion(const intptr_t& x, const intptr_t& y,const std::vector<POINT>& points);
		static Gdiplus::Color ConvertToGDIPlusColor(const COLORREF& color);

		static inline Gdiplus::RectF ConvertToGDIPlusRect(const RECT& rect) {
			return Gdiplus::RectF(Gdiplus::REAL(rect.left)
				, Gdiplus::REAL(rect.top)\
				, Gdiplus::REAL(::abs(rect.right - rect.left))
				, Gdiplus::REAL(::abs(rect.bottom - rect.top)));
		}
		static void DrawText(Gdiplus::Graphics& graphic
			, const Gdiplus::Brush* pBrushText
			, const Gdiplus::Font* pFont
			, const std::wstring& text
			, Gdiplus::RectF rectf
			, const Gdiplus::StringFormat *pStringFormat = nullptr);

		static Gdiplus::Image *LoadGdiplusImage(const std::wstring& imgName);
		static RECT FromGdiplusRectF(const Gdiplus::RectF& rectf);
		
		virtual void Offset(const intptr_t& x, const intptr_t& y) { m_x += x; m_y += y; }
		// width��height ���<0�����
		virtual void Resize(const intptr_t& width,const intptr_t& height, bool redraw = true);
		
		inline void Redraw() { ::InvalidateRect(m_hWnd, &GetRect(), FALSE); }
		//@return true-����������Ҫ������Ϣ��Ӧ����������
		virtual bool Draw(Gdiplus::Graphics& graphic, const PAINTSTRUCT& ps) { return false; }
		virtual bool OnMouseMove(const POINT& pt) { return m_show && PtInArea(pt); }
		virtual bool OnLButtonDown(const POINT& pt) { return m_show && PtInArea(pt); }
		virtual bool OnLButtonDblClk(const POINT& pt) { return m_show && PtInArea(pt); }
		virtual bool OnLButtonUp(const POINT& pt) { return m_show && PtInArea(pt); }
		virtual bool OnRButtonDown(const POINT& pt) { return m_show && PtInArea(pt); }

	protected:
		HWND m_hWnd;
		size_t m_id;
		std::wstring m_fontFamilyName;
		Gdiplus::Font *m_pFont;
		Gdiplus::SolidBrush *m_pBrushForText;
		Gdiplus::SolidBrush *m_pBrushBK; //������ˢ

		//��UI����ڵ�ƽ��λ��
		intptr_t m_x, m_y;
		size_t  m_width, m_height;
		//��UI�����Z���ϵ�λ�ã�Ĭ��Ϊ0.����ײ�
		size_t m_z;
		bool m_show; //����Ƿ���ʾ

		//��������û��Զ�������
		intptr_t m_userData;

		UIBase* m_pParent;

		//��ǰUI�����ͼ�񻺴�
		Gdiplus::Bitmap* m_pImageCatch;
		Gdiplus::Bitmap* GrabImage(); //��ȡ��ǰUI����ʾͼ��
		void CopyBitmapToClipBoard(HBITMAP hBitmap);
	};

	/////////////////////////////////////////////////
	//�ָ���
	class Separator : public UIBase {
	public:
		Separator() :UIBase() { m_pBrushBK->SetColor(Gdiplus::Color(255, 120, 120, 120)); }
		bool Create(const HWND& hWnd,const POINT& pt, const size_t& height);
		virtual bool Draw(Gdiplus::Graphics& graphic, const PAINTSTRUCT& ps);
	};
}