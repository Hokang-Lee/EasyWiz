// Wiz11.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "EasyWiz.h"
#include "profile.h"
#include "Wiz11.h"
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

extern CString mWiz11List;
/////////////////////////////////////////////////////////////////////////////
// CWiz11 プロパティ ページ

IMPLEMENT_DYNCREATE(CWiz11, CPropertyPage)

CWiz11::CWiz11() : CPropertyPage(CWiz11::IDD)
{
	//{{AFX_DATA_INIT(CWiz11)
	m_LocalGroup = _T("");
	//}}AFX_DATA_INIT
}

CWiz11::~CWiz11()
{
}

void CWiz11::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWiz11)
	DDX_CBString(pDX, IDC_COMBO_LOCALGROUP, m_LocalGroup);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWiz11, CPropertyPage)
	//{{AFX_MSG_MAP(CWiz11)
	ON_CBN_DROPDOWN(IDC_COMBO_LOCALGROUP, OnDropdownComboLocalgroup)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWiz11 メッセージ ハンドラ

LRESULT CWiz11::OnWizardNext() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
    UpdateData(TRUE);
    CString m, ms;
    m.LoadString( IDS_STRING105 );
	CHAR mData[1024];
    sprintf(mData, m, (char *)((const char *)m_LocalGroup) );
    mWiz11List = (CString)mData;

	/*
    if (GetUserDefaultLangID() != (LANGID)0x0411) // 日本語以外
      mWiz11List = (CString) " Mail Group " + m_LocalGroup + (CString)"\n\n";
	else
      mWiz11List = (CString) "　メールグループ　" + m_LocalGroup + (CString)"\n\n";
	*/
    UpdateData(FALSE);

	return IDD_EASYWIZ_DIALOG2;

	return CPropertyPage::OnWizardNext();
}

LRESULT CWiz11::OnWizardBack() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	return IDD_EASYWIZ_DIALOG1;

	return CPropertyPage::OnWizardBack();
}

BOOL CWiz11::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: この位置に初期化の補足処理を追加してください
    UpdateData(TRUE);
#ifdef UPDATE_20050128
	char mLGroup[256];
    GetProfileStringEx(SOFT_REG,"MailGroup", DEFAULT_MAIL_GROUP, mLGroup, sizeof(mLGroup));
	NewLocalGroup("", mLGroup, "Mail server user's group");
#endif
    UpdateData(FALSE);
	GetLocalGroupList();
#ifdef UPDATE_20050128
    GetDlgItem(IDC_COMBO_LOCALGROUP)->SendMessage( CB_SELECTSTRING, 0, (LPARAM) mLGroup);
#endif

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CWiz11::GetLocalGroupList()
{
    // ローカルグループ名の一覧を取得します。
    DWORD entriesread, entries, i; 
	DWORD totalentries; 
	DWORD resumehandle;
    LPLOCALGROUP_INFO_0   LG_Info[1];
    char   localgroup[256];// Mes[256];
	CHAR   mPDC[256];
	//NET_API_STATUS nSts;
    //wchar_t wDom[65];
	//LPBYTE  pbuff;
    wchar_t wszDomain[24];

    GetDlgItem(IDC_COMBO_LOCALGROUP)->SendMessage( CB_RESETCONTENT, 0, 0);
	resumehandle = 0;
	entries = 0;
	mPDC[0] = '\x0';
/*
	sprintf(mPDC, "%s", m_PDC);
	if (mPDC[0]) {
      mbstowcs( wDom, m_PDC, 65);
 	  nSts = NetGetAnyDCName(NULL, 
                             wDom,
                             &pbuff);
	  if (nSts == NERR_Success) {
	    wcscpy(wszDomain, (const wchar_t *)pbuff);
        NetApiBufferFree(pbuff);
	  } else {
#ifdef _ENGLISH_
		MessageBox("PDC not found.", "manager", MB_OK);
#else
		MessageBox("PDC が見つかりません。", "ﾏﾈｰｼﾞｬｰ", MB_OK);
#endif
		mPDC[0] = '\x0';
	  }
	}
*/
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

void CWiz11::OnDropdownComboLocalgroup() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
    UpdateData(TRUE);
    GetLocalGroupList();
#ifdef UPDATE_20050128
	char mLGroup[256];
    GetProfileStringEx(SOFT_REG,"MailGroup", DEFAULT_MAIL_GROUP, mLGroup, sizeof(mLGroup));
    GetDlgItem(IDC_COMBO_LOCALGROUP)->SendMessage( CB_SELECTSTRING, 0, (LPARAM) mLGroup);
#else
    GetDlgItem(IDC_COMBO_LOCALGROUP)->SendMessage( CB_SELECTSTRING, 0,(LPARAM)((const char *)m_LocalGroup));
#endif
    UpdateData(FALSE);
}

BOOL CWiz11::OnSetActive() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	CPropertySheet* pSheet = (CPropertySheet*)GetParent();
	ASSERT_KINDOF(CPropertySheet, pSheet);
	pSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT);
	
	return CPropertyPage::OnSetActive();
}

void CWiz11::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
	// TODO: この位置にメッセージ ハンドラ用のコードを追加してください
	if (bShow) {
      UpdateData(TRUE);
	  CPropertySheet* pSheet = (CPropertySheet*)GetParent();
      CString m, ms;
      m.LoadString( IDS_STRING112 );
	  CHAR mTitle[128];
      sprintf(mTitle, m, "1-1");
	  pSheet->SetTitle(mTitle, 0);
	  //pSheet->SetTitle("SPA-PRO Mail Server 簡単セットアップウィザード(ステップ　７)", 0);
      UpdateData(FALSE);
	}
	
}
