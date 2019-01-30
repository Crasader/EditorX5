#include "stdafx.h"
#include "uibase.h"
#include "../WinSDKUtils.h"
#include "../ColorHelp.h"
#include "../GdiplusHelp.h"

namespace lcc_direct_ui {
	//////////////////////////////////////////////////////////////
	//UIBase
	UIBase::UIBase()
		: m_hWnd(NULL)
		, m_fontFamilyName(TEXT("Arial")) //gdiplus��֧��"Courier New"����������
		, m_pFont(nullptr)
		, m_pBrushForText(nullptr)
		, m_pBrushBK(nullptr)
		, m_userData(0)
		, m_pImageCatch(nullptr)
		, m_pParent(nullptr)
		, m_show(true)
	{
		m_x = m_y = m_width = m_height = m_z = 0;
		m_pFont = new Gdiplus::Font(m_fontFamilyName.c_str(),8.6f, Gdiplus::FontStyle::FontStyleRegular);
		m_pBrushForText = new Gdiplus::SolidBrush(Gdiplus::Color(255, 0, 0, 0));
		m_pBrushBK = new Gdiplus::SolidBrush(Gdiplus::Color(255, 255, 255, 255));
	}

	UIBase::~UIBase()
	{
		if (m_pFont) {
			DELETE_GDIPLUS_OBJ(m_pFont);
			m_pFont = nullptr;
		}
		if (m_pImageCatch) {
			DELETE_GDIPLUS_OBJ(m_pImageCatch);
			m_pImageCatch = nullptr;
		}
		if (m_pBrushForText) {
			DELETE_GDIPLUS_OBJ(m_pBrushForText);
			m_pBrushForText = nullptr;
		}
		if (m_pBrushBK) {
			DELETE_GDIPLUS_OBJ(m_pBrushBK);
			m_pBrushBK = nullptr;
		}
	}

	bool UIBase::Create(const HWND& hWnd, const RECT* pRect/*=nullptr*/, const size_t& id/*=0*/)
	{
		m_hWnd = hWnd;
		m_id = id;
		if (pRect) {
			m_x = pRect->left;
			m_y = pRect->top;
			if (pRect->right > pRect->left) m_width = pRect->right - pRect->left;
			if (pRect->bottom > pRect->top) m_height = pRect->bottom - pRect->top;
		}
		return true;
	}

	bool UIBase::ModifyFont(const Gdiplus::FontStyle& style, const Gdiplus::REAL& emSize/*=0*/) {
		Gdiplus::REAL size = ((0 == emSize) ? m_pFont->GetSize() : emSize);
		if (style == m_pFont->GetStyle() && size == emSize) {
			return false;
		}
		delete m_pFont;
		m_pFont = new Gdiplus::Font(m_fontFamilyName.c_str(), size, style);
		return true;
	}

	void UIBase::Resize(const intptr_t& width, const intptr_t& height, bool redraw/*= true*/)
	{
		if (width > -1) {
			m_width = width;
		}
		if (height > -1) {
			m_height = height;
		}
		if (redraw && (width>-1 || height>-1)) {
			Redraw();
		}
	}

	StringUtils::T_STRING UIBase::GenerateUUID32()
	{
		StringUtils::T_STRING uuid;
		GUID guid;
		::CoCreateGuid(&guid);
		uuid = StringUtils::format(TEXT("%08X%04X%04x%02X%02X%02X%02X%02X%02X%02X%02X")
			, guid.Data1, guid.Data2, guid.Data3
			, guid.Data4[0], guid.Data4[1], guid.Data4[2]
			, guid.Data4[3], guid.Data4[4], guid.Data4[5]
			, guid.Data4[6], guid.Data4[7]
		);
		return uuid;
	}

