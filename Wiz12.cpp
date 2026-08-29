ÅEø// Wiz12.cpp : „Ç§„É≥„Éó„É™„É°„É≥„ÉÅEÅE„Ç∑„Éß„É≥ „Éï„Ç°„Ç§„É´
//

#include "stdafx.h"
#include "EasyWiz.h"
#include "profile.h"
#include "Wiz12.h"
#include <lmcons.h>
#include <lmaccess.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <lmjoin.h>
#include "profile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CString mWiz12List;
extern CString g_AdSelectedDnsDomain;

static CString GetJoinedActiveDirectoryName()
{
	LPWSTR joinedName = NULL;
	NETSETUP_JOIN_STATUS joinStatus = NetSetupUnknownStatus;
	CString result;
	if (NetGetJoinInformation(NULL, &joinedName, &joinStatus) == NERR_Success &&
		joinStatus == NetSetupDomainName && joinedName && joinedName[0]) {
		CHAR domainName[256] = {0};
		WideCharToMultiByte(CP_ACP, 0, joinedName, -1,
			domainName, sizeof(domainName), NULL, NULL);
		result = domainName;
	}
	if (joinedName)
		NetApiBufferFree(joinedName);
	return result;
}

static CString GetJoinedDnsDomainName()
{
	CHAR domainName[256] = {0};
	DWORD length = sizeof(domainName);
	if (GetComputerNameEx(ComputerNameDnsDomain, domainName, &length) &&
		domainName[0])
		return CString(domainName);
	return CString("");
}

/////////////////////////////////////////////////////////////////////////////
// CWiz12 „Éó„É≠„Éë„ÉÜ„Ç£ „Éö„ÅE„Ç∏

IMPLEMENT_DYNCREATE(CWiz12, CPropertyPage)

CWiz12::CWiz12() : CPropertyPage(CWiz12::IDD)
{
	//{{AFX_DATA_INIT(CWiz12)
	m_LocalGroup = _T("");
	m_PDC = _T("");
	m_ADUser = _T("");
	m_ADPassword = _T("");
	//}}AFX_DATA_INIT
}

CWiz12::~CWiz12()
{
	if (!m_ADPassword.IsEmpty()) {
		LPTSTR passwordBuffer = m_ADPassword.GetBuffer(m_ADPassword.GetLength());
		SecureZeroMemory(passwordBuffer, m_ADPassword.GetLength());
		m_ADPassword.ReleaseBuffer(0);
	}
}

void CWiz12::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWiz12)
	DDX_CBString(pDX, IDC_COMBO_LOCALGROUP, m_LocalGroup);
	DDX_CBString(pDX, IDC_EDIT_ADNAME, m_PDC);
	DDX_CBString(pDX, IDC_EDIT_ADUSER, m_ADUser);
	DDX_Text(pDX, IDC_EDIT_ADPASSWORD, m_ADPassword);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWiz12, CPropertyPage)
	//{{AFX_MSG_MAP(CWiz12)
	ON_CBN_DROPDOWN(IDC_COMBO_LOCALGROUP, OnDropdownComboLocalgroup)
	ON_WM_SHOWWINDOW()
	ON_CBN_KILLFOCUS(IDC_EDIT_ADNAME, OnKillfocusEditAdname)
	ON_CBN_DROPDOWN(IDC_EDIT_ADNAME, OnDropdownAdname)
	ON_CBN_DROPDOWN(IDC_EDIT_ADUSER, OnDropdownAduser)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWiz12 „É°„ÉÅEÇª„Éº„Ç∏ „Éè„É≥„Éâ„É©

