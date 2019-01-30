#pragma once
#include <string>
#include "MenuBase.h"

/*��ʷ�ļ���¼*/
class MenuFileHistory : public MenuBase
{
public:
	MenuFileHistory();
	virtual ~MenuFileHistory();

	virtual BOOL Create(HINSTANCE hInstance, HWND hParent);
	virtual ITEM* AddItem(const std::wstring& text, const size_t& id, HICON hIcon);

protected:
	//��˵�����������(m_maxItemCount)�ȴ���ӵ��ļ���ʷ
	std::vector<std::wstring> m_arrFileHistoryForAdd;

	intptr_t LoadData();

	virtual void DrawItem(Gdiplus::Graphics& graphic
		, Gdiplus::Brush* pBrushItemBK
		, const ITEM* pItem
		, const RECT& rcItem
		, HDC hdc);
	virtual void OnLButtonDown(const POINT& pt);
};

