#include "stdafx.h"
#include "tab.h"
#include "../WinSDKUtils.h"
#include "../GdiplusHelp.h"
#include <math.h>

namespace lcc_direct_ui {
	const size_t Tab::ITEM_REGION_VERTEX_COUNT = 4;

	Tab::Tab() :UIBase()
		, m_pStringFormat(nullptr)
		, m_colorChecked(255, 162, 219, 236)
		, m_colorCheckedHover(255, 148, 214, 233)
		, m_colorDefault(255, 240, 240, 240)
		, m_colorDefaultHover(255, 220, 220, 220)
		, m_colorBorder(255, 100, 100, 100)
		, m_colorCloseBtnHover(255, 219, 68, 55)
	{
		m_itemWidth = 0;
		m_topWidth = 0;
		m_itemHeight = 0;
		m_bottomHeight = 0;
		m_leftMargin = 0;
		m_closeBtnRadius = 0;

		m_pStringFormat = new Gdiplus::StringFormat(Gdiplus::StringFormatFlags::StringFormatFlagsNoWrap
			| Gdiplus::StringFormatFlags::StringFormatFlagsNoClip
			| Gdiplus::StringFormatFlags::StringFormatFlagsNoFitBlackBox);
		m_pStringFormat->SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
		m_pStringFormat->SetLineAlignment(Gdiplus::StringAlignment::StringAlignmentCenter);

		m_pCallbackBeforeCloseTab = nullptr;
		m_pCallbackAfterCloseTab = nullptr;
		m_pCallbackAfterChangeTab = nullptr;
	}
	Tab::~Tab()
	{
		if (m_pStringFormat) {
			DELETE_GDIPLUS_OBJ(m_pStringFormat);
			m_pStringFormat = nullptr;
		}

		for (int i = 0; i < m_items.size(); i++) {
			delete m_items[i];
		}
		m_items.clear();
	}

	bool Tab::Create(const HWND& hWnd, const RECT* pRect/*= nullptr*/, const size_t& id/*=0*/)
	{
		m_itemWidth = WinSDKUtils::GetAdaptationSizeForDPI(150);
		m_topWidth = WinSDKUtils::GetAdaptationSizeForDPI(130);
		m_itemHeight = WinSDKUtils::GetAdaptationSizeForDPI(28);
		m_leftMargin = WinSDKUtils::GetAdaptationSizeForDPI(2);
		m_closeBtnRadius = WinSDKUtils::GetAdaptationSizeForDPI(6);
		m_tempForDrawCloseBtn = (intptr_t)::sqrt(m_closeBtnRadius * m_closeBtnRadius / 2)- WinSDKUtils::GetAdaptationSizeForDPI(1);

		if (m_itemHeight < WinSDKUtils::RectHeight(*pRect)) {
			m_bottomHeight = WinSDKUtils::RectHeight(*pRect) - m_itemHeight;
		}		

		UIBase::Create(hWnd, pRect, id);

		return true;
	}

	const Tab::ITEM* Tab::AddItem(const std::wstring& text, const intptr_t& userData/*=0*/, bool redraw/*=true*/)
	{
		ITEM *pItem = new ITEM();
		pItem->uuid = GenerateUUID32();
		pItem->userData = userData;
		pItem->state = 0;
		pItem->closeBtnState = 0;
		pItem->tipSize = { 0 };
		SetItemTitle(pItem,text);
		m_items.push_back(pItem);

		if (redraw) {
			Redraw();
		}
		return pItem;
	}

