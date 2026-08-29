#if !defined(AFX_WIZ11_H__EAE6503E_7CBB_41CE_BEC9_E0B6BE2A467E__INCLUDED_)
#define AFX_WIZ11_H__EAE6503E_7CBB_41CE_BEC9_E0B6BE2A467E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Wiz11.h : ?w?b?_?[ ?t?@?C??
//

/////////////////////////////////////////////////////////////////////////////
// CWiz11 ?_?C?A???O

class CWiz11 : public CPropertyPage
{
	DECLARE_DYNCREATE(CWiz11)

// ?R???X?g???N?V????
public:
	void GetLocalGroupList(void);
	void GetLocalUserList(void);
	CWiz11();
	~CWiz11();

// ?_?C?A???O ?f?[?^
	//{{AFX_DATA(CWiz11)
	enum { IDD = IDD_EASYWIZ_DIALOG11 };
	CString	m_LocalGroup;
	CString	m_LocalUser;
	//}}AFX_DATA


// ?I?[?o?[???C?h
	// ClassWizard ????z?????I?[?o?[???C?h??????????B

	//{{AFX_VIRTUAL(CWiz11)
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ?T?|?[?g
	//}}AFX_VIRTUAL

// ?C???v???????e?[?V????
protected:
	// ???????????b?Z?[?W ?}?b?v???
	//{{AFX_MSG(CWiz11)
	virtual BOOL OnInitDialog();
	afx_msg void OnDropdownComboLocalgroup();
	afx_msg void OnDropdownComboLocaluser();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ ??O?s????O?????????}????????B

#endif // !defined(AFX_WIZ11_H__EAE6503E_7CBB_41CE_BEC9_E0B6BE2A467E__INCLUDED_)


