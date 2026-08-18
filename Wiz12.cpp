// Wiz12.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "EasyWiz.h"
#include "profile.h"
#include "Wiz12.h"
#include <lmcons.h>
#include <lmaccess.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include "profile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CString mWiz12List;

/////////////////////////////////////////////////////////////////////////////
// CWiz12 プロパティ ページ

IMPLEMENT_DYNCREATE(CWiz12, CPropertyPage)

CWiz12::CWiz12() : CPropertyPage(CWiz12::IDD)
{
	//{{AFX_DATA_INIT(CWiz12)
	m_LocalGroup = _T("");
	m_PDC = _T("");
	//}}AFX_DATA_INIT
}

CWiz12::~CWiz12()
{
}

void CWiz12::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWiz12)
	DDX_CBString(pDX, IDC_COMBO_LOCALGROUP, m_LocalGroup);
	DDX_Text(pDX, IDC_EDIT_ADNAME, m_PDC);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWiz12, CPropertyPage)
	//{{AFX_MSG_MAP(CWiz12)
	ON_CBN_DROPDOWN(IDC_COMBO_LOCALGROUP, OnDropdownComboLocalgroup)
	ON_WM_SHOWWINDOW()
	ON_EN_KILLFOCUS(IDC_EDIT_ADNAME, OnKillfocusEditAdname)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWiz12 メッセージ ハンドラ

LRESULT CWiz12::OnWizardNext() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
    UpdateData(TRUE);
    CString m, ms;
    m.LoadString( IDS_STRING106 );
	CHAR mData[1024];
    sprintf(mData, m, (char *)((const char *)m_PDC), (char *)((const char *)m_LocalGroup) );
    mWiz12List = (CString)mData;
    /*
    if (GetUserDefaultLangID() != (LANGID)0x0411) { // 日本語以外
      mWiz12List = (CString) " Activi directory(AD) " + m_PDC + (CString)"\n";
      mWiz12List += (CString) " Mail Group " + m_LocalGroup + (CString)"\n\n";
	} else {
      mWiz12List = (CString) "　アクティブディレクトリ　" + m_PDC + (CString)"\n";
      mWiz12List += (CString) "　メールグループ　" + m_LocalGroup + (CString)"\n\n";
	}
	*/
    UpdateData(FALSE);
	return IDD_EASYWIZ_DIALOG2;

	return CPropertyPage::OnWizardNext();
}

LRESULT CWiz12::OnWizardBack() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	return IDD_EASYWIZ_DIALOG1;

	return CPropertyPage::OnWizardBack();
}

void CWiz12::GetLocalGroupList()
{
    // ローカルグループ名の一覧を取得します。
    DWORD entriesread, entries, i; 
	DWORD totalentries; 
	DWORD resumehandle;
    LPLOCALGROUP_INFO_0   LG_Info[1];
    char   localgroup[256];// Mes[256];
	CHAR   mPDC[256];
	NET_API_STATUS nSts;
    wchar_t wDom[65];
	LPBYTE  pbuff;
    wchar_t wszDomain[24];

    GetDlgItem(IDC_COMBO_LOCALGROUP)->SendMessage( CB_RESETCONTENT, 0, 0);
	resumehandle = 0;
	entries = 0;
	sprintf(mPDC, "%s", (LPCSTR)m_PDC);
	if (mPDC[0]) {
      mbstowcs( wDom, m_PDC, 65);
 	  nSts = NetGetAnyDCName(NULL, 
                             wDom,
                             &pbuff);
	  if (nSts == NERR_Success) {
	    wcscpy(wszDomain, (const wchar_t *)pbuff);
        NetApiBufferFree(pbuff);
	  } else {
	    CString m1, m2;
	    m1.LoadString( IDS_STRING108 );
	    m2.LoadString( IDS_STRING107 );
	    MessageBox( m1, m2, MB_OK);
		/*
        if (GetUserDefaultLangID() != (LANGID)0x0411) // 日本語以外
		  MessageBox("PDC not found.", "Simple setup wizard", MB_OK);
        else
		  MessageBox("PDC が見つかりません。", "簡単セットアップウィザード", MB_OK);
		*/
		mPDC[0] = '\x0';
	  }
	}
    if (NetLocalGroupEnum((mPDC[0] ? wszDomain : NULL),
		                  (DWORD) 0,
						  (LPBYTE *)LG_Info,
						  (DWORD) 4096*16,
						  (LPDWORD)&entriesread,  
						  (LPDWORD)&totalentries,
						  NULL
						  ) == NERR_Success) {
        for (i = entries; i < entriesread; i++) {
 	       localgroup[0] = '\x0';
           wcstombs( localgroup, (const wchar_t *)(LG_Info[0]+i)->lgrpi0_name, sizeof(localgroup) );
           GetDlgItem(IDC_COMBO_LOCALGROUP)->SendMessage( CB_ADDSTRING, 0,(LPARAM)localgroup);
		}
      NetApiBufferFree(LG_Info[0]);
	}
}

void CWiz12::OnDropdownComboLocalgroup() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
    UpdateData(TRUE);
#ifdef UPDATE_20050128
	char mLGroup[256];
    GetProfileStringEx(SOFT_REG,"MailGroup", DEFAULT_MAIL_GROUP, mLGroup, sizeof(mLGroup));
	NewLocalGroup((char *)((const char *)m_PDC), mLGroup, "Mail server user's group");
#endif
    GetLocalGroupList();
#ifdef UPDATE_20050128
    GetProfileStringEx(SOFT_REG,"MailGroup", DEFAULT_MAIL_GROUP, mLGroup, sizeof(mLGroup));
    GetDlgItem(IDC_COMBO_LOCALGROUP)->SendMessage( CB_SELECTSTRING, 0, (LPARAM) mLGroup);
#else
    GetDlgItem(IDC_COMBO_LOCALGROUP)->SendMessage( CB_SELECTSTRING, 0, (LPARAM)((const char *)m_LocalGroup));
#endif
    UpdateData(FALSE);
}

BOOL CWiz12::OnSetActive() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	CPropertySheet* pSheet = (CPropertySheet*)GetParent();
	ASSERT_KINDOF(CPropertySheet, pSheet);
	pSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT);
	
	return CPropertyPage::OnSetActive();
}

void CWiz12::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
	// TODO: この位置にメッセージ ハンドラ用のコードを追加してください
	if (bShow) {
      UpdateData(TRUE);
	  CPropertySheet* pSheet = (CPropertySheet*)GetParent();
      CString m, ms;
      m.LoadString( IDS_STRING112 );
	  CHAR mTitle[128];
      sprintf(mTitle, m, "1-2");
	  pSheet->SetTitle(mTitle, 0);
	  //pSheet->SetTitle("SPA-PRO Mail Server 簡単セットアップウィザード(ステップ　７)", 0);
      UpdateData(FALSE);
	}
	
}

void CWiz12::OnKillfocusEditAdname() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
#ifdef UPDATE_20050128
	GetLocalGroupList();
	char mLGroup[256];
    GetProfileStringEx(SOFT_REG,"MailGroup", DEFAULT_MAIL_GROUP, mLGroup, sizeof(mLGroup));
    GetDlgItem(IDC_COMBO_LOCALGROUP)->SendMessage( CB_SELECTSTRING, 0, (LPARAM) mLGroup);
#endif
}