	Gdiplus::Image *UIBase::HICONToGdiplusImage(HICON hIcon)
	{
		ICONINFO icInfo = { 0 };
		if (!::GetIconInfo(hIcon, &icInfo))
		{
			assert(false);
			return nullptr;
		}

		BITMAP bitmap;
		::GetObject(icInfo.hbmColor, sizeof(BITMAP), &bitmap);

		Gdiplus::Bitmap *pImage = nullptr;
		if (bitmap.bmBitsPixel <= 32) {
			pImage = Gdiplus::Bitmap::FromHICON(hIcon);
		}
		else {//֧��͸��ɫ,�ö��㷨�����⣬��������if��Ϊ<=,����ԶΪtrue
			Gdiplus::Bitmap* pWrapBitmap = Gdiplus::Bitmap::FromHBITMAP(icInfo.hbmColor, NULL);
			Gdiplus::BitmapData bitmapData;
			Gdiplus::Rect rcImage(0, 0, pWrapBitmap->GetWidth(), pWrapBitmap->GetHeight());
			pWrapBitmap->LockBits(&rcImage, Gdiplus::ImageLockModeRead, pWrapBitmap->GetPixelFormat(), &bitmapData);
			pImage = new Gdiplus::Bitmap(bitmapData.Width, bitmapData.Height
				, bitmapData.Stride, PixelFormat32bppARGB, (BYTE*)bitmapData.Scan0);
			delete pWrapBitmap;
		}

		return pImage;
	}

	//�ж�ĳ���Ƿ���ֱ����
	bool UIBase::PtInLine(const intptr_t& x, const intptr_t& y
		, const intptr_t& line_start_x, const intptr_t& line_start_y
		, const intptr_t& line_end_x, const intptr_t& line_end_y
	) {
		//���Ƿ����ߵĶ����غ�
		if ((x == line_start_x && y == line_start_y) || (x == line_end_x && y == line_end_y)) {
			return true;
		}

		//�жϸ�ֱ���Ƿ���һ����
		if (line_start_x == line_end_x && line_start_y == line_end_y) {
			return false;
		}

		//�жϸõ��Ƿ����Ը�ֱ��Ϊ�Խ��ߵľ����⣬����ǣ���˵������ֱ���ϣ��������Ч��
		if (x < Min(line_start_x,line_end_x) || Max(line_start_x, line_end_x) < x) return false;
		if (y < Min(line_start_y,line_end_y) || Max(line_start_y, line_end_y) < y) return false;

		//���ֱ���Ǻ��߻���ߣ���һ�������ϣ����������֮ǰ���ж�
		if (line_start_y == line_end_y || line_start_x == line_end_x) {
			return true;
		}

		//ֱ�ߵķ���ʽ��(y-y1)/(y2-y1)=(x-x1)/(x2-x1)
		return (x-line_start_x)*(line_end_y-line_start_y) == (y-line_start_y)*(line_end_x-line_start_x);
	}

