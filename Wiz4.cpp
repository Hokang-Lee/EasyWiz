// Wiz4.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "EasyWiz.h"
#include "profile.h"
#include "Wiz4.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CString mWiz4List;

/////////////////////////////////////////////////////////////////////////////
// CWiz4 プロパティ ページ

IMPLEMENT_DYNCREATE(CWiz4, CPropertyPage)

CWiz4::CWiz4() : CPropertyPage(CWiz4::IDD)
{
	//{{AFX_DATA_INIT(CWiz4)
	m_Postmaster = _T("");
	//}}AFX_DATA_INIT
}

CWiz4::~CWiz4()
{
}

void CWiz4::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWiz4)
	DDX_Text(pDX, IDC_EDIT_NAME1, m_Postmaster);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWiz4, CPropertyPage)
	//{{AFX_MSG_MAP(CWiz4)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWiz4 メッセージ ハンドラ

LRESULT CWiz4::OnWizardNext() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
    UpdateData(TRUE);
	m_Postmaster.TrimLeft();
	m_Postmaster.TrimRight();
	int at = m_Postmaster.Find('@');
	if (at <= 0 || at != m_Postmaster.ReverseFind('@') ||
		at >= m_Postmaster.GetLength() - 3 ||
		m_Postmaster.Find('.', at + 2) < 0 || m_Postmaster.Find(' ') >= 0) {
		AfxMessageBox("管理者メールアドレスを正しく入力してください。\n例: postmaster@example.jp", MB_OK | MB_ICONEXCLAMATION);
		GetDlgItem(IDC_EDIT_NAME1)->SetFocus();
		return -1;
	}
    CString m, ms;
    m.LoadString( IDS_STRING111 );
	CHAR mData[1024];
    sprintf(mData, m, (char *)((const char *)m_Postmaster));
    mWiz4List = (CString)mData;
	/*
    if (GetUserDefaultLangID() != (LANGID)0x0411) // 日本語以外
	  mWiz4List = (CString) "Postmaster\n " + m_Postmaster + (CString)"\n\n";
	else
	  mWiz4List = (CString) "管理者とするメールアドレス\n　" + m_Postmaster + (CString)"\n\n";
	*/
    UpdateData(FALSE);

	return CPropertyPage::OnWizardNext();
}

void CWiz4::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
	// TODO: この位置にメッセージ ハンドラ用のコードを追加してください
	if (bShow) {
      UpdateData(TRUE);
	  CPropertySheet* pSheet = (CPropertySheet*)GetParent();
      CString m, ms;
      m.LoadString( IDS_STRING112 );
	  CHAR mTitle[128];
      sprintf(mTitle, m, "4");
	  pSheet->SetTitle(mTitle, 0);
	  //pSheet->SetTitle("SPA-PRO Mail Server 簡単セットアップウィザード(ステップ　７)", 0);
      UpdateData(FALSE);
	}
	
}
