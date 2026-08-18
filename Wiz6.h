#if !defined(AFX_WIZ6_H__7EBF9E6D_BFFA_47EC_99F9_0DB4DD4856B2__INCLUDED_)
#define AFX_WIZ6_H__7EBF9E6D_BFFA_47EC_99F9_0DB4DD4856B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Wiz6.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CWiz6 ダイアログ

class CWiz6 : public CPropertyPage
{
	DECLARE_DYNCREATE(CWiz6)

// コンストラクション
public:
	CWiz6();
	~CWiz6();

// ダイアログ データ
	//{{AFX_DATA(CWiz6)
	enum { IDD = IDD_EASYWIZ_DIALOG6 };
	CString	m_EMAIL;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CWiz6)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CWiz6)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_WIZ6_H__7EBF9E6D_BFFA_47EC_99F9_0DB4DD4856B2__INCLUDED_)
