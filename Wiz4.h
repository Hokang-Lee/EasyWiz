#if !defined(AFX_WIZ4_H__B03150EE_DAAF_4311_8400_0B7CCFCEE2BA__INCLUDED_)
#define AFX_WIZ4_H__B03150EE_DAAF_4311_8400_0B7CCFCEE2BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Wiz4.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CWiz4 ダイアログ

class CWiz4 : public CPropertyPage
{
	DECLARE_DYNCREATE(CWiz4)

// コンストラクション
public:
	CWiz4();
	~CWiz4();

// ダイアログ データ
	//{{AFX_DATA(CWiz4)
	enum { IDD = IDD_EASYWIZ_DIALOG4 };
	CString	m_Postmaster;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CWiz4)
	public:
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CWiz4)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_WIZ4_H__B03150EE_DAAF_4311_8400_0B7CCFCEE2BA__INCLUDED_)
