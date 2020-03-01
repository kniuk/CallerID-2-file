#include "StdAfx.h"
#include ".\redirectport.h"

CRedirectPort::CRedirectPort(const CString& sPortName, bool IsActive, CWnd* pWnd, int nIndex ):
	 CCommPort()
{
	m_bIsActive  = IsActive;
	m_pParentWnd = pWnd;
	ASSERT( m_pParentWnd  );

	m_sPortName = sPortName; 
	m_sLogFile  = _T(""); 
	m_sLogFile2 = _T("");
	m_sStatusMessage = _T("Ready to start");

	m_bLogStarted = FALSE;
    m_bAppend     = FALSE;
	
    m_nIndex = nIndex;
	// this is indexes 
	m_dwBaudRate     = 9600;
	m_iIndexDataBits = 3;
	m_iIndexParity   = 0;
	m_iIndexStopBits = 0;
	m_iIndexFlowCtrl = 2;
 	
	m_stBytesInFile  = 0;
	m_stBytesInFile2 = 0;
	m_stBytesWritten = 0;

	strTel[0] = '\0';
}

CRedirectPort::~CRedirectPort(void)
{
	if ( IsOpen() )
		Close();
}

void CRedirectPort::OnRxChar( DWORD dwCount )
{
	BYTE buffer[4096];
    DWORD dwSymbolsRead = Read( (VOID*)buffer, 4096 );
	DWORD dwTelCallCnt = 0;
	
	strncat((CHAR*)strTel, (CHAR*)buffer, dwSymbolsRead);

	string bufferTel((CHAR*)strTel);
	int posTel = -1;
	string stmp = "NMBR =  ";
	posTel = bufferTel.find(stmp);
	while( posTel != -1) {
		bufferTel = bufferTel.substr(posTel + stmp.length());
		posTel = bufferTel.find('\r');
		if (posTel != -1) {
			//jest ca³y numer tel do znaku koñca linii
			//zapisz datê i nr tel w pliku
			dwTelCallCnt += 1;
			time_t rawtime;
			struct tm* timeinfo;
			char buffTmp[80];
			char buffTmp2[80];
			char buffTmp3[80];
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			strftime(buffTmp, sizeof(buffTmp), "%d-%m-%Y %H:%M:%S;", timeinfo);
			strcat(buffTmp, bufferTel.substr(0, posTel).c_str());
			strftime(buffTmp2, sizeof(buffTmp2), "%Y%m%d_%H%M%S_", timeinfo);
			strcat(buffTmp2, bufferTel.substr(0, posTel).c_str());
			strftime(buffTmp3, sizeof(buffTmp), "%d-%m-%Y %H:%M:%S ", timeinfo);
			strcat(buffTmp3, bufferTel.substr(0, posTel).c_str());
			CFile tmpFile;
			UINT nOpenFlags = CFile::modeWrite | CFile::shareDenyNone | CFile::modeCreate;
			if (tmpFile.Open(m_sLogFileBase + buffTmp2 + ".log", nOpenFlags))
			{
				try
				{
					tmpFile.Write(buffTmp, strlen(buffTmp));
					tmpFile.Close();
					m_sStatusMessage = _T(buffTmp3);
				}
				catch (CFileException * e)
				{
					TCHAR* lpszCause = m_sStatusMessage.GetBufferSetLength(255);
					if (!e->GetErrorMessage(lpszCause, 255))
						m_sStatusMessage = _T("File error");
					m_pParentWnd->PostMessage(WM_USER_UPDATE_WND, m_nIndex, SC_FILE_EXCEPTION);
					e->Delete();
				}
			}
			else
			{
				m_sStatusMessage = _T("Unable open file for call log");
				m_pParentWnd->PostMessage(WM_USER_CHANGE_STATUS, m_nIndex, SC_OPEN_FILE_ERROR);
			}
			
			m_File.Write(buffTmp, strlen(buffTmp));
			m_File.Write("\r\n", 2);

			//reset the modem, because sometimes it fails to recognized next incoming call
			InitModem();
		}
		posTel = bufferTel.find(stmp);
	}
	strcpy((CHAR*)strTel, bufferTel.c_str());

	try
	{
		m_File2.Write(buffer, dwSymbolsRead);
		m_stBytesWritten += dwSymbolsRead;
		m_stBytesInFile  += dwTelCallCnt; //zapisz liczbê rozpoznanych po³¹czeñ
		m_stBytesInFile2 += dwSymbolsRead;
		if (m_bIsActive)
		{
			m_pParentWnd->PostMessage(WM_USER_UPDATE_WND, 0, 0);
		}
	}
	catch( CFileException *e )
	{
		 

		TCHAR *lpszCause = m_sStatusMessage.GetBufferSetLength( 255 );

		if ( !e ->GetErrorMessage( lpszCause, 255 ) )
			m_sStatusMessage = _T("File error");
		
		m_pParentWnd ->PostMessage( WM_USER_UPDATE_WND, m_nIndex, SC_FILE_EXCEPTION );

		e->Delete();
	}
}

