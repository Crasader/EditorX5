// EditorX.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "EditorX.h"
#include "EditorXFrame.h"
#include "./directui/uibase.h"
#include "GdiplusHelp.h"

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	std::wstring filepath = EditorXFrame::ParseCmdLine(lpCmdLine);

	//������и�ʵ��������ʾǰһ��ʵ��
	const HWND hPre = ::FindWindow(EditorXFrame::GetMainWndClassName().c_str(),NULL);
	if (hPre)
	{
		::PostMessage(hPre,WM_SYSCOMMAND, SC_RESTORE,0);
		if (filepath.length() > 0) {
			/* ������������ʹ�õ����໥���������������ڴ�ռ䣬
			 * ͬһ��ַ�Բ�ͬ�Ľ�����˵����һ��ָ��ͬһ�����ڴ棬����Ҳ�Ͳ�һ��һ����
			 * ��˲�ͬ�����޷�ͨ������ַ�ķ�ʽ�����ַ���������ͬһ�����µĲ�ͬ�߳��ǿ��Եģ�
			 */
			//::SendMessage(hPre,EditorXFrame::WM_OPENFILE,0,LPARAM(filepath.c_str()));
			COPYDATASTRUCT data;
			data.dwData = 1;
			data.lpData = (wchar_t*)filepath.data();
			data.cbData = (DWORD)(filepath.length()*2);
			::SendMessage(hPre,WM_COPYDATA,0,LPARAM(&data));
		} else {
			//�½�һ��tabҳ
			::SendMessage(hPre,WM_COMMAND,MAKELONG(IDM_NEWFILE,0),0);
		}
		
		return FALSE;
	}

	/*
	 * ��������֧��DPI���ţ�����߷ֱ���������д��ڵ��������ʾ��ݣ�����::GetDeviceCaps(hdc,LOGPIXELSX)ʱ���ص�Ҳ��96
	 * �����������˺þã��²�MFC����û����Ӧ�����Զ������˸ú���
	 */
	SetProcessDPIAware();

	// Initialize GDI+.
	GdiplusHelp::LoadGdiplus();

	// ִ��Ӧ�ó����ʼ��:
	if (!EditorXFrame::GetInstance().InitInstance(hInstance, nCmdShow))
	{
		GdiplusHelp::UnloadGdiplus();
		return FALSE;
	}
	if (filepath.length() > 0) {
		EditorXFrame::GetInstance().OpenFile(filepath.c_str());
	}

	// ����Ϣѭ��:
	MSG msg;
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EDITORX));
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	GdiplusHelp::UnloadGdiplus();
	return (int) msg.wParam;
}