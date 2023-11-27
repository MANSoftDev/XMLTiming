// maindlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINDLG_H__43CF0372_E6DA_4248_B6EC_C135AE36A39B__INCLUDED_)
#define AFX_MAINDLG_H__43CF0372_E6DA_4248_B6EC_C135AE36A39B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

enum ALARM_TYPES {VEH_ID, TRN_ID, CMD, ALARM };

typedef struct tagTESTDATA
{
	int nID;
	TCHAR szName[10];
	short sDataType;
	ALARM_TYPES eType;
	short sBit;
	long lClient;
	long lServer;
	TCHAR szValue[4];
} TESTDATA;

class CMainDlg :	public CDialogImpl<CMainDlg>, 
					public CUpdateUI<CMainDlg>,
					public CMessageFilter,	
					public CIdleHandler
{
public:
	CMainDlg();
	~CMainDlg();

	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(ID_BEGIN, OnBegin)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(wID);
		return 0;
	}

	void CloseDialog(int nVal)
	{
		DestroyWindow();
		::PostQuitMessage(nVal);
	}

	LRESULT OnBegin(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	double FindXML(long lVal, bool bClient);
	double FindArrayClient(long lVal);
	double FindArrayServer(long lVal);
	CString GetAttribute(CString strName, IXMLDOMNode* pNode);
	bool LoadLists();

	CListBox m_ClientList;
	CListBox m_ServerList;
	
	CComPtr<IXMLDOMDocument2>	m_pDoc;
	long m_nCount;
	TESTDATA* m_data;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINDLG_H__43CF0372_E6DA_4248_B6EC_C135AE36A39B__INCLUDED_)