LRESULT CWiz12::OnWizardNext() 
{
	// TODO: „Åì„ÅE‰ΩçÁΩÆ„Å´Âõ∫Êúâ„ÅEÂá¶ÁêÅEÇíËøΩÂä†„Åô„Çã„Åã„ÄÅ„Åæ„Åü„ÅEÂü∫Êú¨„ÇØ„É©„Çπ„ÇíÂëº„Å≥Âá∫„Åó„Å¶„Åè„Å†„Åï„ÅÑ
    UpdateData(TRUE);
	m_PDC.TrimLeft();
	m_PDC.TrimRight();
	if (m_PDC.IsEmpty()) {
		AfxMessageBox("Ç±ÇÃPCÇ©ÇÁActive DirectoryÉhÉÅÉCÉìñºÇéÊìæÇ≈Ç´Ç‹ÇπÇÒÅB\nPCÇ™ADÉhÉÅÉCÉìÇ…éQâ¡ÇµÇƒÇ¢ÇÈÇ©ämîFÇ∑ÇÈÇ©ÅAê≥ÇµÇ¢ADñºÇì¸óÕÇµÇƒÇ≠ÇæÇ≥Ç¢ÅB",
			MB_OK | MB_ICONEXCLAMATION);
		GetDlgItem(IDC_EDIT_ADNAME)->SetFocus();
		return -1;
	}
	if (m_LocalGroup.IsEmpty()) {
		AfxMessageBox("òAågÇ∑ÇÈADÇÃÉÅÅ[ÉãÉOÉãÅ[ÉvÇëIëÇ‹ÇΩÇÕì¸óÕÇµÇƒÇ≠ÇæÇ≥Ç¢ÅB",
			MB_OK | MB_ICONEXCLAMATION);
		GetDlgItem(IDC_COMBO_LOCALGROUP)->SetFocus();
		return -1;
	}
	m_ADUser.TrimLeft();
	m_ADUser.TrimRight();
	if (m_ADUser.IsEmpty()) {
		AfxMessageBox("ÉeÉXÉgëóêMÇ…égópÇ∑ÇÈADÉÜÅ[ÉUÅ[ñºÇì¸óÕÇµÇƒÇ≠ÇæÇ≥Ç¢ÅB",
			MB_OK | MB_ICONEXCLAMATION);
		GetDlgItem(IDC_EDIT_ADUSER)->SetFocus();
		return -1;
	}
	if (m_ADPassword.IsEmpty()) {
		AfxMessageBox("ÉeÉXÉgëóêMÇÃSMTPîFèÿÇ…égópÇ∑ÇÈADÉpÉXÉèÅ[ÉhÇì¸óÕÇµÇƒÇ≠ÇæÇ≥Ç¢ÅB\nÉpÉXÉèÅ[ÉhÇÕï€ë∂ÅEÉçÉOèoóÕÇ≥ÇÍÇ‹ÇπÇÒÅB",
			MB_OK | MB_ICONEXCLAMATION);
		GetDlgItem(IDC_EDIT_ADPASSWORD)->SetFocus();
		return -1;
	}
	// éüÇÃÉhÉÅÉCÉìê›íËâÊñ Ç≈ÇÕÅAADÉÜÅ[ÉUÅ[ÇÃUPNÉTÉtÉBÉbÉNÉXÇ
	// ä«óùëŒè€ÉÅÅ[ÉãÉhÉÅÉCÉìÇÃèâä˙ílÇ∆ÇµÇƒégópÇ∑ÇÈÅB
	g_AdSelectedDnsDomain.Empty();
	int upnAt = m_ADUser.Find('@');
	if (upnAt > 0 && upnAt < m_ADUser.GetLength() - 1)
		g_AdSelectedDnsDomain = m_ADUser.Mid(upnAt + 1);
	if (g_AdSelectedDnsDomain.IsEmpty() && m_PDC.Find('.') > 0)
		g_AdSelectedDnsDomain = m_PDC;
	if (g_AdSelectedDnsDomain.IsEmpty())
		g_AdSelectedDnsDomain = GetJoinedDnsDomainName();
    CString m, ms;
    m.LoadString( IDS_STRING106 );
	CHAR mData[1024];
    sprintf(mData, m, (char *)((const char *)m_PDC), (char *)((const char *)m_LocalGroup) );
    mWiz12List = (CString)mData;
    /*
    if (GetUserDefaultLangID() != (LANGID)0x0411) { // Êó•Êú¨Ë™û‰ª•Â§ÅE
      mWiz12List = (CString) " Activi directory(AD) " + m_PDC + (CString)"\n";
      mWiz12List += (CString) " Mail Group " + m_LocalGroup + (CString)"\n\n";
	} else {
      mWiz12List = (CString) "„ÄÄ„Ç¢„ÇØ„ÉÅEÇ£„Éñ„Éá„Ç£„É¨„ÇØ„Éà„É™„ÄÄ" + m_PDC + (CString)"\n";
      mWiz12List += (CString) "„ÄÄ„É°„Éº„É´„Ç∞„É´„Éº„Éó„ÄÄ" + m_LocalGroup + (CString)"\n\n";
	}
	*/
    UpdateData(FALSE);
	return IDD_EASYWIZ_DIALOG2;

	return CPropertyPage::OnWizardNext();
}

