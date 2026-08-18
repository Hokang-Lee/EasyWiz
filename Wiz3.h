#if !defined(AFX_WIZ3_H__B3DB0438_B625_4900_B3F2_AD9C10ABA5D0__INCLUDED_)
#define AFX_WIZ3_H__B3DB0438_B625_4900_B3F2_AD9C10ABA5D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Wiz3.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CWiz3 ダイアログ

class CWiz3 : public CPropertyPage
{
	DECLARE_DYNCREATE(CWiz3)

// コンストラクション
public:
	CWiz3();
	~CWiz3();

// ダイアログ データ
	//{{AFX_DATA(CWiz3)
	enum { IDD = IDD_EASYWIZ_DIALOG3 };
	CString	m_Name1;
	CString	m_Name2;
	CString	m_Name3;
	CString	m_IP1;
	CString	m_IP2;
	CString	m_IP3;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CWiz3)
	public:
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CWiz3)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_WIZ3_H__B3DB0438_B625_4900_B3F2_AD9C10ABA5D0__INCLUDED_)