	bool Tab::CloseItem(const size_t& item, bool confirmBeforeClose/*=true*/)
	{
		if (m_items.size() < item + 1) {
			return true;
		}
		const intptr_t userData = m_items[item]->userData;

		//���ûص������ж��Ƿ���Լ����ر�
		if (confirmBeforeClose 
			&& m_pCallbackBeforeCloseTab 
			&& !m_pCallbackBeforeCloseTab(item, userData)) {
			return false;
		}

		//�ر���itemָ����tab��
		bool redraw(true);
		m_items.erase(m_items.begin() + item);

		//������һ��tab��
		intptr_t active = GetActiveIndex();
		if (active < 0) {
			if (item < m_items.size()) {
				SetActiveItem(item);
				redraw = false;
			}
			else if (item > 0) {
				SetActiveItem(item - 1);
				redraw = false;
			}
		}		
		if (redraw) {//�ػ�
			Redraw();
		}		

		//���ùرպ�Ļص�����
		if (m_pCallbackAfterCloseTab) {
			m_pCallbackAfterCloseTab(item, userData);
		}		
		return true;
	}

	void Tab::SetItemTitle(ITEM* pItem, const std::wstring& title)
	{
		pItem->text = title;
		//�����ı�����ʾ���
		if (pItem->text.length() > 0) {
			HDC hdc = ::GetDC(m_hWnd);
			Gdiplus::Graphics graphic(hdc);
			pItem->textSize = GetTextSize(graphic, title,m_pFont, m_pStringFormat);
			::ReleaseDC(m_hWnd, hdc);
		}
		else {
			pItem->textSize = { 0 };
		}
	}

	void Tab::SetItemTitle(const size_t& item, const std::wstring& title, bool redraw/*= true*/)
	{
		if (m_items.size() < item + 1 || m_items[item]->text.compare(title) == 0) {
			return;
		}
		SetItemTitle(m_items[item], title);

		if (redraw) {
			Redraw();
		}		
	}

	const Tab::ITEM* Tab::GetActiveItem() const
	{
		const intptr_t active = GetActiveIndex();
		return active > -1 ? m_items[active] : nullptr;
	}

	intptr_t Tab::GetActiveIndex() const
	{
		for (size_t i = 0; i < m_items.size(); i++) {
			if (m_items[i]->state & BTN_STATE_CHECKED) {
				return i;
			}
		}
		return -1;
	}

	void Tab::SetActiveItem(const size_t& item)
	{
		if (m_items.size() < item + 1) {
			return;
		}
		const intptr_t preActive = GetActiveIndex();
		if (item == preActive) {
			return;
		}

		if (preActive >= 0) {//���ǰһ��check��־
			m_items[preActive]->state &= ~BTN_STATE_CHECKED;
		}
		//���õ�ǰ���check��־
		m_items[item]->state |= BTN_STATE_CHECKED;
		if (m_pCallbackAfterChangeTab) {
			m_pCallbackAfterChangeTab(item, preActive);
		}
		Redraw();
	}

	bool Tab::Draw(Gdiplus::Graphics& graphic, const PAINTSTRUCT& ps)
	{
		intptr_t intersect = WinSDKUtils::IsIntersectRect(GetRect(), ps.rcPaint);
		if (0 == intersect) {
			return false;
		}

		int activeIndex(-1);
		Gdiplus::Point itemPolygon[ITEM_REGION_VERTEX_COUNT];
		RECT rectCloseBtn;

		Gdiplus::Pen penBorder(m_colorBorder);
		Gdiplus::SolidBrush brush(m_colorChecked);

		//���ײ����򱳾�ɫ
		RECT rcBottom = GetRect();
		rcBottom.top = rcBottom.bottom - int(m_bottomHeight);
		graphic.FillRectangle(&brush,rcBottom.left,rcBottom.top, WinSDKUtils::RectWidth(rcBottom), WinSDKUtils::RectHeight(rcBottom));

		//���Ʒǻtab
		for (int i = 0; i < m_items.size(); i++) {
			if (m_items[i]->state & BTN_STATE_CHECKED) {
				activeIndex = i;
				continue;
			}
			GetItemAreaForDraw(i, itemPolygon);   //���Tab�����������
			GetItemCloseBtnRect(i, rectCloseBtn); //���tab���еĹرհ�ť�ľ��ο�

			DrawTabItem(graphic, penBorder, brush, itemPolygon, rectCloseBtn, *(m_items[i]));
		}

		//���Ƶײ����ϱ���
		if (m_items.size() < 1) {
			graphic.DrawLine(&penBorder, rcBottom.left, rcBottom.top+1, rcBottom.right, rcBottom.top+1);
		}
		else {
			graphic.DrawLine(&penBorder, rcBottom.left, rcBottom.top + 1, int(rcBottom.left + m_leftMargin), rcBottom.top + 1);
			graphic.DrawLine(&penBorder
				, int(rcBottom.left + m_leftMargin + m_items.size()*(m_topWidth + (m_itemWidth - m_topWidth) / 2) + (m_itemWidth - m_topWidth) / 2)
				, rcBottom.top+1
				, rcBottom.right
				, rcBottom.top + 1);
		}		

		//���Ƶ�ǰ���tab
		if (activeIndex > -1) {
			GetItemAreaForDraw(activeIndex, itemPolygon);   //���Tab�����������
			GetItemCloseBtnRect(activeIndex, rectCloseBtn); //���tab���еĹرհ�ť�ľ��ο�
			DrawTabItem(graphic, penBorder, brush, itemPolygon, rectCloseBtn, *(m_items[activeIndex]));
		}

		return intersect == -1;
	}

