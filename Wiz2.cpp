// Wiz2.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "EasyWiz.h"
#include "Wiz2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern int mSel;
extern CString mWiz2List;

/////////////////////////////////////////////////////////////////////////////
// CWiz2 プロパティ ページ

IMPLEMENT_DYNCREATE(CWiz2, CPropertyPage)

CWiz2::CWiz2() : CPropertyPage(CWiz2::IDD)
{
	//{{AFX_DATA_INIT(CWiz2)
	m_DNS1 = _T("");
	m_DNS2 = _T("");
	m_DNS3 = _T("");
	//}}AFX_DATA_INIT
}

CWiz2::~CWiz2()
{
}

void CWiz2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWiz2)
	DDX_Text(pDX, IDC_EDIT_DNS1, m_DNS1);
	DDX_Text(pDX, IDC_EDIT_DNS2, m_DNS2);
	DDX_Text(pDX, IDC_EDIT_DNS3, m_DNS3);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWiz2, CPropertyPage)
	//{{AFX_MSG_MAP(CWiz2)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWiz2 メッセージ ハンドラ

LRESULT CWiz2::OnWizardBack() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
    UpdateData(TRUE);

	if (mSel == 0) {
	  return IDD_EASYWIZ_DIALOG1;
	} else if (mSel == 1) {
	  return IDD_EASYWIZ_DIALOG11;
	} else if (mSel == 2) {
	  return IDD_EASYWIZ_DIALOG12;
	}
    UpdateData(FALSE);

	return CPropertyPage::OnWizardBack();
}

LRESULT CWiz2::OnWizardNext() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
    UpdateData(TRUE);
    CString m, ms;
    m.LoadString( IDS_STRING109 );
	CHAR mData[1024];
    sprintf(mData, m, (char *)((const char *)m_DNS1), (char *)((const char *)m_DNS2), (char *)((const char *)m_DNS3));
    mWiz2List = (CString)mData;
    //sprintf((char *)((const char *)mWiz2List), m, (char *)((const char *)m_DNS1), (char *)((const char *)m_DNS2), (char *)((const char *)m_DNS3));
	/*
    if (GetUserDefaultLangID() != (LANGID)0x0411) { // 日本語以外
      mWiz2List = (CString) "Domain name server (DNS)\n DNS1 " + m_DNS1 + (CString)"\n";
      mWiz2List += (CString) " DNS2 " + m_DNS2 + (CString)"\n";
      mWiz2List += (CString) " DNS3 " + m_DNS3 + (CString)"\n\n";
	} else {
      mWiz2List = (CString) "ドメインネームサーバー\n　ＤＮＳ１　" + m_DNS1 + (CString)"\n";
      mWiz2List += (CString) "　ＤＮＳ２　" + m_DNS2 + (CString)"\n";
      mWiz2List += (CString) "　ＤＮＳ３　" + m_DNS3 + (CString)"\n\n";
	}
	*/
    UpdateData(FALSE);
	
	return CPropertyPage::OnWizardNext();
}

BOOL CWiz2::OnSetActive() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	CPropertySheet* pSheet = (CPropertySheet*)GetParent();
	ASSERT_KINDOF(CPropertySheet, pSheet);
	pSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT);
	
	return CPropertyPage::OnSetActive();
}

void CWiz2::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
	// TODO: この位置にメッセージ ハンドラ用のコードを追加してください
	if (bShow) {
      UpdateData(TRUE);
	  CPropertySheet* pSheet = (CPropertySheet*)GetParent();
      CString m, ms;
      m.LoadString( IDS_STRING112 );
	  CHAR mTitle[128];
      sprintf(mTitle, m, "2");
	  pSheet->SetTitle(mTitle, 0);
	  //pSheet->SetTitle("SPA-PRO Mail Server 簡単セットアップウィザード(ステップ　７)", 0);
      UpdateData(FALSE);
	}
	
}
