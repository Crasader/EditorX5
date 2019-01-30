#pragma once
#include "BaseWnd.h"
#include "./directui/uibase.h"
#include<string>
#include<vector>

class MenuBase : public BaseWnd
{
public:
	struct ITEM {
		std::wstring text;
		SIZE textSize;
		size_t id;
		HICON hIcon;
		SIZE iconSize;// ͼ��Ĵ�С

		//�û��Զ�������
		std::wstring strUserData;
		intptr_t nUserData;

		//lcc_direct_ui::BTN_STATE_...
		size_t state;
	};
public:
	MenuBase();
	virtual ~MenuBase();

	const ITEM* HitTest(const POINT& pt);
	intptr_t HitTestForIndex(const POINT& pt) const;
	//@param pItemRect[out]
	bool HitTest(const POINT& pt,const int& item,RECT *pItemRect=nullptr);
	virtual bool GetItemRect(const size_t& item,RECT *pItemRect) const;

	virtual void Popup(intptr_t x, intptr_t y);
	virtual void Popup();
	virtual BOOL Create(HINSTANCE hInstance, HWND hParent);
	virtual ITEM* AddItem(const std::wstring& text, const size_t& id, HICON hIcon);

protected:
	std::vector<ITEM*> m_items;
	size_t m_itemHeight;
	size_t m_width;
	size_t m_height;
	//�����Դ�ŵ��������
	size_t m_maxItemCount;

	//��������ı߽�߾�
	size_t m_marginLeft;
	size_t m_marginTop;

	Gdiplus::Color m_colorBK;
	Gdiplus::Color m_colorItemBKDefault;
	Gdiplus::Color m_colorItemBKHover;
	Gdiplus::Font *m_pFont;
	Gdiplus::Pen *m_pPenBorder;//���ڻ���ָ����ɫ�ı߿�
	Gdiplus::Brush *m_pBrushForText;
	Gdiplus::StringFormat *m_pStringFormat;

	bool m_redrawBackgroud;

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void OnPaint(HDC& hdc, PAINTSTRUCT& ps);
	inline bool IsHover(const ITEM* pItem) const { return pItem->state & lcc_direct_ui::BTN_STATE_HOVER; };


	virtual void Draw(Gdiplus::Graphics& graphic, PAINTSTRUCT& ps);
	virtual void DrawItem(Gdiplus::Graphics& graphic
		, Gdiplus::Brush* pBrushItemBK
		, const ITEM* pItem
		, const RECT& rcItem
		, HDC hdc
	) {}
	virtual void OnMouseMove(const POINT& pt);
	virtual void OnLButtonDown(const POINT& pt);
};