	void Tab::DrawTabItem(Gdiplus::Graphics& graphic
		, Gdiplus::Pen& penBorder
		, Gdiplus::SolidBrush& brush
		, Gdiplus::Point *lpPtGDIPlus
		, const RECT& rectCloseBtn
		, const ITEM& item)
	{
		//���Ʊ���
		Gdiplus::Color color;
		if (item.state & BTN_STATE_CHECKED) {
			color = item.state & BTN_STATE_HOVER ? m_colorCheckedHover : m_colorChecked;
		}
		else {
			color = item.state & BTN_STATE_HOVER ? m_colorDefaultHover : m_colorDefault;
		}
		brush.SetColor(color);
		graphic.FillPolygon(&brush, lpPtGDIPlus, ITEM_REGION_VERTEX_COUNT);

		//���Ʊ���
		graphic.DrawLine(&penBorder, lpPtGDIPlus[0].X, lpPtGDIPlus[0].Y, lpPtGDIPlus[1].X, lpPtGDIPlus[1].Y);
		graphic.DrawLine(&penBorder, lpPtGDIPlus[1].X, lpPtGDIPlus[1].Y, lpPtGDIPlus[2].X, lpPtGDIPlus[2].Y);
		graphic.DrawLine(&penBorder, lpPtGDIPlus[2].X, lpPtGDIPlus[2].Y, lpPtGDIPlus[3].X, lpPtGDIPlus[3].Y);

		/////////////////////////////////////////////////
		//�����ı�
		Gdiplus::RectF rcText = { Gdiplus::REAL(lpPtGDIPlus[1].X)
			, Gdiplus::REAL(lpPtGDIPlus[1].Y)+ WinSDKUtils::GetAdaptationSizeForDPI(2)
			, Gdiplus::REAL(m_topWidth - (m_closeBtnRadius<<1))
			, Gdiplus::REAL(m_itemHeight)
		};
		if (item.textSize.cx + 2 < rcText.Width) {
			//�����ǰ���ο�����ȫ��ʾ�ı���ֱ�ӻ���
			rcText.X += (rcText.Width - item.textSize.cx) / 2;
			graphic.DrawString(item.text.c_str()
				, -1
				, m_pFont
				, rcText
				, m_pStringFormat
				, m_pBrushForText
			);
		}
		else {//�����ǰ���β�������ȫ��ʾ�ı����򴴽������һ��������ʧ��Ч��
			//1��������tab��������ڻ��Ʊ���
			rcText.Width = (Gdiplus::REAL)(lpPtGDIPlus[2].X- lpPtGDIPlus[1].X);
			graphic.DrawString(item.text.c_str()
				, -1
				, m_pFont
				, rcText
				, m_pStringFormat
				, m_pBrushForText
			);

			//2���Թرհ�ť���λ��Ϊ��㣬������佥��İ�͸������ɫ
			Gdiplus::Color colorTransparent(0, color.GetR(), color.GetG(), color.GetB());
			Gdiplus::RectF rcBrush = { 0
				, Gdiplus::REAL(rcText.Y)
				, Gdiplus::REAL(WinSDKUtils::GetAdaptationSizeForDPI(20))
				, Gdiplus::REAL(rcText.Height)
			};
			rcBrush.X = lpPtGDIPlus[2].X- (m_closeBtnRadius<<1) - rcBrush.Width;
			Gdiplus::LinearGradientBrush linGrBrush(rcBrush
				, colorTransparent //��1����ɫ
				, color            //��2����ɫ
				, 0                //����ɫ�ĽǶ�
			);
			graphic.FillRectangle(&linGrBrush, rcBrush);

			//3���Է�͸������ɫ���رհ�ť��������ı���
			rcBrush.Width = Gdiplus::REAL(m_closeBtnRadius << 1);
			rcBrush.X = lpPtGDIPlus[2].X - rcBrush.Width;
			graphic.FillRectangle(&brush, rcBrush);
		}		

		//���ƹرհ�ť
		if (item.closeBtnState != 100) { //100��ʾ����ʾ�رհ�ť
			Gdiplus::Color color(255,88, 88, 88);
			if (1 == item.closeBtnState) { //����ƶ������Ϸ������ư�ť�����Ը�����ʾ�ð�ť
				brush.SetColor(m_colorCloseBtnHover);
				graphic.FillEllipse(&brush, rectCloseBtn.left, rectCloseBtn.top, WinSDKUtils::RectWidth(rectCloseBtn), WinSDKUtils::RectHeight(rectCloseBtn));
				color = Gdiplus::Color(255, 255, 255, 255);
			}
			//����X
			Gdiplus::Pen pen(color,2);
			graphic.DrawLine(&pen
				, int(rectCloseBtn.right- m_closeBtnRadius- m_tempForDrawCloseBtn)
				, int(rectCloseBtn.bottom - m_closeBtnRadius - m_tempForDrawCloseBtn)
				, int(rectCloseBtn.left + m_closeBtnRadius + m_tempForDrawCloseBtn)
				, int(rectCloseBtn.top + m_closeBtnRadius + m_tempForDrawCloseBtn));
			graphic.DrawLine(&pen
				, int(rectCloseBtn.left + m_closeBtnRadius + m_tempForDrawCloseBtn)
				, int(rectCloseBtn.bottom - m_closeBtnRadius - m_tempForDrawCloseBtn)
				, int(rectCloseBtn.right - m_closeBtnRadius - m_tempForDrawCloseBtn)
				, int(rectCloseBtn.top + m_closeBtnRadius + m_tempForDrawCloseBtn));
		}
	}

