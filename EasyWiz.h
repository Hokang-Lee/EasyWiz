// EasyWiz.h : EASYWIZ アプリケーションのメイン ヘッダー ファイルです。
//

#if !defined(AFX_EASYWIZ_H__23CA038D_4993_4FF0_B960_FD0957A1B247__INCLUDED_)
#define AFX_EASYWIZ_H__23CA038D_4993_4FF0_B960_FD0957A1B247__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// メイン シンボル


/////////////////////////////////////////////////////////////////////////////
// CEasyWizApp:
// このクラスの動作の定義に関しては EasyWiz.cpp ファイルを参照してください。
//

class CEasyWizApp : public CWinApp
{
public:
	void StartSheet(void);
	CEasyWizApp();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CEasyWizApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// インプリメンテーション

	//{{AFX_MSG(CEasyWizApp)
		// メモ - ClassWizard はこの位置にメンバ関数を追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_EASYWIZ_H__23CA038D_4993_4FF0_B960_FD0957A1B247__INCLUDED_)
