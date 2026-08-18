// Wiz5.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "EasyWiz.h"
#include "profile.h"
#include "Wiz1.h"
#include "Wiz11.h"
#include "Wiz12.h"
#include "Wiz2.h"
#include "Wiz3.h"
#include "Wiz4.h"
#include "Wiz5.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern int mSel;
extern CString mWiz1List;
extern CString mWiz11List;
extern CString mWiz12List;
extern CString mWiz2List;
extern CString mWiz3List;
extern CString mWiz4List;
extern CString mWiz5List;
#ifdef REGTOFILE
extern CString mWiz8List;
#endif
#ifdef QSEND
extern CString mWiz7List;
#endif
/////////////////////////////////////////////////////////////////////////////
// CWiz5 プロパティ ページ

IMPLEMENT_DYNCREATE(CWiz5, CPropertyPage)

CWiz5::CWiz5() : CPropertyPage(CWiz5::IDD)
{
	//{{AFX_DATA_INIT(CWiz5)
	m_Setuplist = _T("");
	//}}AFX_DATA_INIT
}

CWiz5::~CWiz5()
{
}

void CWiz5::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWiz5)
	DDX_Text(pDX, IDC_STATIC_SETUPLIST, m_Setuplist);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWiz5, CPropertyPage)
	//{{AFX_MSG_MAP(CWiz5)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWiz5 メッセージ ハンドラ

BOOL CWiz5::OnSetActive() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	CPropertySheet* pSheet = (CPropertySheet*)GetParent();
	ASSERT_KINDOF(CPropertySheet, pSheet);
	// 確認ページでは「次へ」を表示せず、操作を「戻る」か「完了」に絞る。
	pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);

	return CPropertyPage::OnSetActive();
}

BOOL CWiz5::OnWizardFinish()
{
	if (AfxMessageBox("表示されている内容で設定を保存します。\n既存の設定は上書きされます。続行しますか？",
		MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES)
		return FALSE;
	return CPropertyPage::OnWizardFinish();
}

BOOL CWiz5::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: この位置に初期化の補足処理を追加してください
    UpdateData(TRUE);

#ifdef REGTOFILE
	if (mSel == 0) {
#ifdef QSEND
	  m_Setuplist = mWiz1List + mWiz2List + mWiz3List + mWiz4List + mWiz8List + mWiz7List + mWiz5List;
#else
	  m_Setuplist = mWiz1List + mWiz2List + mWiz3List + mWiz4List + mWiz8List + mWiz5List;
#endif
	} else if (mSel == 1) {
#ifdef QSEND
	  m_Setuplist = mWiz1List + mWiz11List + mWiz2List + mWiz3List + mWiz4List + mWiz8List + mWiz7List + mWiz5List;
#else
	  m_Setuplist = mWiz1List + mWiz11List + mWiz2List + mWiz3List + mWiz4List + mWiz8List + mWiz5List;
#endif
	}  else if (mSel == 2) {
#ifdef QSEND
	  m_Setuplist = mWiz1List + mWiz12List + mWiz2List + mWiz3List + mWiz4List + mWiz8List + mWiz7List + mWiz5List;
#else
	  m_Setuplist = mWiz1List + mWiz12List + mWiz2List + mWiz3List + mWiz4List + mWiz8List + mWiz5List;
#endif
	}
#else
	if (mSel == 0) {
	  m_Setuplist = mWiz1List + mWiz2List + mWiz3List + mWiz4List + mWiz5List;
	} else if (mSel == 1) {
	  m_Setuplist = mWiz1List + mWiz11List + mWiz2List + mWiz3List + mWiz4List + mWiz5List;
	}  else if (mSel == 2) {
	  m_Setuplist = mWiz1List + mWiz12List + mWiz2List + mWiz3List + mWiz4List + mWiz5List;
	}
#endif
    UpdateData(FALSE);

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

LRESULT CWiz5::OnWizardBack() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	CPropertySheet* pSheet = (CPropertySheet*)GetParent();
	ASSERT_KINDOF(CPropertySheet, pSheet);
	pSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT);
	
	return CPropertyPage::OnWizardBack();
}

void CWiz5::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
	// TODO: この位置にメッセージ ハンドラ用のコードを追加してください
	if (bShow) {
      UpdateData(TRUE);
	  CPropertySheet* pSheet = (CPropertySheet*)GetParent();
      CString m, ms;
      m.LoadString( IDS_STRING112 );
	  CHAR mTitle[128];
#ifdef LGWAN
#ifdef REGTOFILE
      sprintf(mTitle, m, "8");
#else
      sprintf(mTitle, m, "7");
#endif
#else
#ifdef QSEND
      sprintf(mTitle, m, "7");
#else
#ifdef REGTOFILE
      sprintf(mTitle, m, "6");
#else
      sprintf(mTitle, m, "5");
#endif
#endif
#endif
	  pSheet->SetTitle(mTitle, 0);
	  //pSheet->SetTitle("SPA-PRO Mail Server 簡単セットアップウィザード(ステップ　７)", 0);
      UpdateData(FALSE);
	}

}
