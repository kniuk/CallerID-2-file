// Port2FileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Port2File.h"
#include "Port2FileDlg.h"
#include ".\port2filedlg.h"
#include "consts.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const TCHAR STRING_DISABLE [] =  _T("[Stopped]");
const TCHAR STRING_ENABLE  [] =  _T("[Started]");
const TCHAR STRING_ERROR   [] =  _T("[Error]");  
//jaca
const UINT WM_TRAY = WM_USER + 1;


/////////////////////////////////////////////////////////////
//
//                   CRedirectDlg dialog
//
/////////////////////////////////////////////////////////////

CRedirectDlg::CRedirectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRedirectDlg::IDD, pParent)
	, m_bFileAppend(FALSE)
	, m_sLogFile(_T(""))
	, m_sLogFile2(_T(""))
	, m_iCurItemSel(0)
	, m_sRecvBytes(_T("0 bytes"))
	, m_sTotalFileSize(_T("0 calls"))
	, m_sTotal2FileSize(_T("0 bytes"))
	, m_sErrorStatus(_T(""))
	, m_minToTray(0)
	, m_minimizeOnComOpen(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hIcon2 = AfxGetApp()->LoadIcon(IDI_PLUG);
}

void CRedirectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PORTS, m_ListPorts);
	DDX_Check(pDX, IDC_CHECK_APPEND, m_bFileAppend);
	DDX_Control(pDX, IDC_CB_BAUDRATE, m_cbBaudrate);
	DDX_Control(pDX, IDC_CB_DATABITS, m_cbDataBits);
	DDX_Control(pDX, IDC_CB_PARITY, m_cbParity);
	DDX_Control(pDX, IDC_CB_STOPBITS, m_cbStopBits);
	DDX_Control(pDX, IDC_CB_FC, m_cbFC);
	DDX_Text(pDX, IDC_ED_LOGFILE, m_sLogFile);
	DDX_Text(pDX, IDC_STATIC_RECV,  m_sRecvBytes);
	DDX_Text(pDX, IDC_STATIC_TOTAL, m_sTotalFileSize);
	DDX_Text(pDX, IDC_STATIC_TOTAL2, m_sTotal2FileSize);
	DDX_Text(pDX, IDC_STATIC_STATUS, m_sErrorStatus);
}

BEGIN_MESSAGE_MAP(CRedirectDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BT_VIEW, OnBnClickedBtView)
	ON_BN_CLICKED(IDC_BT_STARTLOG, OnBnClickedBtStartlog)
	ON_BN_CLICKED(IDC_CHECK_APPEND, OnBnClickedCheckAppend)
	ON_CBN_SELCHANGE(IDC_CB_BAUDRATE, OnCbnSelchangeCbBaudrate)
	ON_CBN_SELCHANGE(IDC_CB_DATABITS, OnCbnSelchangeCbDatabits)
	ON_CBN_SELCHANGE(IDC_CB_PARITY, OnCbnSelchangeCbParity)
	ON_CBN_SELCHANGE(IDC_CB_STOPBITS, OnCbnSelchangeCbStopbits)
	ON_CBN_SELCHANGE(IDC_CB_FC, OnCbnSelchangeCbFc)
	ON_EN_CHANGE(IDC_ED_LOGFILE, OnEnChangeEdLogfile)
	ON_MESSAGE  (WM_USER_UPDATE_WND, OnWriteUpdate )
	ON_MESSAGE  (WM_USER_CHANGE_STATUS, OnWriteStatus  )
	ON_NOTIFY   (NM_CUSTOMDRAW,  IDC_LIST_PORTS, OnCustomdrawList)
	ON_BN_CLICKED(IDC_BUTTON1, &CRedirectDlg::OnBnClickedButton1)
	ON_MESSAGE(WM_TRAY, OnTrayMsg)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CRedirectDlg message handlers

