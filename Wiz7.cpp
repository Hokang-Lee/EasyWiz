// Wiz7.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "EasyWiz.h"
#include "profile.h"
#include "Wiz7.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CString mWiz7List;

/////////////////////////////////////////////////////////////////////////////
// CWiz7 プロパティ ページ

IMPLEMENT_DYNCREATE(CWiz7, CPropertyPage)

CWiz7::CWiz7() : CPropertyPage(CWiz7::IDD)
{
	//{{AFX_DATA_INIT(CWiz7)
	m_Gateway = FALSE;
	m_ARCIP = _T("");
	m_GATEIP = _T("");
	m_ARCFolder = _T("");
	m_ARCPort = 10025;
	m_GATEPort = 25;
	//}}AFX_DATA_INIT
}

CWiz7::~CWiz7()
{
}

void CWiz7::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWiz7)
	DDX_Check(pDX, IDC_CHECK_GATEWAY, m_Gateway);
	DDX_Text(pDX, IDC_EDIT_ARCIP, m_ARCIP);
	DDX_Text(pDX, IDC_EDIT_GATEIP, m_GATEIP);
	DDX_Text(pDX, IDC_EDIT_FOLDER, m_ARCFolder);
	DDX_Text(pDX, IDC_EDIT_ARCPORT, m_ARCPort);
	DDX_Text(pDX, IDC_EDIT_GATEPORT, m_GATEPort);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWiz7, CPropertyPage)
	//{{AFX_MSG_MAP(CWiz7)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BUTTON_FOLDER, OnButtonFolder)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWiz7 メッセージ ハンドラ

void CWiz7::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
	// TODO: この位置にメッセージ ハンドラ用のコードを追加してください
	if (bShow) {
      UpdateData(TRUE);
	  CPropertySheet* pSheet = (CPropertySheet*)GetParent();
      CString m, ms;
      m.LoadString( IDS_STRING112 );
	  CHAR mTitle[128];
      sprintf(mTitle, m, "6");
	  pSheet->SetTitle(mTitle, 0);
	  //pSheet->SetTitle("SPA-PRO Mail Server 簡単セットアップウィザード(ステップ　７)", 0);
      UpdateData(FALSE);
	}
	
}

BOOL CWiz7::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: この位置に初期化の補足処理を追加してください
	
	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

LRESULT CWiz7::OnWizardNext() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
    UpdateData(TRUE);
    CString m, ms;
    m.LoadString( IDS_STRING116 );
	CHAR mData[1024];
    sprintf(mData, m, (m_Gateway ? "enable" : "disable"), m_GATEIP, m_GATEPort, m_ARCIP, m_ARCPort, m_ARCFolder);
    mWiz7List = (CString)mData;
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
	
	return CPropertyPage::OnWizardNext();
}

void CWiz7::OnButtonFolder() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
    UpdateData(TRUE);

    LPMALLOC pMalloc; // Shell の標準のアロケータを取得
    if (::SHGetMalloc(&pMalloc) == NOERROR) {
      BROWSEINFO bi;
      char pszBuffer[MAX_PATH];
      LPITEMIDLIST pidl;
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
          // この時点で、pszBuffer に選択されたパスが入っている */.
		  m_ARCFolder = pszBuffer;
		  m_ARCFolder +=  _T("\\");
	      UpdateData(FALSE);
		}
        pMalloc->Free(pidl); // SHBrowseForFolder によって割り当てられた PIDL を解放
	  }
      pMalloc->Release(); // Shell のアロケータを解放
	}
	
}
