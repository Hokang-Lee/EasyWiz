// Wiz3.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "EasyWiz.h"
#include "profile.h"
#include "Wiz3.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern int mSel;
extern CString mWiz3List;

static BOOL IsValidIPv4Address(const CString& value)
{
	int a, b, c, d;
	char tail;
	if (sscanf((LPCTSTR)value, "%d.%d.%d.%d%c", &a, &b, &c, &d, &tail) != 4)
		return FALSE;
	return a >= 0 && a <= 255 && b >= 0 && b <= 255 &&
		c >= 0 && c <= 255 && d >= 0 && d <= 255;
}

static BOOL IsValidDomainName(const CString& value)
{
	if (value.IsEmpty() || value.Find('.') <= 0 || value.Find(' ') >= 0)
		return FALSE;
	return value[0] != '.' && value[value.GetLength() - 1] != '.';
}

/////////////////////////////////////////////////////////////////////////////
// CWiz3 プロパティ ページ

IMPLEMENT_DYNCREATE(CWiz3, CPropertyPage)

CWiz3::CWiz3() : CPropertyPage(CWiz3::IDD)
{
	//{{AFX_DATA_INIT(CWiz3)
	m_Name1 = _T("");
	m_Name2 = _T("");
	m_Name3 = _T("");
	m_IP1 = _T("");
	m_IP2 = _T("");
	m_IP3 = _T("");
	//}}AFX_DATA_INIT
}

CWiz3::~CWiz3()
{
}

void CWiz3::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWiz3)
	DDX_Text(pDX, IDC_EDIT_NAME1, m_Name1);
	DDX_Text(pDX, IDC_EDIT_NAME2, m_Name2);
	DDX_Text(pDX, IDC_EDIT_NAME3, m_Name3);
	DDX_Text(pDX, IDC_EDIT_IP1, m_IP1);
	DDX_Text(pDX, IDC_EDIT_IP2, m_IP2);
	DDX_Text(pDX, IDC_EDIT_IP3, m_IP3);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWiz3, CPropertyPage)
	//{{AFX_MSG_MAP(CWiz3)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWiz3 メッセージ ハンドラ

LRESULT CWiz3::OnWizardNext() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
    UpdateData(TRUE);
	m_Name1.TrimLeft(); m_Name1.TrimRight();
	m_Name2.TrimLeft(); m_Name2.TrimRight();
	m_Name3.TrimLeft(); m_Name3.TrimRight();
	m_IP1.TrimLeft(); m_IP1.TrimRight();
	m_IP2.TrimLeft(); m_IP2.TrimRight();
	m_IP3.TrimLeft(); m_IP3.TrimRight();
	if (!IsValidDomainName(m_Name1)) {
		AfxMessageBox("管理するドメイン名を入力してください。\n例: example.jp", MB_OK | MB_ICONEXCLAMATION);
		GetDlgItem(IDC_EDIT_NAME1)->SetFocus();
		return -1;
	}
	if (mSel == 0 && !IsValidIPv4Address(m_IP1)) {
		AfxMessageBox("ドメイン1の正しい IPv4 アドレスを入力してください。", MB_OK | MB_ICONEXCLAMATION);
		GetDlgItem(IDC_EDIT_IP1)->SetFocus();
		return -1;
	}
	if (!m_Name2.IsEmpty() || !m_IP2.IsEmpty()) {
		if (!IsValidDomainName(m_Name2) || !IsValidIPv4Address(m_IP2)) {
			AfxMessageBox("ドメイン2は、ドメイン名とIPv4アドレスを両方正しく入力してください。", MB_OK | MB_ICONEXCLAMATION);
			GetDlgItem(IDC_EDIT_NAME2)->SetFocus();
			return -1;
		}
	}
	if (!m_Name3.IsEmpty() || !m_IP3.IsEmpty()) {
		if (!IsValidDomainName(m_Name3) || !IsValidIPv4Address(m_IP3)) {
			AfxMessageBox("ドメイン3は、ドメイン名とIPv4アドレスを両方正しく入力してください。", MB_OK | MB_ICONEXCLAMATION);
			GetDlgItem(IDC_EDIT_NAME3)->SetFocus();
			return -1;
		}
	}
    CString m, ms;
    m.LoadString( IDS_STRING110 );
	CHAR mData[1024];
    sprintf(mData, m, (char *)((const char *)m_Name1), (char *)((const char *)m_IP1), (char *)((const char *)m_Name2), (char *)((const char *)m_IP2), (char *)((const char *)m_Name3), (char *)((const char *)m_IP3));
    mWiz3List = (CString)mData;
	/*
    if (GetUserDefaultLangID() != (LANGID)0x0411) { // 日本語以外
	  mWiz3List = (CString) "Domain name to manage\n Domain name1 " + m_Name1 + (CString)" " + m_IP1 + (CString)"\n";
	  mWiz3List += (CString) " Domain name2 " + m_Name2 + (CString)" " + m_IP2 + (CString)"\n";
	  mWiz3List += (CString) " Domain name3 " + m_Name3 + (CString)" " + m_IP3 + (CString)"\n\n";
	} else {
	  mWiz3List = (CString) "管理するドメイン名\n　ドメイン１　" + m_Name1 + (CString)" " + m_IP1 + (CString)"\n";
	  mWiz3List += (CString) "　ドメイン２　" + m_Name2 + (CString)" " + m_IP2 + (CString)"\n";
	  mWiz3List += (CString) "　ドメイン３　" + m_Name3 + (CString)" " + m_IP3 + (CString)"\n\n";
	}
	*/
    UpdateData(FALSE);

	return CPropertyPage::OnWizardNext();
}

