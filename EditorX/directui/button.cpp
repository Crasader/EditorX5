#include "stdafx.h"
#include "button.h"
#include "UIFrame.h"
#include "../WinSDKUtils.h"
#include "../GdiplusHelp.h"
#include "../ColorHelp.h"

namespace lcc_direct_ui {
	Button::Button() : UIBase() 
		, m_colorChecked(255,200, 200, 200)
		, m_colorCheckedHover(255, 170, 170, 170)
		, m_colorDefault(255, 250, 250, 250)
		, m_colorDefaultHover(255, 230, 230, 230)
	{
		m_pImage = nullptr;
		m_imageShowSize = { 0 };
		m_state = 0;
		m_style = BTN_STYLE::BS_DEFAULT;
	}

	Button::~Button()
	{
		if (m_pImage) {
			DELETE_GDIPLUS_OBJ(m_pImage)
			m_pImage;
		}
	}

	bool Button::Create(const HWND& hWnd, const RECT& rect, const StringUtils::T_STRING& text, const size_t& id/* = 0*/)
	{
		UIBase::Create(hWnd, &rect, id);
		m_text = text;
		return true;
	}

	bool Button::Create(const HWND& hWnd
		, const RECT* pRect
		, const StringUtils::T_STRING& image
		, const StringUtils::T_STRING& tip
		, const size_t& id/* = 0*/)
	{
		UIBase::Create(hWnd, pRect, id);
		m_text = TEXT("");
		m_tip = tip;
		m_pImage = LoadGdiplusImage(image);
		if (!m_pImage) {
			assert(false);
			return false;
		}
		m_imageShowSize.cx = m_imageShowSize.cy = (LONG)WinSDKUtils::GetAdaptationSizeForDPI(32);
		//m_imageShowSize.cx = m_pImage->GetWidth();
		//m_imageShowSize.cy = m_pImage->GetHeight();
		
		return true;
	}

	bool Button::Draw(Gdiplus::Graphics& graphic, const PAINTSTRUCT& ps)
	{
		const intptr_t intersect = WinSDKUtils::IsIntersectRect(GetRect(), ps.rcPaint);
		if (!intersect) {
			return false;
		}

		Gdiplus::Color color;
		if (IsHover()) { //����ڰ�ť���ƶ�
			color = (IsChecked() ? m_colorCheckedHover : m_colorDefaultHover);
		}
		else {
			color = (IsChecked() ? m_colorChecked : m_colorDefault);
		}

		//Բ�ǵİ뾶
		const intptr_t radius = WinSDKUtils::GetAdaptationSizeForDPI(5);
		
		//���Ʊ���
		if (m_style != BTN_STYLE::BS_ONLYTEXT || IsHover()) {
			//���������������ػ����ʱ���о��
			graphic.FillRectangle(m_pBrushBK
				, (INT)m_x
				, (INT)m_y
				, (INT)m_width
				, (INT)m_height);

			//����ʾ�ı��İ�ť��hoverʱ�ı���ɫ���⴦��
			Gdiplus::SolidBrush solidBrush(color);
			if (BTN_STYLE::BS_ONLYTEXT == m_style && IsHover()) {
				solidBrush.SetColor(Gdiplus::Color(255, 234, 243, 248));
			}

			if (BTN_STYLE::BS_ELLIPSE == m_style) { //������Բ				
				graphic.FillEllipse(&solidBrush
					, (INT)m_x
					, (INT)m_y
					, (INT)m_width
					, (INT)m_height
				);
			}
			else {//����Բ�Ǿ���
				GdiplusHelp::FillRoundRect(graphic,&solidBrush
					, (INT)m_x, (INT)m_y, (INT)m_width,(INT)m_height,(INT)radius
				);
			}
		}

		if (m_pImage) {//����ͼ��
			graphic.DrawImage(m_pImage
				, (Gdiplus::REAL)(GetRectLeft() + (m_width - m_imageShowSize.cx) / 2)
				, (Gdiplus::REAL)(GetRectTop() + (m_height - m_imageShowSize.cy) / 2)
				, (Gdiplus::REAL)m_imageShowSize.cx
				, (Gdiplus::REAL)m_imageShowSize.cy
			);
		}
		else {//��������
			if (BTN_STYLE::BS_ONLYTEXT == m_style) { //������������֣���ͨ��������ɫ���ְ�ť״̬
				m_pBrushForText->SetColor(color);
			}
			else {
				m_pBrushForText->SetColor(Gdiplus::Color(255, 0, 0, 0));
			}
			DrawText(graphic, m_pBrushForText, m_pFont, m_text, GetRectF());
		}		

		return intersect == -1;
	}

	bool Button::OnMouseMove(const POINT& pt)
	{
		if (PtInArea(pt)) {
			if (!IsHover()) {
				SetHover(true);
			}
			if (m_tip.length() > 0) {
				POINT point = { (LONG)(GetRectLeft() + WinSDKUtils::GetAdaptationSizeForDPI(3))
					, (LONG)(GetRectBottom() - WinSDKUtils::GetAdaptationSizeForDPI(3))
				};
				UIFrame::GetInstance().ShowTip(this, m_tip, &point);
			}			
			return true;
		}
		else {
			if (IsHover()) {
				SetHover(false);
			}
			UIFrame::GetInstance().HideTip(this);
			return false;
		}
	}

	bool Button::OnLButtonDown(const POINT& pt)
	{
		if (!PtInArea(pt)) {
			return false;
		}
		SetChecked(true);
		return true;
	}

	bool Button::OnLButtonUp(const POINT& pt)
	{
		if (!PtInArea(pt)) {
			return false;
		}
		SetChecked(false);
		::PostMessage(m_hWnd, WM_COMMAND, MAKELONG(GetId(), 0), 0);
		return true;
	}
}
