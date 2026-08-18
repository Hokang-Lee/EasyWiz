#if !defined(AFX_WIZ7_H__4B0DB07D_3BF4_4DF1_A421_17A945FDF37E__INCLUDED_)
#define AFX_WIZ7_H__4B0DB07D_3BF4_4DF1_A421_17A945FDF37E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Wiz7.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CWiz7 ダイアログ

class CWiz7 : public CPropertyPage
{
	DECLARE_DYNCREATE(CWiz7)

// コンストラクション
public:
	CWiz7();
	~CWiz7();

// ダイアログ データ
	//{{AFX_DATA(CWiz7)
	enum { IDD = IDD_EASYWIZ_DIALOG7 };
	BOOL	m_Gateway;
	CString	m_ARCIP;
	CString	m_GATEIP;
	CString	m_ARCFolder;
	int		m_ARCPort;
	int		m_GATEPort;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CWiz7)
	public:
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CWiz7)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonFolder();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_WIZ7_H__4B0DB07D_3BF4_4DF1_A421_17A945FDF37E__INCLUDED_)
