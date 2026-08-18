// Wiz6.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "EasyWiz.h"
#include "profile.h"
#include "Wiz6.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWiz6 プロパティ ページ

IMPLEMENT_DYNCREATE(CWiz6, CPropertyPage)

CWiz6::CWiz6() : CPropertyPage(CWiz6::IDD)
{
	//{{AFX_DATA_INIT(CWiz6)
	m_EMAIL = _T("");
	//}}AFX_DATA_INIT
}

CWiz6::~CWiz6()
{
}

void CWiz6::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWiz6)
	DDX_Text(pDX, IDC_EDIT_EMAIL, m_EMAIL);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWiz6, CPropertyPage)
	//{{AFX_MSG_MAP(CWiz6)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWiz6 メッセージ ハンドラ

void CWiz6::OnShowWindow(BOOL bShow, UINT nStatus) 
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