BOOL CRedirectPort::Open()
{
	return Open( m_dwBaudRate, m_iIndexDataBits + 5, m_iIndexParity, m_iIndexStopBits, m_iIndexFlowCtrl );
}

BOOL CRedirectPort::Open( DWORD dwBaudrate, BYTE bDataBits, BYTE bParity, BYTE bStopBits, BYTE bFC  )
{
	UINT nOpenFlags = CFile::modeWrite | CFile::shareDenyNone | CFile::modeCreate;
	if( m_bAppend ) 
	{
		nOpenFlags |= CFile::modeNoTruncate;
	}

	if ( m_File.Open( m_sLogFile, nOpenFlags ) && m_File2.Open(m_sLogFile2, nOpenFlags))
	{
		m_stBytesWritten = 0;
		m_stBytesInFile = 0; //calls

		if ( m_bAppend ) 
		{
			m_File.SeekToEnd();
			m_File2.SeekToEnd();
			CFileStatus fsStatus, fsStatus2;
			m_File.GetStatus( fsStatus );
			m_File2.GetStatus(fsStatus2);
			m_stBytesInFile2 = (size_t)fsStatus2.m_size;
		}
		else
		{
			m_stBytesInFile2 = 0;
		}

		if( CCommPort::Open( m_sPortName, dwBaudrate, bDataBits, bStopBits, bParity, bFC ) ) 
		{
			COMMTIMEOUTS Timeouts;
			memset( (LPVOID)&Timeouts, 0, sizeof(Timeouts));
	
			Timeouts.ReadTotalTimeoutConstant  = 300;
			Timeouts.WriteTotalTimeoutConstant = 300;
			SetTimeouts( &Timeouts );
			m_pParentWnd ->PostMessage( WM_USER_CHANGE_STATUS, m_nIndex, SC_OPEN_SUCCESS );
			
			m_sStatusMessage = _T("Logging started");

			//jaca
			//zapisz czas startu do pliku .dbg
			time_t rawtime;
			struct tm* timeinfo;
			char buffTmp[80];
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			strftime(buffTmp, sizeof(buffTmp), "\r\nSTART %d-%m-%Y %H:%M:%S\r\n", timeinfo);
			m_File2.Write(buffTmp, strlen(buffTmp));
			m_stBytesInFile2 += strlen(buffTmp);

			if (InitModem()) {
				m_sStatusMessage = "modem initialized (GCI=3D VCID=1)";
			}
			int i = m_sLogFile.ReverseFind('\\');
			m_sLogFileBase = m_sLogFile.Left(i+1); //wyznacz nazwê pliku z jednym po³¹czeniem
			if (m_bIsActive)
			{
				m_pParentWnd->PostMessage(WM_USER_UPDATE_WND, 0, 0);
			}
			//MessageBox(m_pParentWnd->GetSafeHwnd(), m_sLogFileBase, "CRedirectPort", MB_OK);
			return TRUE;
		}
		else
		{
			m_iStatus = SC_OPEN_PORT_ERROR;
			m_sStatusMessage = _T("Unable open port"); 
			m_pParentWnd ->PostMessage( WM_USER_CHANGE_STATUS, m_nIndex, SC_OPEN_PORT_ERROR );

			m_File.Close();
			m_File2.Close();
			return FALSE;		
		}
	}
	else
	{
		m_sStatusMessage = _T("Unable open file"); 
		m_pParentWnd ->PostMessage( WM_USER_CHANGE_STATUS, m_nIndex, SC_OPEN_FILE_ERROR );
		return FALSE;
	}
}