	bool Tab::OnMouseMove(const POINT& pt)
	{
		if (!PtInArea(pt)) {
			return false;
		}
		if (pt.y >= GetRect().bottom- (int)m_bottomHeight) {//λ�ڵײ�������
			return true;
		}
		if (pt.x >= m_x + m_leftMargin + m_itemWidth * m_items.size()) {//λ�ڿհ�������
			return true;
		}

		RECT rcCloseBtn;
		bool redraw(false);
		for (int i = 0; i < m_items.size(); i++) {
			if (PtInItem(pt, i)) {
				if (!IsHover(i)) {
					m_items[i]->state |= BTN_STATE_HOVER;
					redraw = true;
				}

				//���ùرհ�ť��״̬
				if (m_items[i]->closeBtnState != 100) {
					GetItemCloseBtnRect(i,rcCloseBtn);
					if (::PtInRect(&rcCloseBtn,pt)) {
						if (m_items[i]->closeBtnState != 1) {
							m_items[i]->closeBtnState = 1;
							redraw = true;
						}
					}
					else if (1 == m_items[i]->closeBtnState) {
						m_items[i]->closeBtnState = 0;
						redraw = true;
					}
					
				}
			}
			else {
				if (IsHover(i)) {
					m_items[i]->state &= ~BTN_STATE_HOVER;
					redraw = true;
				}

				//���ùرհ�ť��״̬
				if (m_items[i]->closeBtnState != 100) {
					m_items[i]->closeBtnState = 0;
				}
			}
		}
		if (redraw) {
			Redraw();
		}

		return true;
	}