BOOL CWiz3::OnSetActive() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
    UpdateData(TRUE);

	if (mSel == 0) {
	  GetDlgItem(IDC_STATIC1)->EnableWindow(TRUE);
	  GetDlgItem(IDC_EDIT_NAME1)->EnableWindow(TRUE);
	  GetDlgItem(IDC_EDIT_IP1)->EnableWindow(TRUE);
	  GetDlgItem(IDC_STATIC2)->EnableWindow(TRUE);
	  GetDlgItem(IDC_EDIT_NAME2)->EnableWindow(TRUE);
	  GetDlgItem(IDC_EDIT_IP2)->EnableWindow(TRUE);
	  GetDlgItem(IDC_STATIC3)->EnableWindow(TRUE);
	  GetDlgItem(IDC_EDIT_NAME3)->EnableWindow(TRUE);
	  GetDlgItem(IDC_EDIT_IP3)->EnableWindow(TRUE);
	} else {
	  GetDlgItem(IDC_EDIT_IP1)->EnableWindow(FALSE);
	  GetDlgItem(IDC_STATIC2)->EnableWindow(FALSE);
	  GetDlgItem(IDC_EDIT_NAME2)->EnableWindow(FALSE);
	  GetDlgItem(IDC_EDIT_IP2)->EnableWindow(FALSE);
	  GetDlgItem(IDC_STATIC3)->EnableWindow(FALSE);
	  GetDlgItem(IDC_EDIT_NAME3)->EnableWindow(FALSE);
	  GetDlgItem(IDC_EDIT_IP3)->EnableWindow(FALSE);
	}

    UpdateData(FALSE);
	
	return CPropertyPage::OnSetActive();
}

void CWiz3::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
	// TODO: この位置にメッセージ ハンドラ用のコードを追加してください
	if (bShow) {
      UpdateData(TRUE);
	  CPropertySheet* pSheet = (CPropertySheet*)GetParent();
      CString m, ms;
      m.LoadString( IDS_STRING112 );
	  CHAR mTitle[128];
      sprintf(mTitle, m, "3");
	  pSheet->SetTitle(mTitle, 0);
	  //pSheet->SetTitle("SPA-PRO Mail Server 簡単セットアップウィザード(ステップ　７)", 0);
      UpdateData(FALSE);
	}
	
}