BOOL CRedirectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// comm initialization 
	m_IconList.Create( 16, 16, ILC_MASK | ILC_COLOR24, 1, 1 );  
	m_IconList.Add( AfxGetApp() ->LoadIcon( MAKEINTRESOURCE( IDI_PLUG ) ) );
	m_ListPorts.SetExtendedStyle( LVS_EX_FULLROWSELECT );
	m_ListPorts.SetImageList( &m_IconList, LVSIL_SMALL );

	CWnd* pWnd = (CWnd*)GetDlgItem( IDC_LIST_PORTS );
	CRect rect;
	pWnd ->GetClientRect( &rect );
	m_ListPorts.InsertColumn( 0, _T(""), LVCFMT_LEFT, int(rect.Width() / 2 - 0.05 * rect.Width())  );
	m_ListPorts.InsertColumn( 1, _T(""), LVCFMT_LEFT, int(rect.Width() / 2 - 0.05 * rect.Width())  );
	
	m_minimizeOnComOpen = true;

	// fill controls 
    for ( int i=0; i<NUM_BAUDRATE; i++ )
	{
		m_cbBaudrate.AddString( VAL_BAUDRATE[i] );
	}
	m_cbBaudrate.SetCurSel( 6 );

	for ( int i=0; i<NUM_DATABITS; i++ )
	{
		m_cbDataBits.AddString( VAL_DATABITS[i] );
	}
	m_cbDataBits.SetCurSel( 3 );

	for ( int i=0; i<NUM_PARITY; i++ )
	{
		m_cbParity.AddString( VAL_PARITY[i] );
	}
	m_cbParity.SetCurSel( 0 );

	for ( int i=0; i<NUM_STOPBIT; i++ )
	{
		m_cbStopBits.AddString( VAL_STOPBIT[i] );
	}
	m_cbStopBits.SetCurSel( 0 );

	for ( int i=0; i<NUM_FC; i++ )
	{
		m_cbFC.AddString( VAL_FC[i] );
	}
	m_cbFC.SetCurSel( 2 );

	GetAvailablePorts();

	//jaca
	// Declare NOTIFYICONDATA details. 
	// Error handling is omitted here for brevity. Do not omit it in your code.

	m_nid.cbSize = sizeof(m_nid);
	m_nid.hWnd = GetSafeHwnd(); //hWnd;
	m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;// | NIF_GUID;
	m_nid.uCallbackMessage = WM_TRAY;

	// Note: This is an example GUID only and should not be used.
	// Normally, you should use a GUID-generating tool to provide the value to
	// assign to guidItem.
	
	//static const GUID myGUID =
	//{ 0x23977b55, 0x10e0, 0x4041, {0xb8, 0x62, 0xb1, 0x95, 0x41, 0x96, 0x36, 0x69} };
	//m_nid.guidItem = myGUID;

	//m_nid.guidItem = guid;

	// This text will be shown as the icon's tooltip.
	strcpy_s(m_nid.szTip, ARRAYSIZE(m_nid.szTip), "CallerID 2 file");

	// Load the icon for high DPI.
	//LoadIconMetric(GetModuleHandleA(NULL), MAKEINTRESOURCE(IDI_SMALL), LIM_SMALL, &(m_nid.hIcon));
	//m_nid.hIcon = LoadIcon(GetModuleHandleA(NULL), MAKEINTRESOURCE(IDI_APPLICATION));
	m_nid.hIcon = m_hIcon; // = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// Show the notification.
	Shell_NotifyIcon(NIM_ADD, &m_nid) ? S_OK : E_FAIL;
	SetTimer(1, 5000, NULL); //set timer to hide Window
	//jaca koniec
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRedirectDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CRedirectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

////////////////////////////////////////////////////////////////////////////////////
//
//							control handling 
//
/////////////////////////////////////////////////////////////////////////////////////
void CRedirectDlg::OnEnChangeEdLogfile()
{
	UpdateData();
	if ( m_sLogFile.GetLength() && m_sLogFile2.GetLength() )
	{
		CWnd* pWnd = (CWnd*)GetDlgItem( IDC_BT_STARTLOG );
		pWnd ->EnableWindow( TRUE );
	}
	else
	{
		CWnd* pWnd = (CWnd*)GetDlgItem( IDC_BT_STARTLOG );
		pWnd ->EnableWindow( FALSE );	
	}

	m_Ports[ m_iCurItemSel ] ->m_sLogFile = m_sLogFile; 
	m_Ports[m_iCurItemSel]->m_sLogFile2 = m_sLogFile2;
}