	//@return 0-�����߶β��ཻ��-1-�����߶��غϣ�1�����߶��ཻ
	intptr_t UIBase::GetIntersectPointByLines(const intptr_t& x1, const intptr_t& y1
		, const intptr_t& x2, const intptr_t& y2
		, const intptr_t& x3, const intptr_t& y3
		, const intptr_t& x4, const intptr_t& y4
		, intptr_t* lpIntersectPointX/*=nullptr*/
		, intptr_t* lpIntersectPointY/*=nullptr*/
	) {
		//�����ų⣺�����߶�Ϊ�Խ�����ɵľ��Σ��������������û���ص��Ĳ��֣���ô�����߶β������ཻ
		if (Min(x1, x2) <= Max(x3, x4)
			&& Min(y3, y4) <= Max(y1, y2)
			&& Min(x3, x4) <= Max(x1, x2)
			&& Min(y1, y2) <= Max(y3, y4)
			) {
		}
		else {
			return 0;
		}

		/* ֱ�ߵķ���ʽ��(y-y1)/(y2-y1)=(x-x1)/(x2-x1)
		 * �ɸ÷���ʽ�ó�ֱ��1 �� ��x2-x1)(y-y1)=(y2-y1)(x-x1)
		 * �ɸ÷���ʽ�ó�ֱ��2 ��  (x4-x3)(y-y3)=(y4-y3)(x-x3)
		 * ���Խ��������Ϊ��
		 *     x=[(y2-y1)(x4-x3)x1-(y4-y3)(x2-x1)x3]/[(y2-y1)(x4-x3)-(y4-y3)(x2-x1)]
		 *	   y=[(x2-x1)(y4-y3)y1-(x4-x3)(y2-y1)y3]/[(x2-x1)(y4-y3)-(x4-x3)(y2-y1)]

		 x = [(x2-x1)(y3-y1)(x4-x3)+(y2-y1)(x4-x3)x1-(y4-y3)(x2-x1)x3]/[(y2-y1)(x4-x3)-(y4-y3)(x2-x1)]
		 y = [(y2-y1)(x3-x1)(y4-y3)+(x2-x1)(y4-y3)y1-(x4-x3)(y2-y1)y3]/[(x2-x1)(y4-y3)-(x4-x3)(y2-y1)]
		 */
		intptr_t x, y; //���������
		intptr_t a,b,c,d; //�ֱ��¼�������� x=a/b  y=c/d
		a = (x2 - x1)*(y3 - y1)*(x4 - x3) + (y2 - y1)*(x4 - x3)*x1 - (y4 - y3)*(x2 - x1)*x3;
		b = (y2 - y1)*(x4 - x3) - (y4 - y3)*(x2 - x1);
		c = (y2 - y1)*(x3 - x1)*(y4 - y3) + (x2 - x1)*(y4 - y3)*y1 - (x4 - x3)*(y2 - y1)*y3;
		d = (x2 - x1)*(y4 - y3) - (x4 - x3)*(y2 - y1);

		if (0 == b && 0 == d) {//����ֱ�����غϵ�
			return -1;
		}
		x = intptr_t(a / b);
		y = intptr_t(c / d);
		if (lpIntersectPointX) *lpIntersectPointX = x;
		if (lpIntersectPointY) *lpIntersectPointY = y;

		//������������е�һ���߶��ϣ���˵�������߶��ཻ�������������ཻ�������ӳ����Ͽ������ཻ�ģ�
		return PtInLine(x,y,x1,y1,x2,y2) ? 1 : 0;
	}

	bool UIBase::PtInRegion(const intptr_t& x, const intptr_t& y, const std::vector<POINT>& points)
	{
		if (points.size() < 2) {
			assert(false);
			return false;
		}

		//�����ų⣬����㲻�ڸ�����ĺ���������ķ�Χ�ڣ���˵���õ㲻�ڸ�������
		intptr_t minX(points[0].x), maxX(points[0].x), minY(points[0].y), maxY(points[0].y);
		for (size_t i = 1; i < points.size(); i++) {
			if (points[i].x < minX) minX = points[i].x;
			if (maxX < points[i].x) maxX = points[i].x;
			if (points[i].y < minY) minY = points[i].y;
			if (maxY < points[i].y) maxY = points[i].y;
		}
		if (minX == maxX || minY == maxY) {
			assert(false);
			return false;
		}
		if (!(minX < x && x < maxX && minY < y && y < maxY)) {
			return false;
		}

		//�жϵ��Ƿ���ĳ�������ϣ�����ǣ��ж�����������
		for (size_t i = 0; i < points.size(); i++) {
			if (i + 1 == points.size()) {
				if (PtInLine(x, y, points[i].x, points[i].y, points[0].x, points[0].y)) {
					return false;
				}
			}
			else {
				if (PtInLine(x, y, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y)) {
					return false;
				}
			}
		}

		//���߷��жϣ��ӣ�x,y����һ�����ߣ���������ཻ�ıߵ�����Ϊ��������˵������������
		intptr_t endX(x), endY(y+10000),nextPtX(0),nextPtY(0),result(0);
		intptr_t intersectX(0), intersectY(0);
		intptr_t count_intersect(0);//�����ཻ������
		std::vector<POINT> vertexs; //��¼���ߴ����Ķ���
		for (size_t i = 0; i < points.size(); i++) {
			if (i + 1 == points.size()) {
				nextPtX = points[0].x;
				nextPtY = points[0].y;
			}
			else {
				nextPtX = points[i + 1].x;
				nextPtY = points[i + 1].y;
			}
			result = GetIntersectPointByLines(x, y, endX, endY
				, points[i].x, points[i].y, nextPtX, nextPtY
				, &intersectX, &intersectY
			);
			if (result > 0) { //�������ཻ
				if ((intersectX == points[i].x && intersectY == points[i].y)
					|| (intersectX == nextPtX && intersectY == nextPtY)
					) {
					//��¼���ߴ����Ķ���
					bool exist(false);
					for (size_t j = 0; j < vertexs.size(); j++) {
						if (vertexs[j].x == intersectX && vertexs[j].y == intersectY) {
							exist = true;
							break;
						}
					}
					if (!exist) {
						vertexs.push_back({ (LONG)intersectX,(LONG)intersectY });
					}
				}
				else {
					count_intersect++;
				}
			}
		}
		return count_intersect % 2 > 0 || vertexs .size()%2>0;
	}