LRESULT CWiz12::OnWizardBack() 
{
	// TODO: „Åì„ÅE‰ΩçÁΩÆ„Å´Âõ∫Êúâ„ÅEÂá¶ÁêÅEÇíËøΩÂä†„Åô„Çã„Åã„ÄÅ„Åæ„Åü„ÅEÂü∫Êú¨„ÇØ„É©„Çπ„ÇíÂëº„Å≥Âá∫„Åó„Å¶„Åè„Å†„Åï„ÅÑ
	return IDD_EASYWIZ_DIALOG1;

	return CPropertyPage::OnWizardBack();
}

void CWiz12::GetLocalGroupList()
{
    // „É≠„Éº„Ç´„É´„Ç∞„É´„Éº„ÉóÂêç„ÅÆ‰∏ÄË¶ß„ÇíÂèñÂæó„Åó„Åæ„Åô„ÄÅE
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
        if (GetUserDefaultLangID() != (LANGID)0x0411) // Êó•Êú¨Ë™û‰ª•Â§ÅE
		  MessageBox("PDC not found.", "Simple setup wizard", MB_OK);
        else
		  MessageBox("PDC „ÅåË¶ã„Å§„Åã„Çä„Åæ„Åõ„Çì„ÄÅE, "Á∞°Âçò„Çª„ÉÅEÉà„Ç¢„ÉÅEÅE„Ç¶„Ç£„Ç∂„Éº„ÉÅE, MB_OK);
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

void CWiz12::PopulateAdDomainList()
{
	CComboBox *combo = (CComboBox *)GetDlgItem(IDC_EDIT_ADNAME);
	if (!combo) return;
	CString current = m_PDC;
	combo->ResetContent();
	CString dnsDomain = GetJoinedDnsDomainName();
	CString joinedDomain = GetJoinedActiveDirectoryName();
	if (!dnsDomain.IsEmpty()) combo->AddString(dnsDomain);
	if (!joinedDomain.IsEmpty() && joinedDomain.CompareNoCase(dnsDomain) != 0)
		combo->AddString(joinedDomain);
	if (!current.IsEmpty() && current.CompareNoCase(dnsDomain) != 0 &&
		current.CompareNoCase(joinedDomain) != 0)
		combo->AddString(current);
}

void CWiz12::GetAdUserList()
{
	CComboBox *combo = (CComboBox *)GetDlgItem(IDC_EDIT_ADUSER);
	if (!combo) return;
	CString current = m_ADUser;
	combo->ResetContent();
	if (!current.IsEmpty()) combo->AddString(current);
	if (m_PDC.IsEmpty() || m_LocalGroup.IsEmpty()) return;

	wchar_t domain[256] = {0};
	wchar_t group[256] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_PDC, -1, domain, 256);
	MultiByteToWideChar(CP_ACP, 0, m_LocalGroup, -1, group, 256);
	LPBYTE dcBuffer = NULL;
	if (NetGetAnyDCName(NULL, domain, &dcBuffer) != NERR_Success)
		return;

	DWORD_PTR resume = 0;
	NET_API_STATUS status;
	do {
		LPLOCALGROUP_MEMBERS_INFO_3 members = NULL;
		DWORD read = 0, total = 0;
		status = NetLocalGroupGetMembers((LPCWSTR)dcBuffer, group, 3,
			(LPBYTE *)&members, MAX_PREFERRED_LENGTH, &read, &total, &resume);
		if (status == NERR_Success || status == ERROR_MORE_DATA) {
			for (DWORD i = 0; i < read; ++i) {
				CHAR qualified[512] = {0};
				WideCharToMultiByte(CP_ACP, 0, members[i].lgrmi3_domainandname,
					-1, qualified, sizeof(qualified), NULL, NULL);
				LPSTR name = strrchr(qualified, '\\');
				name = name ? name + 1 : qualified;
				if (name[0] && combo->FindStringExact(-1, name) == CB_ERR)
					combo->AddString(name);
			}
		}
		if (members) NetApiBufferFree(members);
	} while (status == ERROR_MORE_DATA);
	NetApiBufferFree(dcBuffer);
}

