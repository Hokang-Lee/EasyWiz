// Wiz1.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "EasyWiz.h"
#include "Wiz1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern int mSel;
extern CString mWiz1List;

/////////////////////////////////////////////////////////////////////////////
// CWiz1 プロパティ ページ

IMPLEMENT_DYNCREATE(CWiz1, CPropertyPage)

CWiz1::CWiz1() : CPropertyPage(CWiz1::IDD)
{
	//{{AFX_DATA_INIT(CWiz1)
	m_Sel = 0;
	//}}AFX_DATA_INIT
}

CWiz1::~CWiz1()
{
}

void CWiz1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWiz1)
	DDX_Radio(pDX, IDC_RADIO_SEL1, m_Sel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWiz1, CPropertyPage)
	//{{AFX_MSG_MAP(CWiz1)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWiz1 メッセージ ハンドラ

LRESULT CWiz1::OnWizardNext() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
    UpdateData(TRUE);
	mSel = m_Sel;

	if (m_Sel == 0) {
	  mWiz1List.LoadString( IDS_STRING102 );
	  /*
      if (GetUserDefaultLangID() != (LANGID)0x0411) // 日本語以外
	    mWiz1List = (CString) "Account\n Multi-domain account.\n\n";
	  else
	    mWiz1List = (CString) "アカウント管理\n　マルチドメイン　アカウント\n\n";
	  */
	  return IDD_EASYWIZ_DIALOG2;
	} else if (m_Sel == 1) {
	  mWiz1List.LoadString( IDS_STRING103 );
	  /*
      if (GetUserDefaultLangID() != (LANGID)0x0411) // 日本語以外
	    mWiz1List = (CString) "Account\n The local account of WINDOWS.\n";
	  else
	    mWiz1List = (CString) "アカウント管理\n　Windows ローカル　アカウント\n";
	  */
	  return IDD_EASYWIZ_DIALOG11;
	}  else if (m_Sel == 2) {
	  mWiz1List.LoadString( IDS_STRING104 );
	  /*
      if (GetUserDefaultLangID() != (LANGID)0x0411) // 日本語以外
	    mWiz1List = (CString) "Account\n The activ directory(AD) account of WINDOWS.\n";
	  else
	    mWiz1List = (CString) "アカウント管理\n　Windows アクティブディレクトリ　アカウント\n";
	  */
	  return IDD_EASYWIZ_DIALOG12;
	}
    UpdateData(FALSE);

   return CPropertyPage::OnWizardNext();
}

BOOL CWiz1::OnSetActive() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	CPropertySheet* pSheet = (CPropertySheet*)GetParent();
	ASSERT_KINDOF(CPropertySheet, pSheet);
	pSheet->SetWizardButtons( PSWIZB_NEXT);
	
	return CPropertyPage::OnSetActive();
}

BOOL CWiz1::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	CString intro;
	intro.Format("%sの設定をナビゲートします。", (LPCTSTR)GetProductDisplayName());
	GetDlgItem(IDC_STATIC_PRODUCT_INTRO)->SetWindowText(intro);
	CString firewallNotice = IsMailServerProduct() ?
		"なお、このウィザードを完了すると、Windows Serverのファイアウォール設定で、SMTP・POP3・IMAPが使用するPortの許可設定が自動的に行われます。ご留意ください。" :
		"なお、このウィザードを完了すると、Windows Serverのファイアウォール設定で、SMTPが使用するPortの許可設定が自動的に行われます。POP3・IMAPの許可設定は行いません。ご留意ください。";
	CWnd *notice = GetDlgItem(IDC_STATIC_FIREWALL_NOTICE);
	notice->SetWindowText(firewallNotice);
	LOGFONT logFont;
	ZeroMemory(&logFont, sizeof(logFont));
	CFont *currentFont = notice->GetFont();
	if (currentFont && currentFont->GetLogFont(&logFont)) {
		logFont.lfWeight = FW_BOLD;
		m_boldNoticeFont.CreateFontIndirect(&logFont);
		notice->SetFont(&m_boldNoticeFont);
	}
	return TRUE;
}

void CWiz1::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
	// TODO: この位置にメッセージ ハンドラ用のコードを追加してください
	if (bShow) {
      UpdateData(TRUE);
	  CPropertySheet* pSheet = (CPropertySheet*)GetParent();
      CString m, ms;
      m = GetWizardTitleFormat();
	  CHAR mTitle[128];
      sprintf(mTitle, m, "1");
	  pSheet->SetTitle(mTitle, 0);
	  //pSheet->SetTitle("SPA-PRO Mail Server 簡単セットアップウィザード(ステップ　７)", 0);
      UpdateData(FALSE);
	}
	
}
