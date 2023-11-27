// XMLTiming.cpp : main source file for XMLTiming.exe
//

#include "stdafx.h"
#include "resource.h"

#include "maindlg.h"

const int LOOPS = 5;

CMainDlg::CMainDlg()
{

}

CMainDlg::~CMainDlg()
{
	if( m_pDoc )
		m_pDoc.Release();
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);
	
	m_ClientList.Attach(GetDlgItem(IDC_CLIENT_LIST));
	m_ServerList.Attach(GetDlgItem(IDC_SERVER_LIST));

	if( SUCCEEDED(m_pDoc.CoCreateInstance(L"Msxml2.DOMDocument.4.0")) && m_pDoc)
	{
		short bSuccess;
		CComVariant varVal(_T("config_test.xml"));

		if( FAILED(m_pDoc->load(varVal, &bSuccess) ) )
			return FALSE;			
	}
	else
		return FALSE;
	
	if( !LoadLists() )
		return FALSE;

	return TRUE;
}

LRESULT CMainDlg::OnBegin(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	double dXMLTime = 0;
	double dArrayTime = 0;

	// What list is selected
	if( m_ClientList.GetCurSel() != LB_ERR )
	{
		long lVal = m_ClientList.GetItemData(m_ClientList.GetCurSel());
		
		// Perform this test 5 times
		for( int x = 0; x < LOOPS; x++ )
			dXMLTime += FindXML(lVal, true);

		// Get the average
		dXMLTime /= LOOPS;

		// Perform this test 5 times
		for( x = 0; x < LOOPS; x++ )
			dArrayTime += FindArrayClient(lVal);
		
		// Get the average
		dArrayTime /= LOOPS;
	}
	else if( m_ServerList.GetCurSel() != LB_ERR )
	{
		long lVal = m_ServerList.GetItemData(m_ServerList.GetCurSel());
		
		// Perform this test 5 times
		for( int x = 0; x < LOOPS; x++ )
			dXMLTime += FindXML(lVal, false);
		
		// Get the average
		dXMLTime /= LOOPS;

		// Perform this test 5 times
		for( x = 0; x < LOOPS; x++ )
			dArrayTime += FindArrayServer(lVal);

		// Get the average
		dArrayTime /= LOOPS;
	}


	TCHAR* szBuf1 = new TCHAR[10];
	sprintf(szBuf1, _T("%2.10f"), dXMLTime);
	
	TCHAR* szBuf2 = new TCHAR[10];
	sprintf(szBuf2, _T("%2.10f"), dArrayTime);

	CString strTime;
	strTime.Format(_T("XML: %s sec\nArray: %s sec"), szBuf1, szBuf2);
	
	SetDlgItemText(IDC_TIME, strTime);

	// Clear any selections
	m_ClientList.SetCurSel(-1);
	m_ServerList.SetCurSel(-1);

	return 0;
}

bool CMainDlg::LoadLists()
{
	USES_CONVERSION;
	
	// Only interested in the tag elements
	CString strParam(_T("*/*/tag"));

	CComPtr<IXMLDOMNodeList> pNodeList;
	if( SUCCEEDED(m_pDoc->selectNodes(A2OLE(strParam), &pNodeList)) && pNodeList)
	{
		pNodeList->get_length(&m_nCount);
		m_data = new TESTDATA[m_nCount];
		int x = 0;

		// Iterate through the list
		CComPtr<IXMLDOMNode> pNode;
		while( pNodeList->nextNode(&pNode) == S_OK )
		{
			CString strVal = GetAttribute(_T("client_id"), pNode);
			// Fill in the list boxes
			int nIndex = m_ClientList.AddString(strVal);
			m_ClientList.SetItemData(nIndex, atol(strVal)); 
			
			strVal = GetAttribute(_T("server_id"), pNode);
			nIndex = m_ServerList.AddString(strVal);
			m_ServerList.SetItemData(nIndex, atol(strVal)); 


			// Fill in the data struct
			strVal = GetAttribute(_T("id"), pNode);
			m_data[x].nID = atoi(strVal);
			
			strVal = GetAttribute(_T("client_id"), pNode);
			m_data[x].lClient = atol(strVal);

			strVal = GetAttribute(_T("server_id"), pNode);
			m_data[x].lServer = atol(strVal);

			strVal = GetAttribute(_T("bit"), pNode);
			m_data[x].sBit = atoi(strVal);

			strVal = GetAttribute(_T("datatype"), pNode);
			if( strVal == "BOOL" )
				m_data[x].sDataType = 1;
			else if( strVal == "INT" )
				m_data[x].sDataType = 2;
				
			strVal = GetAttribute(_T("type"), pNode);
			if( strVal == "CMD" )
				m_data[x].eType = CMD;
			else if( strVal == "ALARM" )
				m_data[x].eType = ALARM;
			if( strVal == "VEH_ID" )
				m_data[x].eType = VEH_ID;
			if( strVal == "TRN_ID" )
				m_data[x].eType = TRN_ID;
			
			strVal = GetAttribute(_T("name"), pNode);
			_tcscpy(m_data[x].szName, strVal);

			strVal = GetAttribute(_T("value"), pNode);
			_tcscpy(m_data[x].szValue, strVal);

			// Release and go to the next node
			x++;
			pNode.Release();
		}
	}
	else
		return false;

	return true;	
}