void CWiz12::OnDropdownComboLocalgroup() 
{
	// TODO: „Åì„ÅE‰ΩçÁΩÆ„Å´„Ç≥„É≥„Éà„É≠„Éº„É´ÈÄöÁü•„Éè„É≥„Éâ„É©Áî®„ÅÆ„Ç≥„Éº„Éâ„ÇíËøΩÂä†„Åó„Å¶„Åè„Å†„Åï„ÅÑ
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
	// TODO: „Åì„ÅE‰ΩçÁΩÆ„Å´Âõ∫Êúâ„ÅEÂá¶ÁêÅEÇíËøΩÂä†„Åô„Çã„Åã„ÄÅ„Åæ„Åü„ÅEÂü∫Êú¨„ÇØ„É©„Çπ„ÇíÂëº„Å≥Âá∫„Åó„Å¶„Åè„Å†„Åï„ÅÑ
	CPropertySheet* pSheet = (CPropertySheet*)GetParent();
	ASSERT_KINDOF(CPropertySheet, pSheet);
	if (m_PDC.IsEmpty()) {
		m_PDC = GetJoinedDnsDomainName();
		if (m_PDC.IsEmpty()) m_PDC = GetJoinedActiveDirectoryName();
	}
	PopulateAdDomainList();
	if (m_ADUser.IsEmpty()) {
		CHAR currentUser[UNLEN + 1] = {0};
		DWORD currentUserLength = sizeof(currentUser);
		if (GetUserName(currentUser, &currentUserLength) && currentUser[0]) {
			// ADÇÃDNSÉhÉÅÉCÉìñºÇégÇ¡ÇΩUPNå`éÆÇä˘íËÇ…Ç∑ÇÈÅB
			// DNSÉhÉÅÉCÉìñºÇÅuDOMAIN\\userÅvÇ…ó¨ópÇ∑ÇÈÇ∆ÅANetBIOSñºÇ∆
			// àÍívÇµÇ»Ç¢ä¬ã´Ç≈ÇÕîFèÿÇ…é∏îsÇ∑ÇÈÇΩÇﬂégópÇµÇ»Ç¢ÅB
			if (!m_PDC.IsEmpty())
				m_ADUser.Format("%s@%s", currentUser, m_PDC);
			else
				m_ADUser = currentUser;
		}
	}
	UpdateData(FALSE);
	if (!m_PDC.IsEmpty())
		GetLocalGroupList();
	GetAdUserList();
	pSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT);
	
	return CPropertyPage::OnSetActive();
}

void CWiz12::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
	// TODO: „Åì„ÅE‰ΩçÁΩÆ„Å´„É°„ÉÅEÇª„Éº„Ç∏ „Éè„É≥„Éâ„É©Áî®„ÅÆ„Ç≥„Éº„Éâ„ÇíËøΩÂä†„Åó„Å¶„Åè„Å†„Åï„ÅÑ
	if (bShow) {
      UpdateData(TRUE);
	  CPropertySheet* pSheet = (CPropertySheet*)GetParent();
      CString m, ms;
      m = GetWizardTitleFormat();
	  CHAR mTitle[128];
      sprintf(mTitle, m, "1-2");
	  pSheet->SetTitle(mTitle, 0);
	  //pSheet->SetTitle("SPA-PRO Mail Server Á∞°Âçò„Çª„ÉÅEÉà„Ç¢„ÉÅEÅE„Ç¶„Ç£„Ç∂„Éº„ÉÅE„Çπ„ÉÅEÉÉ„Éó„ÄÄÅEÅE", 0);
      UpdateData(FALSE);
	}
	
}

void CWiz12::OnKillfocusEditAdname() 
{
	// TODO: „Åì„ÅE‰ΩçÁΩÆ„Å´„Ç≥„É≥„Éà„É≠„Éº„É´ÈÄöÁü•„Éè„É≥„Éâ„É©Áî®„ÅÆ„Ç≥„Éº„Éâ„ÇíËøΩÂä†„Åó„Å¶„Åè„Å†„Åï„ÅÑ
#ifdef UPDATE_20050128
	GetLocalGroupList();
	char mLGroup[256];
    GetProfileStringEx(SOFT_REG,"MailGroup", DEFAULT_MAIL_GROUP, mLGroup, sizeof(mLGroup));
    GetDlgItem(IDC_COMBO_LOCALGROUP)->SendMessage( CB_SELECTSTRING, 0, (LPARAM) mLGroup);
#endif
}

void CWiz12::OnDropdownAdname()
{
	UpdateData(TRUE);
	PopulateAdDomainList();
	CComboBox *combo = (CComboBox *)GetDlgItem(IDC_EDIT_ADNAME);
	if (combo) combo->SelectString(-1, m_PDC);
}

void CWiz12::OnDropdownAduser()
{
	UpdateData(TRUE);
	GetAdUserList();
	CComboBox *combo = (CComboBox *)GetDlgItem(IDC_EDIT_ADUSER);
	if (combo) combo->SelectString(-1, m_ADUser);
}