void CRedirectPort::Close()
{	
	m_File.Close();
	m_File2.Close();
	CCommPort::Close();

	m_sStatusMessage = _T("Logging stopped"); 
}

BOOL CRedirectPort::InitModem()
{
	char buffer[4096];
	CString ATString("");
	DWORD dwCount;

	ATString = ("ATZ\r");
	strcpy_s(buffer, 4096, CStringA(ATString).GetString());
	dwCount = ATString.GetLength();
	if (Write((VOID*)buffer, dwCount) == dwCount) {
	}
	else {
		m_sStatusMessage = ATString.Left(ATString.GetLength() - 1) + CString(" error");
		return false;
	}

	ATString = "ATE1\r";
	strcpy_s(buffer, 4096, CStringA(ATString).GetString());
	dwCount = ATString.GetLength();
	if (Write((VOID*)buffer, dwCount) == dwCount) {
	}
	else {
		m_sStatusMessage = ATString.Left(ATString.GetLength() - 1) + CString(" error");
		return false;
	}

	ATString = ("AT+GCI=3D\r"); //set country code to France (3D) or USA (B5) for proper FSK CID recognition
	strcpy_s(buffer, 4096, CStringA(ATString).GetString());
	dwCount = ATString.GetLength();
	if (Write((VOID*)buffer, dwCount) == dwCount) {
	}
	else {
		m_sStatusMessage = ATString.Left(ATString.GetLength() - 1) + CString(" error");
		return false;
	}

	ATString = "AT+VCID=1\r";
	strcpy_s(buffer, 4096, CStringA(ATString).GetString());
	dwCount = ATString.GetLength();
	if (Write((VOID*)buffer, dwCount) == dwCount) {
	}
	else {
		m_sStatusMessage = ATString.Left(ATString.GetLength() - 1) + CString(" error");
		return false;
	}
	return true;
}

BOOL CRedirectPort::SaveSettings()
{
	HKEY hKey = NULL;  
		       
	CString sPreferenceKey( PREFERENCES_KEY );

	sPreferenceKey += _T("\\");
	sPreferenceKey += m_sPortName;

	DWORD dwDisposition;
	if ( RegCreateKeyEx( HKEY_CURRENT_USER, sPreferenceKey, 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
							NULL, &hKey, &dwDisposition ) != ERROR_SUCCESS )
	{
		return FALSE; 
	} 

	// start logging 
	DWORD dwStarted = (DWORD)m_bLogStarted;
	RegSetValueEx( hKey, _T("logging"), 0, REG_DWORD, (BYTE*)&dwStarted, 4 ); 
	
	// redirect file
	RegSetValueEx( hKey, _T("file"), 0, REG_SZ, (BYTE*)(LPCTSTR)m_sLogFile, m_sLogFile.GetLength() * sizeof (TCHAR) ); 
	RegSetValueEx(hKey, _T("file2"), 0, REG_SZ, (BYTE*)(LPCTSTR)m_sLogFile2, m_sLogFile2.GetLength() * sizeof(TCHAR));

	
	// check append 
	RegSetValueEx( hKey, _T("append"), 0, REG_DWORD,(BYTE*)&m_bAppend, 4 );
	// port options
	RegSetValueEx( hKey, _T("baudrate"), 0, REG_DWORD,(BYTE*)&m_dwBaudRate, 4 );
	
	RegSetValueEx( hKey, _T("databits"), 0, REG_DWORD,(BYTE*)&m_iIndexDataBits, 4 );
	
	RegSetValueEx( hKey, _T("parity"), 0, REG_DWORD,(BYTE*)&m_iIndexParity, 4 );
	
	RegSetValueEx( hKey, _T("stopbits"), 0, REG_DWORD,(BYTE*)&m_iIndexStopBits, 4 );
	
	RegSetValueEx( hKey, _T("flowctrl"), 0, REG_DWORD,(BYTE*)&m_iIndexFlowCtrl, 4 );

	return TRUE; 
}

