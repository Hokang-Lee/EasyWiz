// Wiz11.cpp : ?C???v???????e?[?V???? ?t?@?C??
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
// CWiz11 ?v???p?e?B ?y?[?W

IMPLEMENT_DYNCREATE(CWiz11, CPropertyPage)

CWiz11::CWiz11() : CPropertyPage(CWiz11::IDD)
{
	//{{AFX_DATA_INIT(CWiz11)
	m_LocalGroup = _T("");
	m_LocalUser = _T("");
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
	DDX_CBString(pDX, IDC_COMBO_LOCALUSER, m_LocalUser);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWiz11, CPropertyPage)
	//{{AFX_MSG_MAP(CWiz11)
	ON_CBN_DROPDOWN(IDC_COMBO_LOCALGROUP, OnDropdownComboLocalgroup)
	ON_CBN_DROPDOWN(IDC_COMBO_LOCALUSER, OnDropdownComboLocaluser)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWiz11 ???b?Z?[?W ?n???h??

LRESULT CWiz11::OnWizardNext() 
{
	// TODO: ?????u???L??????????????A??????{?N???X????яo???????????
    UpdateData(TRUE);
	m_LocalGroup.TrimLeft();
	m_LocalGroup.TrimRight();
	m_LocalUser.TrimLeft();
	m_LocalUser.TrimRight();
	if (m_LocalGroup.IsEmpty()) {
		AfxMessageBox("ユーザーが現在所属するグループを一覧から選択してください。", MB_OK | MB_ICONEXCLAMATION);
		GetDlgItem(IDC_COMBO_LOCALGROUP)->SetFocus();
		return -1;
	}
	if (m_LocalUser.IsEmpty()) {
		AfxMessageBox("テスト送信に使用する既存のWindowsユーザーを選択または入力してください。", MB_OK | MB_ICONEXCLAMATION);
		GetDlgItem(IDC_COMBO_LOCALUSER)->SetFocus();
		return -1;
	}
	for (int nameIndex = 0; nameIndex < m_LocalUser.GetLength(); ++nameIndex) {
		unsigned char c = (unsigned char)m_LocalUser[nameIndex];
		if (!(isalnum(c) || c == '.' || c == '_' || c == '-')) {
			AfxMessageBox("Windowsユーザー名は、メールアドレスに使用できる半角英数字と . _ - で指定してください。", MB_OK | MB_ICONEXCLAMATION);
			GetDlgItem(IDC_COMBO_LOCALUSER)->SetFocus();
			return -1;
		}
	}
	WCHAR wideUser[UNLEN + 1] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_LocalUser, -1, wideUser, UNLEN + 1);
	LPUSER_INFO_1 userInfo = NULL;
	NET_API_STATUS userStatus = NetUserGetInfo(NULL, wideUser, 1, (LPBYTE *)&userInfo);
	if (userStatus != NERR_Success) {
		AfxMessageBox("入力されたWindowsローカルユーザーは存在しません。一覧から既存のユーザーを選択してください。", MB_OK | MB_ICONEXCLAMATION);
		GetDlgItem(IDC_COMBO_LOCALUSER)->SetFocus();
		return -1;
	}
	BOOL disabled = (userInfo->usri1_flags & UF_ACCOUNTDISABLE) != 0;
	NetApiBufferFree(userInfo);
	if (disabled) {
		AfxMessageBox("無効なWindowsユーザーはテスト送信に使用できません。", MB_OK | MB_ICONEXCLAMATION);
		GetDlgItem(IDC_COMBO_LOCALUSER)->SetFocus();
		return -1;
	}
	WCHAR wideGroup[256] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_LocalGroup, -1, wideGroup, 256);
	LPLOCALGROUP_USERS_INFO_0 groups = NULL;
	DWORD groupsRead = 0, groupsTotal = 0;
	BOOL isMember = FALSE;
	NET_API_STATUS groupStatus = NetUserGetLocalGroups(NULL, wideUser, 0,
		0, (LPBYTE *)&groups, MAX_PREFERRED_LENGTH,
		&groupsRead, &groupsTotal);
	if (groupStatus == NERR_Success) {
		for (DWORD groupIndex = 0; groupIndex < groupsRead; ++groupIndex) {
			if (groups[groupIndex].lgrui0_name &&
				_wcsicmp(groups[groupIndex].lgrui0_name, wideGroup) == 0) {
				isMember = TRUE;
				break;
			}
		}
	}
	if (groups) NetApiBufferFree(groups);
	if (!isMember) {
		CString membershipMessage;
		membershipMessage.Format("ユーザー「%s」は選択グループ「%s」に直接所属していません。\n正しい所属グループを選択してください。",
			(LPCTSTR)m_LocalUser, (LPCTSTR)m_LocalGroup);
		AfxMessageBox(membershipMessage, MB_OK | MB_ICONEXCLAMATION);
		GetDlgItem(IDC_COMBO_LOCALUSER)->SetFocus();
		return -1;
	}
    CString m, ms;
    m.LoadString( IDS_STRING105 );
	CHAR mData[1024];
    sprintf(mData, m, (char *)((const char *)m_LocalGroup) );
    mWiz11List = (CString)mData;
	mWiz11List += (CString)"テスト送信ユーザー\n  " + m_LocalUser + (CString)"\n\n";
	mWiz11List += (CString)"※完了時に、選択した既存Windowsユーザーをメールドメインと同名のグループへ追加します。\n現在の所属グループは削除・変更しません。\n\n";

	/*
    if (GetUserDefaultLangID() != (LANGID)0x0411) // ???{???O
      mWiz11List = (CString) " Mail Group " + m_LocalGroup + (CString)"\n\n";
	else
      mWiz11List = (CString) "?@???[???O???[?v?@" + m_LocalGroup + (CString)"\n\n";
	*/
    UpdateData(FALSE);

	return IDD_EASYWIZ_DIALOG2;

	return CPropertyPage::OnWizardNext();
}

