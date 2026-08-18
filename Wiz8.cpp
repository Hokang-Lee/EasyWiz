// Wiz8.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "EasyWiz.h"
#include "profile.h"
#include "Wiz8.h"
#include <Winnetwk.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CString mWiz8List;

/////////////////////////////////////////////////////////////////////////////
// CWiz8 プロパティ ページ

IMPLEMENT_DYNCREATE(CWiz8, CPropertyPage)

CWiz8::CWiz8() : CPropertyPage(CWiz8::IDD)
{
	//{{AFX_DATA_INIT(CWiz8)
	m_Computername = _T("");
	m_MailSpoolDir = _T("");
	//}}AFX_DATA_INIT
}

CWiz8::~CWiz8()
{
}

void CWiz8::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWiz8)
	DDX_Text(pDX, IDC_EDIT_SHARE_COMP, m_Computername);
	DDX_Text(pDX, IDC_EDIT_SPOOL, m_MailSpoolDir);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWiz8, CPropertyPage)
	//{{AFX_MSG_MAP(CWiz8)
	ON_BN_CLICKED(IDC_BUTTON_SPOOL, OnButtonSpool)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWiz8 メッセージ ハンドラ

void CWiz8::OnButtonSpool() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
    UpdateData(TRUE);

    LPMALLOC pMalloc; // Shell の標準のアロケータを取得
    if (::SHGetMalloc(&pMalloc) == NOERROR) {
      BROWSEINFO bi;
      char  pszBuffer[MAX_PATH];
      LPITEMIDLIST pidl;
      DWORD cbBuff = 1000;    // Size of Buffer
      TCHAR szBuff[1000];    // Buffer to receive information
      REMOTE_NAME_INFO  * prni = (REMOTE_NAME_INFO *)   &szBuff;
      // BROWSEINFO 構造体を埋める - 必要なところをすべて埋める。
      bi.hwndOwner = GetSafeHwnd();
      bi.pidlRoot = NULL;
      bi.pszDisplayName = pszBuffer;
      bi.lpszTitle = _T(""); //Select a Directory");
      bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
      bi.lpfn = NULL;
      bi.lParam = 0;
      // 次の呼び出しがダイアログ ボックスを表示する。
      if ((pidl = ::SHBrowseForFolder(&bi)) != NULL) {
        if (::SHGetPathFromIDList(pidl, pszBuffer)) { 
          // この時点で、pszBuffer に選択されたパスが入っている
          if(WNetGetUniversalName(pszBuffer,  // ネットワーク資源のパス
                               UNIVERSAL_NAME_INFO_LEVEL,    // 情報のレベル
                              (LPVOID) &szBuff,    //Structure is written to this block of memory
                              &cbBuff) == NO_ERROR)
		    m_MailSpoolDir = prni->lpUniversalName; //pszBuffer;
		  else
			m_MailSpoolDir = pszBuffer;
		  m_MailSpoolDir +=  _T("\\");
	      UpdateData(FALSE);
		}
        pMalloc->Free(pidl); // SHBrowseForFolder によって割り当てられた PIDL を解放
	  }
      pMalloc->Release(); // Shell のアロケータを解放
	}
}

LRESULT CWiz8::OnWizardNext() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
    UpdateData(TRUE);
    CString m, ms;
    m.LoadString(IDS_STRING113);
	CHAR mData[1024];
    sprintf(mData, m, (char *)((const char *)m_MailSpoolDir), (char *)((const char *)m_Computername));
    mWiz8List = (CString)mData;
    UpdateData(FALSE);
	
	return CPropertyPage::OnWizardNext();
}

void CWiz8::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
	// TODO: この位置にメッセージ ハンドラ用のコードを追加してください
	if (bShow) {
      UpdateData(TRUE);
	  CPropertySheet* pSheet = (CPropertySheet*)GetParent();
      CString m, ms;
      m.LoadString( IDS_STRING112 );
	  CHAR mTitle[128];
      sprintf(mTitle, m, "5");
	  pSheet->SetTitle(mTitle, 0);
	  //pSheet->SetTitle("SPA-PRO Mail Server 簡単セットアップウィザード(ステップ　７)", 0);
      UpdateData(FALSE);
	}
}