CString CMainDlg::GetAttribute(CString strName, IXMLDOMNode* pNode)
{
	HRESULT hr = S_OK;

	CComPtr<IXMLDOMNamedNodeMap> pNodeMap;
	if( SUCCEEDED(hr = pNode->get_attributes(&pNodeMap)) && pNodeMap)
	{
		CComPtr<IXMLDOMNode> pAttributeNode;
		if( SUCCEEDED(hr = pNodeMap->getNamedItem(strName.AllocSysString(), &pAttributeNode)) && pAttributeNode)
		{
			CComVariant varVal;
			if( SUCCEEDED(pAttributeNode->get_nodeValue(&varVal)) )
				return CString(varVal.bstrVal);
		}
	}

	return _T("");
}

double CMainDlg::FindXML(long lVal, bool bClient)
{
	CString strParam;
	// Check what we are looking for and set the
	// XPath properly
	if( bClient )
		strParam.Format(_T("*/*/tag[@client_id=\"%ld\"]"), lVal);
	else
		strParam.Format(_T("*/*/tag[@server_id=\"%ld\"]"), lVal);

	
	LARGE_INTEGER liStart, liEnd, liAPI, liFreq, liDiff;
	QueryPerformanceFrequency(&liFreq);
	QueryPerformanceCounter(&liStart);
	QueryPerformanceCounter(&liEnd);
	// Need to figure the overhead for calling the API
	liAPI.QuadPart = liEnd.QuadPart - liStart.QuadPart;
	QueryPerformanceCounter(&liStart);

	// Find the node with this client_id attribute
	CComPtr<IXMLDOMNode> pNode = NULL;
	if( SUCCEEDED(m_pDoc->selectSingleNode(strParam.AllocSysString(), &pNode)) && pNode)
	{
		QueryPerformanceCounter(&liEnd);
		liDiff.QuadPart = liEnd.QuadPart - liStart.QuadPart - liAPI.QuadPart;
		return (double)liDiff.QuadPart / (double)liFreq.QuadPart;
	}
	
	return -1;
}

double CMainDlg::FindArrayClient(long lVal)
{
	LARGE_INTEGER liStart, liEnd, liAPI, liFreq, liDiff;
	QueryPerformanceFrequency(&liFreq);
	QueryPerformanceCounter(&liStart);
	QueryPerformanceCounter(&liEnd);
	// Need to figure the overhead for calling the API
	liAPI.QuadPart = liEnd.QuadPart - liStart.QuadPart;
	QueryPerformanceCounter(&liStart);

	for(long x = 0; x < m_nCount; x++ )
	{
		if( m_data[x].lClient == lVal )
			break;
	}

	QueryPerformanceCounter(&liEnd);
	liDiff.QuadPart = liEnd.QuadPart - liStart.QuadPart - liAPI.QuadPart;
	double dSec = (double)liDiff.QuadPart / (double)liFreq.QuadPart;

	return dSec;
}

double CMainDlg::FindArrayServer(long lVal)
{
	LARGE_INTEGER liStart, liEnd, liAPI, liFreq, liDiff;
	QueryPerformanceFrequency(&liFreq);
	QueryPerformanceCounter(&liStart);
	QueryPerformanceCounter(&liEnd);
	// Need to figure the overhead for calling the API
	liAPI.QuadPart = liEnd.QuadPart - liStart.QuadPart;
	QueryPerformanceCounter(&liStart);

	for(long x = 0; x < m_nCount; x++ )
	{
		if( m_data[x].lServer == lVal )
			break;
	}

	QueryPerformanceCounter(&liEnd);
	liDiff.QuadPart = liEnd.QuadPart - liStart.QuadPart - liAPI.QuadPart;
	double dSec = (double)liDiff.QuadPart / (double)liFreq.QuadPart;

	return dSec;
}