	Gdiplus::Color UIBase::ConvertToGDIPlusColor(const COLORREF& color) {
		unsigned char r, g, b;
		ColorHelp::ExtractRGB(color, r, g, b);
		return Gdiplus::Color(255, r, g, b);
	}

	SIZE UIBase::GetTextSize(const std::wstring& text
		, const Gdiplus::Font* pFont/*=nullptr*/
		, const Gdiplus::StringFormat* pStringFormat/*=nullptr*/)
	{
		if (text.length() < 1) {
			return { 0,0 };
		}

		HDC hdc = ::GetDC(m_hWnd);
		Gdiplus::Graphics graphic(hdc);
		SIZE sz = GetTextSize(graphic, text, pFont ? pFont : m_pFont, pStringFormat);
		::ReleaseDC(m_hWnd, hdc);
		return sz;
	}

	SIZE UIBase::GetTextSize(Gdiplus::Graphics& graphic
		, const std::wstring& text
		, const Gdiplus::Font* pFont
		, const Gdiplus::StringFormat* pStringFormat/*=nullptr*/)
	{
		if (text.length() < 1) {
			return { 0,0 };
		}
		Gdiplus::RectF rectf;
		if (pStringFormat) {
			graphic.MeasureString(text.c_str(), (int)text.length(), pFont, Gdiplus::PointF(0.0f, 0.0f), pStringFormat, &rectf);
		}
		else {
			graphic.MeasureString(text.c_str(), (int)text.length(), pFont, Gdiplus::PointF(0.0f, 0.0f), &rectf);
		}		

		/*Gdiplus::GraphicsPath graphicsPathObj;
		Gdiplus::FontFamily fontFamily;
		pFont->GetFamily(&fontFamily);
		Gdiplus::StringFormat fmt(Gdiplus::StringFormatFlags::StringFormatFlagsNoWrap
		);
		graphicsPathObj.AddString(text.c_str(),-1
			, &fontFamily,pFont->GetStyle(),pFont->GetSize()
			, Gdiplus::PointF(0.0f, 0.0f)
			, &fmt
		);
		graphicsPathObj.GetBounds(&rectf);*/

		// �����ı��Ŀ��
		return { (LONG)rectf.Width,(LONG)rectf.Height };
	}