void CRedirectDlg::OnBnClickedBtView()
{
	static TCHAR BASED_CODE szFilter[] = _T("Text Files (*.txt)|*.txt|All Files (*.*)|*.*||");
	
	CFileDialog dlg( FALSE, _T("*.txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter );						
	if ( dlg.DoModal() == IDOK )
	{
		m_sLogFile = dlg.GetPathName();
		m_sLogFile2 = dlg.GetPathName() + ".dbg";
		
		if ( m_sLogFile.GetLength() && m_sLogFile2.GetLength() )
		{
			CWnd* pWnd = (CWnd*)GetDlgItem( IDC_BT_STARTLOG );
			pWnd ->EnableWindow( TRUE );
		}

		m_Ports[ m_iCurItemSel ] ->m_sLogFile = m_sLogFile;
		m_Ports[m_iCurItemSel]->m_sLogFile2 = m_sLogFile2;
	}

	CEdit* pEdit = (CEdit*)GetDlgItem( IDC_ED_LOGFILE );
	pEdit ->SetWindowText( m_sLogFile );
	pEdit ->SetSel( m_sLogFile.GetLength(), m_sLogFile.GetLength() );

	UpdateData( FALSE );
}

void CRedirectDlg::OnBnClickedBtStartlog()
{
	if ( m_Ports[ m_iCurItemSel ] ->m_bLogStarted )
	{
		// stop logging  
		m_Ports[ m_iCurItemSel ] ->Close();
		m_Ports[ m_iCurItemSel ] ->m_bLogStarted = FALSE;
		m_sErrorStatus = m_Ports[m_iCurItemSel] ->m_sStatusMessage;
		UpdateSettings();
	}
	else
	{
		// run logging
		CString sBaudrate;
		m_cbBaudrate.GetLBText( m_cbBaudrate.GetCurSel(), sBaudrate );
		DWORD dwBaudrate = _ttoi( sBaudrate ); 
		int iBits     = m_cbDataBits.GetCurSel() + 5;
		int iParity   = m_cbParity.GetCurSel();
		int iStopbits = m_cbStopBits.GetCurSel();   
		int iFC		  = m_cbFC.GetCurSel();
		
		UpdateData();
		m_Ports[ m_iCurItemSel ] ->m_sLogFile = m_sLogFile;
		m_Ports[m_iCurItemSel]->m_sLogFile2 = m_sLogFile2;

		if ( (iBits != -1 && iParity != -1 && iStopbits != -1) &&
			 m_Ports[ m_iCurItemSel ] ->Open( dwBaudrate, iBits, iParity, iStopbits, iFC ) )
		{
			m_Ports[ m_iCurItemSel ] ->m_bLogStarted = TRUE;
			UpdateSettings();
		}
		else
		{
			m_ListPorts.SetItemText( m_iCurItemSel, 1, STRING_ERROR );					
		}
	}
}

void CRedirectDlg::OnCbnSelchangeCbBaudrate()
{
	CString sBaudrate;
	m_cbBaudrate.GetLBText( m_cbBaudrate.GetCurSel(), sBaudrate );
	m_Ports[ m_iCurItemSel ] ->m_dwBaudRate = _ttoi( sBaudrate ); 
	m_Ports[ m_iCurItemSel ] ->SetBaudrate( m_Ports[ m_iCurItemSel ] ->m_dwBaudRate ); 
}

void CRedirectDlg::OnCbnSelchangeCbDatabits()
{
	m_Ports[ m_iCurItemSel ] ->m_iIndexDataBits = m_cbDataBits.GetCurSel();
	m_Ports[ m_iCurItemSel ] ->SetDataBits( m_cbDataBits.GetCurSel() + 4 );
}

void CRedirectDlg::OnCbnSelchangeCbParity()
{
	m_Ports[ m_iCurItemSel ] ->m_iIndexParity = m_cbParity.GetCurSel();
	m_Ports[ m_iCurItemSel ] ->SetParity( m_cbParity.GetCurSel() );
}

void CRedirectDlg::OnCbnSelchangeCbStopbits()
{
	m_Ports[ m_iCurItemSel ] ->m_iIndexStopBits = m_cbStopBits.GetCurSel();
	m_Ports[ m_iCurItemSel ] ->SetStopBits( m_cbStopBits.GetCurSel() );
}

void CRedirectDlg::OnCbnSelchangeCbFc()
{
	m_Ports[ m_iCurItemSel ] ->m_iIndexFlowCtrl = m_cbFC.GetCurSel();
	m_Ports[ m_iCurItemSel ] ->SetFlowControl( m_cbFC.GetCurSel() );	
}

void CRedirectDlg::OnBnClickedCheckAppend()
{
	UpdateData(); 
	m_Ports[m_iCurItemSel] ->m_bAppend = m_bFileAppend;
}

void CRedirectDlg::OnOK()
{
	OnBnClickedBtStartlog();
}

void CRedirectDlg::OnCancel()
{
	//if( MessageBox( _T("Czy na pewno chcesz zamkn¹æ? Odczyt dzwoni¹cych numerów nie bêdzie mo¿liwy."), _T("CLIP2file"), 
	if (MessageBox(_T("Really close ? It will disable recognition of incoming calls numbers."), _T("CLIP2file"),
					MB_ICONQUESTION | MB_YESNO ) == IDYES )
	{
		for ( UINT i=0; i<m_Ports.size(); i++ )
		{
			CRedirectPort* pPort = m_Ports[i];
			pPort ->SaveSettings();
			delete pPort; 
		}
		//jaca
		Shell_NotifyIcon(NIM_DELETE, &m_nid);
		CDialog::OnCancel();
	}
}

BOOL CRedirectDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR *nmhdr = (NMHDR*)lParam;  

	if ( nmhdr ->hwndFrom == m_ListPorts.m_hWnd )
	{
		if ( nmhdr->code == LVN_ITEMCHANGED )
		{
			
			LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam;

			if ( pnmv ->iItem != m_iCurItemSel  )
			{
				m_Ports[m_iCurItemSel] ->m_bIsActive = false;  
				m_iCurItemSel = pnmv ->iItem;
				m_Ports[m_iCurItemSel] ->m_bIsActive = true;  
				UpdateSettings();  
			}
		}		
	}

	return CDialog::OnNotify(wParam, lParam, pResult);
}

LRESULT CRedirectDlg::OnWriteUpdate ( WPARAM, LPARAM )
{
	UpdateCounters();

	UpdateData( FALSE );
	
	return 0;
}

LRESULT CRedirectDlg::OnTrayMsg  ( WPARAM, LPARAM lParam)
{
	switch (lParam)
	{
	case WM_LBUTTONDBLCLK:
		if (m_minToTray==2) {
			m_minToTray = 0;
			ShowWindow(SW_SHOW);
			BringWindowToTop();
			UpdateWindow();
		}
		else {
			m_minToTray = 2;
			ShowWindow(SW_HIDE);
			UpdateWindow();
		}
		break;
	}
	return 0;
}

LRESULT CRedirectDlg::OnWriteStatus ( WPARAM wp, LPARAM lp )
{
	// lp - status code 
	if ( wp == m_iCurItemSel )
	{
		m_sErrorStatus = m_Ports[m_iCurItemSel] ->m_sStatusMessage;
	}
	
	if ( lp != SC_OPEN_SUCCESS && lp != SC_NO_ERROR )
	{
		m_ListPorts.SetItemText( (int)wp, 1, STRING_ERROR );	
	}
	UpdateData( FALSE ); 
	return 0;
}

void CRedirectDlg::OnCustomdrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);

	*pResult = 0;

	if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else
	{
		if (CDDS_ITEM == pLVCD->nmcd.dwDrawStage)
		{
			TRACE("dfssdafas");
		}


		if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
		{
			if (!m_Ports[pLVCD->nmcd.dwItemSpec]->m_bLogStarted)
			{
				pLVCD->clrText = RGB(128, 128, 128);
			}

			*pResult = CDRF_DODEFAULT;
		}
	}
}

