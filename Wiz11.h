#if !defined(AFX_WIZ11_H__EAE6503E_7CBB_41CE_BEC9_E0B6BE2A467E__INCLUDED_)
#define AFX_WIZ11_H__EAE6503E_7CBB_41CE_BEC9_E0B6BE2A467E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Wiz11.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CWiz11 ダイアログ

class CWiz11 : public CPropertyPage
{
	DECLARE_DYNCREATE(CWiz11)

// コンストラクション
public:
	void GetLocalGroupList(void);
	CWiz11();
	~CWiz11();

// ダイアログ データ
	//{{AFX_DATA(CWiz11)
	enum { IDD = IDD_EASYWIZ_DIALOG11 };
	CString	m_LocalGroup;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CWiz11)
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
	//{{AFX_MSG(CWiz11)
	virtual BOOL OnInitDialog();
	afx_msg void OnDropdownComboLocalgroup();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_WIZ11_H__EAE6503E_7CBB_41CE_BEC9_E0B6BE2A467E__INCLUDED_)