	bool Tab::OnLButtonDown(const POINT& pt)
	{
		if (!PtInArea(pt)) {
			return false;
		}
		if (pt.y >= GetRectBottom() - (int)m_bottomHeight) {//λ�ڵײ�������
			return true;
		}
		if (pt.x >= GetRectLeft() + m_leftMargin + m_itemWidth * m_items.size()) {//λ�ڿհ�������
			return true;
		}

		//�ж��Ƿ����˹رհ�ť
		RECT rcCloseBtn;
		for (int i = 0; i < m_items.size(); i++) {
			GetItemCloseBtnRect(i, rcCloseBtn);
			if (::PtInRect(&rcCloseBtn,pt)) {
				CloseItem(i);
				return true;
			}
		}

		//�ж��Ƿ�����ĳһ��tab��
		int active = HitTestForIndex(pt);
		if (active>-1 && !IsActive(active)) {
			SetActiveItem(active);
		}
		return true;
	}

	bool Tab::OnRButtonDown(const POINT& pt)
	{
		if (!OnLButtonDown(pt)) {
			return false;
		}

		//�����Ҽ��˵�
		int active = HitTestForIndex(pt);
		if (active > -1) {

		}
		return true;
	}

	bool Tab::OnLButtonDblClk(const POINT& pt)
	{
		if (!PtInArea(pt)) {
			return false;
		}
		if (pt.y >= GetRectBottom() - (int)m_bottomHeight) {//λ�ڵײ�������
			return true;
		}

		//˫���¼�����Ӧ�հ���
		if (pt.x >= GetRectLeft() + m_leftMargin + m_itemWidth * m_items.size()) {
			if (m_pCallbackDblClk) {
				m_pCallbackDblClk(intptr_t(&pt),0);
			}
		}
		return true;
	}

	bool Tab::GetItemArea(const size_t& item, std::vector<POINT>& points)
	{
		if (m_items.size() < item + 1) {
			return false;
		}

		Gdiplus::Point pts[ITEM_REGION_VERTEX_COUNT];
		GetItemAreaForDraw(item, pts);

		if (IsActive(item) || item+1==m_items.size()) {			
			points.resize(ITEM_REGION_VERTEX_COUNT);
			for (int i = 0; i < ITEM_REGION_VERTEX_COUNT; i++) {
				points[i].x = pts[i].X;
				points[i].y = pts[i].Y;
			}
		}
		else {
			points.resize(5);
			for (int i = 0; i < 3; i++) {
				points[i].x = pts[i].X;
				points[i].y = pts[i].Y;
			}

			//��ȡ��һ��tab����������ߵ�ֱ��λ��
			Gdiplus::Point ptsNext[ITEM_REGION_VERTEX_COUNT];
			GetItemAreaForDraw(item + 1, ptsNext);
			//��ǰtab���Ҳ������һ��tab����������ߵĽ���
			UIBase::GetIntersectPointByLines(
				pts[2].X, pts[2].Y, pts[3].X, pts[3].Y
				, ptsNext[0].X, ptsNext[0].Y, ptsNext[1].X, ptsNext[1].Y
				, (intptr_t*)&(points[3].x), (intptr_t*)&(points[3].y)
			);

			//points[2]Ϊ���ε����Ͻ�
			points[4].x = points[2].x;
			points[4].y = points[0].y;

			
		}

		return true;
	}

