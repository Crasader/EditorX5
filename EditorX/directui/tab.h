#pragma once
#include<string>
#include<vector>
#include "uibase.h"

namespace lcc_direct_ui {
	//tabҳ
	class Tab : public UIBase {
	public:
		struct ITEM {
			//Ψһ��ʶ��ǰtab
			std::wstring uuid;

			std::wstring text;
			SIZE textSize;

			size_t state;  //Tab��״̬:lcc_direct_ui::BTN_STATE_* 
			size_t closeBtnState;//�رհ�ť��״̬��0-������ʾ��1-mouse hover ,100-����

			std::wstring tip;
			SIZE tipSize;

			//�û��Զ�������
			intptr_t userData;
		};
		typedef intptr_t(FUNC_CALLBACK)(intptr_t,intptr_t);

	public:
		Tab();
		virtual ~Tab();

		bool Create(const HWND& hWnd, const RECT* pRect = nullptr, const size_t& id = 0);
		const ITEM* AddItem(const std::wstring& text,const intptr_t& userData=0,bool redraw=true);
		//�ر�ĳ��tab��رշ���true,δ�رշ���false
		bool CloseItem(const size_t& item, bool confirmBeforeClose=true);

		//tab��ر�ǰ�¼��Ļص�����
		void SetCallbackBeforeCloseTab(FUNC_CALLBACK* pFunc) { m_pCallbackBeforeCloseTab = pFunc; }
		//tab��رպ��¼��Ļص�����
		void SetCallbackAfterCloseTab(FUNC_CALLBACK* pFunc) { m_pCallbackAfterCloseTab = pFunc; }
		//tab���л��¼��Ļص�����
		void SetCallbackAfterChangeTab(FUNC_CALLBACK* pFunc) { m_pCallbackAfterChangeTab = pFunc; }
		//Tab˫���¼��Ļص�����������˫���հ�������Tab�ʱ�Ż���øú���
		void SetCallbackDblClk(FUNC_CALLBACK* pFunc) { m_pCallbackDblClk = pFunc; }

		void SetActiveItem(const size_t& item);
		const ITEM* GetActiveItem() const;
		intptr_t GetActiveIndex() const;
		int HitTestForIndex(const POINT& pt);
		ITEM* HitTest(const POINT& pt);

		/*
		* ���tab���ʵ������,GetItemAreaForDraw��õ���tab�����������������
		* @param points[out] ����Ķ���
		*/
		bool GetItemArea(const size_t& item, std::vector<POINT>& points);
		bool PtInItem(const POINT& pt, const size_t& item);

		inline size_t GetItemCount() const { return m_items.size(); }
		inline std::wstring GetItemTitle(const size_t& item) const { return (m_items.size() < item + 1) ? 0 : m_items[item]->text; }
		void SetItemTitle(const size_t& item, const std::wstring& title,bool redraw=true);
		inline std::wstring GetItemID(const size_t& item) const { return (m_items.size() < item + 1) ? 0 : m_items[item]->uuid; }
		intptr_t GetItemIndexByUserData(const intptr_t& userdata);
		intptr_t GetItemIndexByID(const std::wstring& id);
		ITEM* GetItemByID(const std::wstring& id);
		ITEM* GetItem(intptr_t index);

		//Ϊĳ��tab����û��Զ�������
		bool SetItemData(const size_t& item, intptr_t data);
		inline intptr_t GetItemData(const size_t& item) const { return (m_items.size() < item + 1) ? 0 : m_items[item]->userData; }
		intptr_t GetActiveItemData() const;

		inline bool IsActive(const size_t& item) const { return (m_items.size() < item + 1) ? false : IsActive(m_items[item]); }
		inline bool IsActive(const ITEM* pItem) const { return pItem->state & BTN_STATE_CHECKED; }
		inline bool IsHover(const size_t& item) const { return (m_items.size() < item + 1) ? false : (m_items[item]->state & BTN_STATE_HOVER); }

