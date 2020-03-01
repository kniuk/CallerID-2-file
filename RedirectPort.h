#pragma once
#include "commport.h"


//jaca start
#include <iostream>
#include <stdlib.h>
#include <string>

#include "atlbase.h"
#include "atlstr.h"
#include "comutil.h"

using namespace std;
//jaca end

#define WM_USER_UPDATE_WND WM_USER + 5587
#define WM_USER_CHANGE_STATUS WM_USER + 5588

const TCHAR PREFERENCES_KEY[] = _T("Software\\Eltima\\Port2File");
// status codes
enum 
{
	SC_NO_ERROR     = 0,
	SC_OPEN_SUCCESS = 1,  
	SC_OPEN_FILE_ERROR = 2,
	SC_OPEN_PORT_ERROR = 3,
	SC_FILE_EXCEPTION  = 10 
};

class CRedirectPort :
	public  CCommPort
{
private:
	CWnd* m_pParentWnd; 
	CFile m_File; 
	CFile m_File2;
	void OnRxChar( DWORD dwCount );
		
	size_t m_stBytesInFile;
	size_t m_stBytesInFile2;
	size_t m_stBytesWritten;
	CString m_sPortName; 
	int m_nIndex;
	BYTE strTel[4096];

public:
	bool m_bIsActive;   
	BOOL m_bLogStarted;
	BOOL m_bAppend; 
	CString m_sLogFile; 
	CString m_sLogFileBase;
	CString m_sLogFile2;
	CString m_sStatusMessage; 
	
	DWORD m_dwBaudRate;
	int  m_iIndexDataBits;
	int  m_iIndexParity;
	int  m_iIndexStopBits;
	int  m_iIndexFlowCtrl;
	int  m_iStatus; 
	
private:
	BOOL InitModem();

public:
	BOOL Open();
	BOOL Open ( DWORD dwBaudrate, BYTE bDataBits, BYTE bParity, BYTE bStopBits, BYTE bFC  );
	void Close(); 

	size_t GetFileTotalBytes()   const { return  m_stBytesInFile;  }
	size_t GetFileTotal2Bytes()   const { return  m_stBytesInFile2; }
	size_t GetFileWrittenBytes() const { return  m_stBytesWritten; }
	
	BOOL SaveSettings ();
	BOOL LoadSettings ();

	CRedirectPort(const CString& sPortName, bool IsActive, CWnd* pWnd, int nIndex );
	virtual ~CRedirectPort(void);
};