	void UIBase::DrawText(Gdiplus::Graphics& graphic
		, const Gdiplus::Brush* pBrushText
		, const Gdiplus::Font* pFont
		, const std::wstring& text
		, Gdiplus::RectF rectf
		, const Gdiplus::StringFormat *pStringFormat/*=nullptr*/
	)
	{
		if (StringUtils::rtrim(text).length() < 1) {
			return;
		}

		//DrawString�����ĵĶ������Ӣ�ĵĸ�һ�㣬�����ٴ����㴦��
		bool allMultiChars = true;
		for (int i = 0; i < text.length(); i++) {
			if (text.at(i) <= 0x00ff) {
				allMultiChars = false;
				break;
			}
		}

		Gdiplus::StringFormat *pSF = const_cast<Gdiplus::StringFormat *>(pStringFormat);
		if (!pStringFormat) {
			pSF = new Gdiplus::StringFormat(Gdiplus::StringFormatFlags::StringFormatFlagsNoWrap
				| Gdiplus::StringFormatFlags::StringFormatFlagsNoClip
				| Gdiplus::StringFormatFlags::StringFormatFlagsNoFitBlackBox
				//| Gdiplus::StringFormatFlags::StringFormatFlagsNoFontFallback
			);
			pSF->SetAlignment(Gdiplus::StringAlignment::StringAlignmentCenter);
			pSF->SetLineAlignment(Gdiplus::StringAlignment::StringAlignmentCenter);
		}

		rectf.Y += allMultiChars ? WinSDKUtils::GetAdaptationSizeForDPI(3) : WinSDKUtils::GetAdaptationSizeForDPI(2);
		graphic.DrawString(text.c_str()
			, -1
			, pFont
			, rectf
			, pSF
			, pBrushText
		);
		if (!pStringFormat) {
			delete pSF;
		}
	}

	Gdiplus::Bitmap* UIBase::GrabImage()
	{
		if (m_pImageCatch) {
			delete m_pImageCatch;
			m_pImageCatch = nullptr;
		}

		RECT rect = GetRect();
		WinSDKUtils::ClientToScreen(m_hWnd, rect);

		HDC hDCScreen = ::CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
		HDC hDCTemp = ::CreateCompatibleDC(hDCScreen);
		HBITMAP hBitmap = ::CreateCompatibleBitmap(hDCScreen, (int)m_width, (int)m_height);
		HBITMAP oldBmp = (HBITMAP)::SelectObject(hDCTemp, hBitmap);
		::BitBlt(hDCTemp, 0, 0, (int)m_width, (int)m_height, hDCScreen, rect.left, rect.top, SRCCOPY);
		::SelectObject(hDCTemp, oldBmp);
		::DeleteDC(hDCTemp);
		::DeleteDC(hDCScreen);

		m_pImageCatch = new Gdiplus::Bitmap(hBitmap, NULL);
		::DeleteObject(hBitmap);
		return m_pImageCatch;
	}

	void UIBase::CopyBitmapToClipBoard(HBITMAP hBitmap)
	{
		::OpenClipboard(m_hWnd);
		::EmptyClipboard();
		SetClipboardData(CF_BITMAP, hBitmap);
		::CloseClipboard();
	}

	Gdiplus::Image *UIBase::LoadGdiplusImage(const std::wstring& imgName)
	{
		std::wstring path = WinSDKUtils::GetModulePath() + TEXT("\\res\\") + imgName;
		if (::_waccess(path.c_str(), 0) != 0) {
			return nullptr;
		}
		return new Gdiplus::Image(path.c_str());
	}

	RECT UIBase::FromGdiplusRectF(const Gdiplus::RectF& rectf) {
		RECT rect = { 0 };
		rect.left = (LONG)rectf.X;
		rect.top = (LONG)rectf.Y;
		rect.right = rect.left + (LONG)rectf.Width;
		rect.bottom = rect.top + (LONG)rectf.Height;
		return rect;
	}

	//////////////////////////////////////////////////////////////
	//Separator
	bool Separator::Create(const HWND& hWnd, const POINT& pt, const size_t& height)
	{		
		bool result = UIBase::Create(hWnd);
		m_x = pt.x;
		m_y = pt.y;
		m_width = 1;
		m_height = height;
		return result;
	}

	bool Separator::Draw(Gdiplus::Graphics& graphic, const PAINTSTRUCT& ps)
	{
		const intptr_t intersect = WinSDKUtils::IsIntersectRect(GetRect(), ps.rcPaint);
		if (!intersect) {
			return false;
		}

		//��䱳��ɫ
		graphic.FillRectangle(m_pBrushBK
			, (INT)m_x
			, (INT)m_y
			, (INT)m_width
			, (INT)m_height);

		return true;
	}
}