void CRedirectDlg::OnBnClickedButton1()
{
	//jaca
	m_minToTray = 2;
	ShowWindow(SW_HIDE);
	UpdateWindow();
}

void CRedirectDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Dodaj tutaj swój kod procedury obs³ugi komunikatów i/lub wywo³aj domyœlny
	if (IsWindowVisible() && m_minToTray == 1) {
		KillTimer(1);
		m_minToTray = 2;
		ShowWindow(SW_HIDE);
		UpdateWindow();
	}
	CDialog::OnTimer(nIDEvent);
}

////////////////////////////////////////////////////////////////////////////////
//
//					user functions utilites 						
//
///////////////////////////////////////////////////////////////////////////////

void CRedirectDlg::GetAvailablePorts()
{
	CStringList portNameList;   
		
	if ( CCommPort::GetAvailablePorts( &portNameList ) )
	{
		m_Ports.reserve( portNameList.GetCount() );

		CString sDefaultPath;
		TCHAR	*Buf = sDefaultPath.GetBufferSetLength (1024);
		GetModuleFileName (NULL, Buf, 1024);
		*(_tcsrchr (Buf, _T('\\'))) = 0;
		sDefaultPath.ReleaseBuffer ();
		sDefaultPath += _T("\\");
		sDefaultPath += _T("Logs");
		CreateDirectory( sDefaultPath, NULL );		
		
		POSITION pos = portNameList.GetHeadPosition();
		for ( int i=0; i< portNameList.GetCount(); i++ )
		{
			LV_ITEM item;
			item.mask = LVIF_IMAGE | LVIF_TEXT;
			item.iItem = m_ListPorts.GetItemCount();
			
			CString sPortName = portNameList.GetNext( pos );
			TCHAR buffer[255]; 
			_tcsncpy( buffer, sPortName, 255 );
			item.pszText = buffer;
			item.iImage = 0;
			item.iSubItem = 0;

			m_Ports.push_back( new CRedirectPort( sPortName, (i==0)?true:false, this, i ) );	

			m_Ports[i] ->LoadSettings();

			if ( m_Ports[i] ->m_sLogFile.IsEmpty() )
			{
				CString sCommDefaultPath = sDefaultPath;
				sCommDefaultPath += _T("\\"); 
				sCommDefaultPath += sPortName;
				sCommDefaultPath += _T("port");
				sCommDefaultPath += _T(".txt");
        		m_Ports[i] ->m_sLogFile = sCommDefaultPath;
				m_Ports[i]->m_sLogFile2 = sCommDefaultPath + ".dbg";
			}
					
			m_ListPorts.InsertItem( &item );
			
			if ( m_Ports[i] ->m_bLogStarted && m_Ports[ i ] ->Open()  )
			{
				//jaca
				if (m_minimizeOnComOpen && m_minToTray==0) {
					m_minToTray = 1; //trigger hiding window when possible (allow to show first, after dialog initialization)
				}
				//jaca koniec
				m_ListPorts.SetItemText( item.iItem, 1, STRING_ENABLE );

				//set started port as selected
				m_ListPorts.EnsureVisible(item.iItem, FALSE);
				m_ListPorts.SetFocus();
				m_ListPorts.SetItemState(item.iItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			}
			else
			{
				m_ListPorts.SetItemText( item.iItem, 1, STRING_DISABLE );
			}
		}

		UpdateSettings();
	}
}

// set active port settings 
void CRedirectDlg::UpdateSettings()
{
	m_sLogFile = m_Ports[m_iCurItemSel] ->m_sLogFile;
	m_sLogFile2 = m_Ports[m_iCurItemSel]->m_sLogFile2;

	if ( m_Ports[ m_iCurItemSel ] ->m_bLogStarted )
	{
		CWnd* pWnd = (CWnd*)GetDlgItem(IDC_BT_STARTLOG);
		pWnd->SetWindowText(_T("Stop logging"));

		pWnd = (CWnd*)GetDlgItem(IDC_ED_LOGFILE);
		pWnd->EnableWindow(FALSE);

		pWnd = (CWnd*)GetDlgItem(IDC_BT_VIEW);
		pWnd->EnableWindow(FALSE);

		pWnd = (CWnd*)GetDlgItem(IDC_CHECK_APPEND);
		pWnd->EnableWindow(FALSE);

		pWnd = (CWnd*)GetDlgItem(IDC_CB_BAUDRATE);
		pWnd->EnableWindow(FALSE);

		pWnd = (CWnd*)GetDlgItem(IDC_CB_DATABITS);
		pWnd->EnableWindow(FALSE);

		pWnd = (CWnd*)GetDlgItem(IDC_CB_PARITY);
		pWnd->EnableWindow(FALSE);

		pWnd = (CWnd*)GetDlgItem(IDC_CB_STOPBITS);
		pWnd->EnableWindow(FALSE);

		pWnd = (CWnd*)GetDlgItem(IDC_CB_FC);
		pWnd->EnableWindow(FALSE);

		m_ListPorts.SetItemText(m_iCurItemSel, 1, STRING_ENABLE);

	}
	else
	{
		CWnd* pWnd = (CWnd*)GetDlgItem(IDC_BT_STARTLOG);
		pWnd->SetWindowText(_T("Start logging"));
		if (m_sLogFile.GetLength() && m_sLogFile2.GetLength())
		{
			pWnd->EnableWindow(TRUE);
		}
		else
		{
			pWnd->EnableWindow(FALSE);
		}

		pWnd = (CWnd*)GetDlgItem(IDC_ED_LOGFILE);
		pWnd->EnableWindow(TRUE);

		pWnd = (CWnd*)GetDlgItem(IDC_BT_VIEW);
		pWnd->EnableWindow(TRUE);

		pWnd = (CWnd*)GetDlgItem(IDC_CHECK_APPEND);
		pWnd->EnableWindow(TRUE);

		pWnd = (CWnd*)GetDlgItem(IDC_CB_BAUDRATE);
		pWnd->EnableWindow(TRUE);

		pWnd = (CWnd*)GetDlgItem(IDC_CB_DATABITS);
		pWnd->EnableWindow(TRUE);

		pWnd = (CWnd*)GetDlgItem(IDC_CB_PARITY);
		pWnd->EnableWindow(TRUE);

		pWnd = (CWnd*)GetDlgItem(IDC_CB_STOPBITS);
		pWnd->EnableWindow(TRUE);

		pWnd = (CWnd*)GetDlgItem(IDC_CB_FC);
		pWnd->EnableWindow(TRUE);

		m_ListPorts.SetItemText(m_iCurItemSel, 1, STRING_DISABLE);
	}

	m_bFileAppend = m_Ports[m_iCurItemSel] ->m_bAppend;

	CString sBaudrate; 
	sBaudrate.Format( _T("%d"), m_Ports[m_iCurItemSel] ->m_dwBaudRate );
	m_cbBaudrate.SetWindowText( sBaudrate );
	
	m_cbDataBits.SetCurSel( m_Ports[m_iCurItemSel] ->m_iIndexDataBits );
	m_cbParity.SetCurSel  ( m_Ports[m_iCurItemSel] ->m_iIndexParity   );
	m_cbStopBits.SetCurSel( m_Ports[m_iCurItemSel] ->m_iIndexStopBits );
	m_cbFC.SetCurSel	  ( m_Ports[m_iCurItemSel] ->m_iIndexFlowCtrl );


	UpdateCounters();

	UpdateData( FALSE ); //FALSE = UI controls updated with dialog member variables

	CEdit* pEdit = ( CEdit*)GetDlgItem( IDC_ED_LOGFILE );
	pEdit ->SetWindowText( m_sLogFile );
	pEdit ->SetSel( m_sLogFile.GetLength() - 1, m_sLogFile.GetLength() );

	m_ListPorts.RedrawItems(m_iCurItemSel, m_iCurItemSel);
	m_ListPorts.Invalidate(TRUE);
	m_ListPorts.UpdateWindow();
}

void CRedirectDlg::UpdateCounters()
{
	m_sTotalFileSize.Format(_T("%d calls"), m_Ports[m_iCurItemSel]->GetFileTotalBytes());
	m_sTotal2FileSize.Format(_T("%d bytes"), m_Ports[m_iCurItemSel]->GetFileTotal2Bytes());
	m_sRecvBytes.Format(_T("%d bytes"), m_Ports[m_iCurItemSel]->GetFileWrittenBytes());
	m_sErrorStatus = m_Ports[m_iCurItemSel]->m_sStatusMessage;
}