#pragma once
#include<vector>
#include "uibase.h"
#include "tooltip.h"

/* ��DirectUI��ʽ���ɵĽ�����������������UIBaseΪ����,����UIFrame�������н������
 * ����Gdiplu���ƽ���
 * ���ڳ�������ʱ�����lcc_direct_ui::UIBase::LoadGdiplus������Gdiplus����
 * ����ر�ʱ����lcc_direct_ui::UIBase::UnloadGdiplus�Թر�Gdiplus����
 * ��� 201802
 */
namespace lcc_direct_ui {
	/**
	 * UI��ܣ����ڼ��ɸ�directUI���
	 * ����GDIPlus���л��ƣ�������Ҫ�ڳ����ʼ��ʱ����Gdiplus::GdiplusStartup��GdiplusShutdown
	 */
	class UIFrame : public UIBase
	{
	public:
		virtual ~UIFrame();

		static inline UIFrame& GetInstance() { return m_instance; }
		bool Create(const HINSTANCE& hInst,const HWND& hWnd, const RECT& rect);
		bool AddUIObject(UIBase *pUIObj);

		//���pPtΪnull����Ĭ�Ͼ�����ʾ
		void Hint(const std::wstring& message,const POINT* pPt=nullptr);

		void ShowTip(UIBase* pUIObj, const std::wstring& tip, const POINT* pPt = nullptr);
		inline void HideTip(const UIBase* pUIObj) { if (m_pTooltip) m_pTooltip->HideTip(pUIObj); }

		//��ȡ����UI�����е�����Z��λ��
		size_t GetMaxZOrder() const;

		UIBase* GetUIObjectByID(const size_t& id) const;

		//@return true-��������Ҫ�������ƺ���������
		bool Draw(const HDC& hdc, const PAINTSTRUCT& ps);
		bool OnMouseMove(const POINT& pt);
		bool OnLButtonDown(const POINT& pt);
		bool OnLButtonDblClk(const POINT& pt);
		bool OnLButtonUp(const POINT& pt);
		bool OnRButtonDown(const POINT& pt);

		HBITMAP GrabEditorBackgroud();

	protected:
		friend class EditorXFrame;
		static UIFrame m_instance;
		UIFrame();

		static HBITMAP m_hEditorBackgroud;

		HCURSOR m_hCursorSystem;
		HCURSOR m_hCursorCurrent;
		HCURSOR m_hCursorHand;

		HINSTANCE m_hInst; //Ӧ�õ���ʵ�����
		HWND m_hWnd;
		Gdiplus::Color m_colorBK; //����ɫ

		/*
		 * �洢���е�UI����
		 */
		std::vector<UIBase*> m_uiobjs;
		Tooltip *m_pTooltip;

		//for ��ʾ��Ϣ
		std::wstring m_messageText;
		Gdiplus::RectF m_messageRect;
		Gdiplus::StringFormat *m_pSfForMessage;
		Gdiplus::Brush *m_pBrushForMessageBK;
		bool DrawHint(Gdiplus::Graphics& graphic, const PAINTSTRUCT& ps);
		//��ʱ������ʾ��Ϣ
		static DWORD WINAPI DelayHideHintThread(LPVOID lpParam);

	private:
		//��ֹ���ø���ĺ���
		bool Create(const HWND& hWnd, const RECT* pRect = nullptr, const size_t& id = 0) { return false; }
		bool Draw(Gdiplus::Graphics& graphic, const PAINTSTRUCT& ps) { return false; }
	};

};