	bool Tab::GetItemAreaForDraw(const size_t& item, Gdiplus::Point *lpPtGDIPlus) {
		if (m_items.size() < item + 1) {
			return false;
		}

		//��1����(���ε����½�)
		lpPtGDIPlus[0].X = int(m_x + m_leftMargin)
			+ int(item*(m_topWidth + (m_itemWidth - m_topWidth) / 2));
		lpPtGDIPlus[0].Y = int(GetRectBottom() - m_bottomHeight)+1;
		//��2����(���ε����Ͻ�)
		lpPtGDIPlus[1].X = lpPtGDIPlus[0].X + (int)(m_itemWidth - m_topWidth) / 2;
		lpPtGDIPlus[1].Y = lpPtGDIPlus[0].Y - (int)m_itemHeight;
		//��3����(���ε����Ͻ�)
		lpPtGDIPlus[2].X = lpPtGDIPlus[1].X + (int)m_topWidth;
		lpPtGDIPlus[2].Y = lpPtGDIPlus[1].Y;
		//��4����(���ε����½�)
		lpPtGDIPlus[3].X = lpPtGDIPlus[0].X + (int)m_itemWidth;
		lpPtGDIPlus[3].Y = lpPtGDIPlus[0].Y;

		return true;
	}

	bool Tab::GetItemCloseBtnRect(const size_t& item, RECT& rect)
	{
		Gdiplus::Point points[ITEM_REGION_VERTEX_COUNT];
		if (!GetItemAreaForDraw(item, points)) {
			return false;
		}
		//point[2]Ϊ�������Ͻ�����
		rect.left = int(points[2].X- m_closeBtnRadius - m_closeBtnRadius);
		rect.top = int(points[2].Y + WinSDKUtils::GetAdaptationSizeForDPI(4));
		rect.right = int(rect.left + m_closeBtnRadius + m_closeBtnRadius);
		rect.bottom = int(rect.top + m_closeBtnRadius + m_closeBtnRadius);

		return true;
	}

	bool Tab::PtInItem(const POINT& pt, const size_t& item)
	{
		std::vector<POINT> points;
		if (!GetItemArea(item, points)) {
			return false;
		}
		return PtInRegion(pt.x, pt.y, points);
	}

	int Tab::HitTestForIndex(const POINT& pt) {
		if (!PtInArea(pt)) {
			return -1;
		}
		if (pt.y >= GetRectBottom() - (int)m_bottomHeight) {//λ�ڵײ�������
			return -1;
		}
		if (pt.x >= m_x + m_leftMargin + m_itemWidth * m_items.size()) {//λ�ڿհ�������
			return -1;
		}

		for (int i = 0; i < m_items.size(); i++) {
			if (PtInItem(pt, i)) {
				return i;
			}
		}
		return -1;
	}

	Tab::ITEM* Tab::HitTest(const POINT& pt)
	{
		int index = HitTestForIndex(pt);
		return index > -1 ? m_items[index] : nullptr;
	}

	bool Tab::SetItemData(const size_t& item, intptr_t data) {
		if (m_items.size() < item + 1) {
			return false;
		}
		else {
			m_items[item]->userData = data;
			return true;
		}
	}

	intptr_t Tab::GetActiveItemData() const
	{
		const ITEM* pActive = GetActiveItem();
		return pActive ? pActive->userData : 0;
	}

	intptr_t Tab::GetItemIndexByID(const std::wstring& id)
	{
		for (size_t i = 0; i < m_items.size(); i++) {
			if (m_items[i]->uuid.compare(id) == 0) {
				return i;
			}
		}
		return -1;
	}

	intptr_t Tab::GetItemIndexByUserData(const intptr_t& userdata)
	{
		for (size_t i = 0; i < m_items.size(); i++) {
			if (userdata == m_items[i]->userData) {
				return i;
			}
		}
		return -1;
	}

	Tab::ITEM* Tab::GetItemByID(const std::wstring& id)
	{
		intptr_t index = GetItemIndexByID(id);
		return index > -1 ? m_items[index] : nullptr;
	}

	Tab::ITEM* Tab::GetItem(intptr_t index)
	{
		if (index < 0 || (intptr_t)m_items.size() <= index) {
			return nullptr;
		}
		return m_items[index];
	}
}