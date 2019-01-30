#pragma once

//"����"�Ի���
class DlgAbout
{
public:
	~DlgAbout();
	static DlgAbout& GetInstance();

	//������ģʽ�Ի���
	void ShowInModeless();

protected:
	static DlgAbout m_instance;
	DlgAbout();

	HWND m_hDlgModeless;
	RECT m_rcDlg;
	HBRUSH m_brushBK;

	static INT_PTR CALLBACK OnDlgMsg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