LRESULT CWiz11::OnWizardBack() 
{
	// TODO: ?????u???L??????????????A??????{?N???X????яo???????????
	return IDD_EASYWIZ_DIALOG1;

	return CPropertyPage::OnWizardBack();
}

BOOL CWiz11::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: ?????u?????????⑫???????????????????
    UpdateData(TRUE);
#ifdef UPDATE_20050128
	char mLGroup[256];
    GetProfileStringEx(SOFT_REG,"MailGroup", DEFAULT_MAIL_GROUP, mLGroup, sizeof(mLGroup));
	NewLocalGroup("", mLGroup, "Mail server user's group");
#endif
    UpdateData(FALSE);
	GetLocalGroupList();
	GetLocalUserList();
#ifdef UPDATE_20050128
    GetDlgItem(IDC_COMBO_LOCALGROUP)->SendMessage( CB_SELECTSTRING, 0, (LPARAM) mLGroup);
#endif

	return TRUE;  // ?R???g???[????t?H?[?J?X???????????A???l?? TRUE ??????
	              // ??O: OCX ?v???p?e?B ?y?[?W????l?? FALSE ??????
}

void CWiz11::GetLocalGroupList()
{
    // ???[?J???O???[?v????????擾??????B
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
		MessageBox("PDC ?????????????B", "?????", MB_OK);
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
	// TODO: ?????u??R???g???[????m?n???h???p??R?[?h???????????????
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

void CWiz11::GetLocalUserList()
{
	CComboBox *combo = (CComboBox *)GetDlgItem(IDC_COMBO_LOCALUSER);
	combo->ResetContent();
	CString groupName;
	GetDlgItem(IDC_COMBO_LOCALGROUP)->GetWindowText(groupName);
	groupName.TrimLeft();
	groupName.TrimRight();
	if (groupName.IsEmpty()) return;
	WCHAR wideGroup[256] = {0};
	MultiByteToWideChar(CP_ACP, 0, groupName, -1, wideGroup, 256);
	LPLOCALGROUP_MEMBERS_INFO_1 members = NULL;
	DWORD read = 0, total = 0, resume = 0;
	NET_API_STATUS status;
	do {
		status = NetLocalGroupGetMembers(NULL, wideGroup, 1, (LPBYTE *)&members,
			MAX_PREFERRED_LENGTH, &read, &total, &resume);
		if (status == NERR_Success || status == ERROR_MORE_DATA) {
			for (DWORD i = 0; i < read; ++i) {
				if (members[i].lgrmi1_sidusage != SidTypeUser || !members[i].lgrmi1_name)
					continue;
				CHAR account[UNLEN + 1] = {0};
				WideCharToMultiByte(CP_ACP, 0, members[i].lgrmi1_name, -1,
					account, sizeof(account), NULL, NULL);
				CHAR *separator = strrchr(account, '\\');
				CHAR *localName = separator ? separator + 1 : account;
				BOOL safe = localName[0] != 0;
				for (CHAR *p = localName; safe && *p; ++p) {
					unsigned char c = (unsigned char)*p;
					safe = isalnum(c) || c == '.' || c == '_' || c == '-';
				}
				if (safe) combo->AddString(localName);
			}
		}
		if (members) NetApiBufferFree(members);
		members = NULL;
	} while (status == ERROR_MORE_DATA);
}

void CWiz11::OnDropdownComboLocaluser()
{
	UpdateData(TRUE);
	GetLocalUserList();
	GetDlgItem(IDC_COMBO_LOCALUSER)->SendMessage(CB_SELECTSTRING, 0,
		(LPARAM)((const char *)m_LocalUser));
	UpdateData(FALSE);
}

BOOL CWiz11::OnSetActive() 
{
	// TODO: ?????u???L??????????????A??????{?N???X????яo???????????
	CPropertySheet* pSheet = (CPropertySheet*)GetParent();
	ASSERT_KINDOF(CPropertySheet, pSheet);
	pSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT);
	
	return CPropertyPage::OnSetActive();
}

void CWiz11::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
	// TODO: ?????u????b?Z?[?W ?n???h???p??R?[?h???????????????
	if (bShow) {
      UpdateData(TRUE);
	  CPropertySheet* pSheet = (CPropertySheet*)GetParent();
      CString m, ms;
      m = GetWizardTitleFormat();
	  CHAR mTitle[128];
      sprintf(mTitle, m, "1-1");
	  pSheet->SetTitle(mTitle, 0);
	  //pSheet->SetTitle("SPA-PRO Mail Server ??P?Z?b?g?A?b?v?E?B?U?[?h(?X?e?b?v?@?V)", 0);
      UpdateData(FALSE);
	}
	
}


