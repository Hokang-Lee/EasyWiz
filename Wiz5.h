#if !defined(AFX_WIZ5_H__9D9C6AF3_9888_4D9A_A8E0_A7F6A035C11D__INCLUDED_)
#define AFX_WIZ5_H__9D9C6AF3_9888_4D9A_A8E0_A7F6A035C11D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Wiz5.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CWiz5 ダイアログ

class CWiz5 : public CPropertyPage
{
	DECLARE_DYNCREATE(CWiz5)

// コンストラクション
public:
	CWiz5();
	~CWiz5();

// ダイアログ データ
	//{{AFX_DATA(CWiz5)
	enum { IDD = IDD_EASYWIZ_DIALOG5 };
	CString	m_Setuplist;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CWiz5)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CWiz5)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_WIZ5_H__9D9C6AF3_9888_4D9A_A8E0_A7F6A035C11D__INCLUDED_)
