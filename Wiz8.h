#if !defined(AFX_WIZ8_H__B476F176_A797_4DD0_94A7_EAC994561715__INCLUDED_)
#define AFX_WIZ8_H__B476F176_A797_4DD0_94A7_EAC994561715__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Wiz8.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CWiz8 ダイアログ

class CWiz8 : public CPropertyPage
{
	DECLARE_DYNCREATE(CWiz8)

// コンストラクション
public:
	CWiz8();
	~CWiz8();

// ダイアログ データ
	//{{AFX_DATA(CWiz8)
	enum { IDD = IDD_EASYWIZ_DIALOG8 };
	CString	m_Computername;
	CString	m_MailSpoolDir;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CWiz8)
	public:
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CWiz8)
	afx_msg void OnButtonSpool();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_WIZ8_H__B476F176_A797_4DD0_94A7_EAC994561715__INCLUDED_)
