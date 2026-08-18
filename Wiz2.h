#if !defined(AFX_WIZ2_H__D04A3E80_08F5_4FDB_9A79_743890B9924B__INCLUDED_)
#define AFX_WIZ2_H__D04A3E80_08F5_4FDB_9A79_743890B9924B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Wiz2.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CWiz2 ダイアログ

class CWiz2 : public CPropertyPage
{
	DECLARE_DYNCREATE(CWiz2)

// コンストラクション
public:
	CWiz2();
	~CWiz2();

// ダイアログ データ
	//{{AFX_DATA(CWiz2)
	enum { IDD = IDD_EASYWIZ_DIALOG2 };
	CString	m_DNS1;
	CString	m_DNS2;
	CString	m_DNS3;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CWiz2)
	public:
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CWiz2)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_WIZ2_H__D04A3E80_08F5_4FDB_9A79_743890B9924B__INCLUDED_)