BOOL CRedirectPort::LoadSettings ( )
{
	HKEY hKey = NULL;  
	
	CString sPreferenceKey( PREFERENCES_KEY );
	sPreferenceKey += _T("\\");
	sPreferenceKey += m_sPortName;

	if ( RegOpenKeyEx( HKEY_CURRENT_USER, sPreferenceKey, 0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
	{
		return FALSE;
	}
	
	DWORD cValues;              // number of values for key 
    DWORD cchMaxValue;          // longest value name 
    DWORD cbMaxValueData;       // longest value data 
 	BYTE  bData [1024];
	DWORD dwDataLen = 1024;
 
	DWORD retCode;
	TCHAR achValue[254]; 
	DWORD cchValue = 255; //16383; 
 
    // Get the class name and the value count. 
    retCode = RegQueryInfoKey( hKey, NULL, NULL, NULL, NULL, NULL, NULL, 
								&cValues, &cchMaxValue, &cbMaxValueData, NULL, NULL); 
     //  search values of keys 
    if (cValues) 
    {
        for (DWORD i=0, retCode=ERROR_SUCCESS; i<cValues; i++)
        { 
            cchValue = 255; //16383;
            memset ( bData, 0, 1024 );
			dwDataLen = 1024;
			DWORD dwType = 0;
            retCode = RegEnumValue(hKey, i,	achValue, &cchValue, NULL, &dwType, bData, &dwDataLen );
			
			if (retCode == ERROR_SUCCESS ) 
            { 
			    if ( (dwType == REG_DWORD) && (_tcscmp( _T("logging"), achValue) == 0 ) )
				{
					memcpy ( &m_bLogStarted, bData, 4 ); 
				}

				if ( (dwType == REG_SZ) && (_tcscmp( _T("file"), achValue) == 0) )
				{
					int len = (int)_tcslen( (LPCTSTR)bData );
					LPTSTR lpStrData = m_sLogFile.GetBufferSetLength( len );
					_tcscpy( lpStrData, (LPCTSTR)bData );
				}

				if ((dwType == REG_SZ) && (_tcscmp(_T("file2"), achValue) == 0))
				{
					int len = (int)_tcslen((LPCTSTR)bData);
					LPTSTR lpStrData = m_sLogFile2.GetBufferSetLength(len);
					_tcscpy(lpStrData, (LPCTSTR)bData);
				}

				if ( (dwType == REG_DWORD) && (_tcscmp( _T("append"), achValue) == 0) )
				{
					memcpy ( &m_bAppend, bData, 4 ); 
				}

				if ( (dwType == REG_DWORD) && ( _tcscmp( _T("baudrate"), achValue) == 0) )
				{
					memcpy ( &m_dwBaudRate, bData, 4 ); 
				}

				if ( (dwType == REG_DWORD) && ( _tcscmp( _T("databits"), achValue) == 0) )
				{
					memcpy ( &m_iIndexDataBits, bData, 4 ); 
				}

				if ( (dwType == REG_DWORD) && ( _tcscmp( _T("parity"), achValue) == 0) )
				{
					memcpy ( &m_iIndexParity, bData, 4 ); 
				}

				if ( (dwType == REG_DWORD) && ( _tcscmp( _T("stopbits"), achValue) == 0) )
				{
					memcpy ( &m_iIndexStopBits, bData, 4 ); 
				}

				if ( (dwType == REG_DWORD) && ( _tcscmp( _T("flowctrl"), achValue) == 0) )
				{
					memcpy ( &m_iIndexFlowCtrl, bData, 4 ); 
				}
			}

		}
		//if (m_sLogFile2 == "") m_sLogFile2 = m_sLogFile + ".dbg"; //jacenty gdy brak w rejestrze zapisu o file2, tymczasowo na czas testów, potem usun¹æ
	} 

	return TRUE;
}