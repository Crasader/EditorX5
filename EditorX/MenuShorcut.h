#pragma once
#include "MenuBase.h"
#include<string>
#include<vector>

class MenuShorcut : public MenuBase
{
public:
	MenuShorcut();
	virtual ~MenuShorcut();

	virtual BOOL Create(HINSTANCE hInstance, HWND hParent);
	virtual bool GetItemRect(const size_t& item, RECT *pItemRect) const;

protected:
	//���ؿ�ݷ�ʽ(������m_arrShortCut),���ؿ�ݷ�ʽ������
	static std::wstring m_shortcutFolderPath;

	size_t m_maxColCount; //�������
	//m_itemHeight��Ϊ�����ı߳�

	intptr_t LoadData();

	virtual void DrawItem(Gdiplus::Graphics& graphic
		, Gdiplus::Brush* pBrushItemBK
		, const ITEM* pItem
		, const RECT& rcItem
		, HDC hdc);
	virtual void OnLButtonDown(const POINT& pt);
};

