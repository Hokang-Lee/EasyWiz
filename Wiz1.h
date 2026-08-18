#if !defined(AFX_WIZ1_H__95AFDE79_5BAE_4ADC_BB22_AEC177F14AFD__INCLUDED_)
#define AFX_WIZ1_H__95AFDE79_5BAE_4ADC_BB22_AEC177F14AFD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Wiz1.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CWiz1 ダイアログ

class CWiz1 : public CPropertyPage
{
	DECLARE_DYNCREATE(CWiz1)

// コンストラクション
public:
	CWiz1();
	~CWiz1();

// ダイアログ データ
	//{{AFX_DATA(CWiz1)
	enum { IDD = IDD_EASYWIZ_DIALOG1 };
	int		m_Sel;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CWiz1)
	public:
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CWiz1)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_WIZ1_H__95AFDE79_5BAE_4ADC_BB22_AEC177F14AFD__INCLUDED_)