		virtual bool Draw(Gdiplus::Graphics& graphic, const PAINTSTRUCT& ps);
		virtual bool OnMouseMove(const POINT& pt);
		virtual bool OnLButtonDown(const POINT& pt);
		virtual bool OnLButtonDblClk(const POINT& pt);
		virtual bool OnRButtonDown(const POINT& pt);

	protected:
		//tab����������Ķ�����������Σ�ͨ��Ϊ4
		static const size_t ITEM_REGION_VERTEX_COUNT;

		std::vector<ITEM*> m_items;

		size_t m_itemWidth; //tab�Ŀ�ȣ������ε��±߾�
		size_t m_topWidth;  //���ε��ϱ߾�
		size_t m_itemHeight;//���εĸ߶�
		size_t m_bottomHeight; //�ײ�����ĸ߶�
		size_t m_leftMargin;   //���հ����Ŀ��
		size_t m_closeBtnRadius; //ÿ��Tab���Ϲرհ�ť�İ뾶����

		//�ñ�������DrawTabItem�б�ʹ�ã���Create�б���ֵ�����䶨��Ϊ���Ա��Ϊ����߼���Ч��
		intptr_t m_tempForDrawCloseBtn;
		Gdiplus::StringFormat *m_pStringFormat;

		Gdiplus::Color m_colorChecked;          //ѡ��ʱ����ɫ
		Gdiplus::Color m_colorCheckedHover;     //ѡ��ʱ,��������ƶ�ʱ����ɫ
		Gdiplus::Color m_colorDefault;          //��ѡ��ʱ����ɫ
		Gdiplus::Color m_colorDefaultHover;     //��ѡ��ʱ,��������ƶ�ʱ����ɫ
		Gdiplus::Color m_colorBorder;           //�߿����ɫ
		Gdiplus::Color m_colorCloseBtnHover;    //�رհ�ťhoverʱ�ı�����ɫ

		/*
		 * ���tab����������������,GetItemArea��õ���tab���ʵ������
		 * @param lpPoints[out] �����������Ϊ4�����Σ�
		 */
		bool GetItemAreaForDraw(const size_t& item, Gdiplus::Point *lpPtGDIPlus);
		bool GetItemCloseBtnRect(const size_t& item, RECT& rect);
		void DrawTabItem(Gdiplus::Graphics& graphic
			, Gdiplus::Pen& penBorder
			, Gdiplus::SolidBrush& brush
			, Gdiplus::Point *lpPtGDIPlus
			, const RECT& rectCloseBtn
			, const ITEM& item);
		void SetItemTitle(ITEM* pItem, const std::wstring& title);

		/**
		* tab��ر�ǰ�¼��Ļص�����
		* @param index ���رյ�tab�������
		* @param userData ���رյ�tab��󶨵��û�����
		* @return TRUE-���Լ����ر�,FALSE-�����Թر�
		*/
		FUNC_CALLBACK* m_pCallbackBeforeCloseTab;
		/**
		* tab��رպ��¼��Ļص�����
		* @param index ���رյ�tab�������,���������ø�ֵ����m_items[index]����Ϊ�ѱ�ɾ��
		* @param userData ���رյ�tab��󶨵��û�����
		* @return ���Է���ֵ
		*/
		FUNC_CALLBACK* m_pCallbackAfterCloseTab;
		/**
		* tab���л����¼��Ļص�����
		* @param index �л����tab�������
		* @param preActive �л�ǰ�Ļtab�������
		* @return ���Է���ֵ
		*/
		FUNC_CALLBACK* m_pCallbackAfterChangeTab;
		/**
		 * Tab˫���¼��Ļص�����������˫���հ�������Tab�ʱ�Ż���øú���
		 * @param pPoint �����������
		 * @param reserve ����
		 * @return ���Է���ֵ
		 */
		FUNC_CALLBACK* m_pCallbackDblClk;
	};
}