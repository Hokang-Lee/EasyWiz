#if !defined(AFX_WIZ12_H__A8997784_CDA4_4029_A3C0_CEE13F921F2E__INCLUDED_)
#define AFX_WIZ12_H__A8997784_CDA4_4029_A3C0_CEE13F921F2E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Wiz12.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CWiz12 ダイアログ

class CWiz12 : public CPropertyPage
{
	DECLARE_DYNCREATE(CWiz12)

// コンストラクション
public:
	void GetLocalGroupList(void);
	void GetAdUserList(void);
	void PopulateAdDomainList(void);
	CWiz12();
	~CWiz12();

// ダイアログ データ
	//{{AFX_DATA(CWiz12)
	enum { IDD = IDD_EASYWIZ_DIALOG12 };
	CString	m_LocalGroup;
	CString	m_PDC;
	CString	m_ADUser;
	CString	m_ADPassword;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CWiz12)
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CWiz12)
	afx_msg void OnDropdownComboLocalgroup();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnKillfocusEditAdname();
	afx_msg void OnDropdownAdname();
	afx_msg void OnDropdownAduser();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_WIZ12_H__A8997784_CDA4_4029_A3C0_CEE13F921F2E__INCLUDED_)
