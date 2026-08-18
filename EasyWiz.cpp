// EasyWiz.cpp : アプリケーション用クラスの定義を行います。
//

#include "stdafx.h"
#include "EasyWiz.h"
#include "profile.h"
#include "Wiz1.h"
#include "Wiz11.h"
#include "Wiz12.h"
#include "Wiz2.h"
#include "Wiz3.h"
#include "Wiz4.h"
#include "Wiz5.h"
#ifdef LGWAN
#include "Wiz6.h"
#include "Wiz7.h"
#endif
#ifdef QSEND
#include "Wiz7.h"
#endif
#ifdef REGTOFILE
#include "Wiz8.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int mSel;
CString mWiz1List;
CString mWiz11List;
CString mWiz12List;
CString mWiz2List;
CString mWiz3List;
CString mWiz4List;
CString mWiz5List;
CString mWiz6List;
CString mWiz7List;
CString mWiz8List;

#ifdef REGTOFILE
BOOL    nClustering;
DWORD   nProductcode;
char    mMailSpoolDir[128];
#endif

void GetReg(FILE *fp, LPCTSTR lpAppName);
void RestoreFile(char *pfn);
int UserRight(char *Account, char *Machine, BOOL bAction);
#ifdef UPDATE_20050128
NET_API_STATUS AddLocalGroupAccount(CHAR *lpszContry, CHAR *lpszDomain, CHAR *lpszUser, CHAR *lpszLocalGroup );
#endif
/////////////////////////////////////////////////////////////////////////////
// CEasyWizApp

BEGIN_MESSAGE_MAP(CEasyWizApp, CWinApp)
	//{{AFX_MSG_MAP(CEasyWizApp)
		// メモ - ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEasyWizApp クラスの構築

CEasyWizApp::CEasyWizApp()
{
	// TODO: この位置に構築用のコードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
}

/////////////////////////////////////////////////////////////////////////////
// 唯一の CEasyWizApp オブジェクト

CEasyWizApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CEasyWizApp クラスの初期化

BOOL CEasyWizApp::InitInstance()
{
	AfxEnableControlContainer();

	// 標準的な初期化処理
	// もしこれらの機能を使用せず、実行ファイルのサイズを小さくしたけ
	//  れば以下の特定の初期化ルーチンの中から不必要なものを削除して
	//  ください。

#ifdef _AFXDLL
	Enable3dControls();			// 共有 DLL 内で MFC を使う場合はここをコールしてください。
#else
	Enable3dControlsStatic();	// MFC と静的にリンクする場合はここをコールしてください。
#endif

    StartSheet();
	// ダイアログが閉じられてからアプリケーションのメッセージ ポンプを開始するよりは、
	// アプリケーションを終了するために FALSE を返してください。
	return FALSE;
}

void CEasyWizApp::StartSheet()
{
   CWiz1     Wiz1;
   CWiz11    Wiz11;
   CWiz12    Wiz12;
   CWiz2     Wiz2;
   CWiz3     Wiz3;
   CWiz4     Wiz4;
   CWiz5     Wiz5;
#ifdef LGWAN
   CWiz6     Wiz6;
   CWiz7     Wiz7;
#endif
#ifdef QSEND
   CWiz7     Wiz7;
#endif
#ifdef REGTOFILE
   CWiz8     Wiz8;
   BOOL      bUNC;
#endif
   CHAR      mReg[_MAX_PATH];
   CHAR      mMailbox[256], mSpool[256], mDomain[1024];
   char      *p, mPath[256], mFn[256], mCmpName[256], mMMLISTFn[256];

#ifdef E_POST
   CPropertySheet cPropSheet("E-Post Mail Server 簡単設定ウィザード");
#else
   CPropertySheet cPropSheet("SPA-PRO Mail Server 簡単設定ウィザード");
#endif

  cPropSheet.AddPage(&Wiz1);
  cPropSheet.AddPage(&Wiz11);
  cPropSheet.AddPage(&Wiz12);
  cPropSheet.AddPage(&Wiz2);
  cPropSheet.AddPage(&Wiz3);
  cPropSheet.AddPage(&Wiz4);
#ifdef REGTOFILE
  cPropSheet.AddPage(&Wiz8);
#endif
#ifdef LGWAN                 ////// LGWAN環境テンプレート
  cPropSheet.AddPage(&Wiz6);
  cPropSheet.AddPage(&Wiz7);
#endif
#ifdef QSEND
  cPropSheet.AddPage(&Wiz7);
#endif
  cPropSheet.AddPage(&Wiz5);

  cPropSheet.SetWizardMode();

  strcpy(mPath, __argv[0]);
  //////////////////////////////////////////////////////
  Wiz1.m_Sel = 0;
  Wiz11.m_LocalGroup = (CString)"";
  Wiz12.m_PDC = (CString)"";
  Wiz12.m_LocalGroup = (CString)"";
  Wiz2.m_DNS1 = (CString)"";
  Wiz2.m_DNS2 = (CString)""; 
  Wiz2.m_DNS3 = (CString)"";
  Wiz3.m_Name1 = (CString)"";
  Wiz3.m_Name2 = (CString)"";
  Wiz3.m_Name3 = (CString)"";
  Wiz3.m_IP1 = (CString)"";
  Wiz3.m_IP2 = (CString)"";
  Wiz3.m_IP3 = (CString)"";
  Wiz4.m_Postmaster = (CString)"";
  //////////////////////////////////////////////////////
#ifdef E_POST
  FILE *fp;
  if ((p = strrchr(mPath, '\\'))) {
    *p = '\x0';
    sprintf(mFn, "%s\\epstms.chg", mPath);
	sprintf(mMMLISTFn, "%s\\mmlist.dat", mPath);
  } else {
    strcpy(mFn, "epstms.chg");
	sprintf(mMMLISTFn, "mmlist.dat", mPath);
  }
  
  if ((fp = fopen(mFn, "wt"))) {
    fprintf(fp, "REGEDIT4\n");
	GetReg(fp, "SOFTWARE\\EMWAC");
    GetReg(fp, "SOFTWARE\\SPA-PRO"); // 旧バージョンの設定データをバックアップ
    GetReg(fp, "SYSTEM\\CurrentControlSet\\Services\\SPARS-PRO"); // 旧バージョンの設定データをバックアップ
    GetReg(fp, "SYSTEM\\CurrentControlSet\\Services\\SPADS-PRO"); // 旧バージョンの設定データをバックアップ
    GetReg(fp, "SYSTEM\\CurrentControlSet\\Services\\SPAPOP3S-PRO"); // 旧バージョンの設定データをバックアップ
    GetReg(fp, "SYSTEM\\CurrentControlSet\\Services\\SPAIMAP4S-PRO"); // 旧バージョンの設定データをバックアップ
    fclose(fp);
  }
#endif
  //////////////////////////////////////////////////////
#ifdef REGTOFILE
   ///// スプール先はレジストリから取得
   sprintf(mSpool, "%c:\\mail", (char)(_getdrive() + 'A' - 1 ));
   GetProfileStringEx(SOFT_REG, "MailSpoolDir", "", mMailSpoolDir, sizeof(mMailSpoolDir)); // メールボックスフォルダ
   if (!mMailSpoolDir[0])
	 strcpy(mMailSpoolDir, mSpool);
   Wiz8.m_MailSpoolDir = (CString)mMailSpoolDir;
   Wiz8.m_Computername = (CString)"";
   ///// 製品コード取得
   nProductcode = GetProfileIntEx(SOFT_REG, "Productcode", (int)0); // 0:Mail Server, 1:SMTP Server
   ///// クラスタ対応モードはレジストリから取得
   nClustering = GetProfileIntEx(SOFT_REG, "Clustering", (int)0);
#endif
  //////////////////////////////////////////////////////
  if (cPropSheet.DoModal() == ID_WIZFINISH) {
     //// Create Regstry Key ////
 	 ///// スプール先はレジストリへ
#ifdef REGTOFILE
	 sprintf(mSpool, (char *)((const char*)Wiz8.m_MailSpoolDir));
     ///////////////////////
	 // フォルダをSakusei
	 DWORD     a;
	 CHAR      mTemp[256], *tmp;
	 bUNC = FALSE;
#ifdef UPDATE_20060827 // メールスプールフォルダドライブがメールボックスフォルダ・アカウントＤＢフォルダのドライブに反映されない
	 char      cDrv;
	 if (mSpool[1] == ':')
       cDrv = mSpool[0];
	 else
       cDrv = (char)(_getdrive() + 'A' - 1 );
#endif
	 strcpy(mTemp, mSpool); 
	 tmp = strstr(mTemp,":\\");
	 if (tmp)
	   tmp = strstr(tmp+2,"\\");
     else if ((tmp = strstr(mTemp,"\\\\"))) {
	   bUNC = TRUE;  // UNC接続
       if ((tmp = strstr(tmp+2,"\\")))
         tmp = strstr(tmp+1,"\\");
	 }
     while(tmp) {
       *tmp = '\x0';
       if (_mkdir(mTemp) == -1) {         // 処理用フォルダ作成
		 a = errno;
		 if (a != 17 && a != 13) { // 既に存在する以外
           CString m;
		   CHAR    mMess[256];
           m.LoadString(IDS_STRING115);
           sprintf(mMess, (LPCSTR)m, mSpool);
		   MessageBox(NULL, mMess, "EasyWiz", MB_ICONSTOP | MB_OK);
		   _exit(-1);
		 }
	   }
       *tmp = '\\';
       tmp = strstr(tmp+1,"\\");
	 }
     _mkdir(mTemp);         // 処理用フォルダ作成
     ////////////////////////////////////////
   	 DWORD n1 = nClustering;
	 nClustering = 0;
	 WriteProfileStringEx(SOFT_REG,"MailSpoolDir", mSpool); // レジストリにメールボックスフォルダ
	 nClustering = n1;
	 WriteProfileStringEx(SOFT_REG,"MailSpoolDir", mSpool); // 共有フォルダ上のレジストリ情報にもメールボックスフォルダ
#else
	 sprintf(mSpool, "%c:\\mail", (char)(_getdrive() + 'A' - 1 ));
	 WriteProfileStringEx(SOFT_REG,"MailSpoolDir", mSpool); // メールボックスフォルダ
#endif
#ifdef REGTOFILE
	 ///// スプール先
	 strcpy(mMailSpoolDir, mSpool);
	 //// 共有コンピュータリスト
	 FILE *fp;
	 strcpy(mCmpName, (char *)((const char *)Wiz8.m_Computername));
	 if (mCmpName[0] == '\\' && mCmpName[1] == '\\')
	   p = &mCmpName[2];
	 else if (mCmpName[0] == '\\' && mCmpName[1] != '\\')
	   p = &mCmpName[1];
	 else
	   p = mCmpName;
     if ((fp = fopen(mMMLISTFn, "wt"))) {
	   fputs(p, fp);
	   fclose(fp);
	 }
#endif
     HKEY   hKey;
#ifdef REGTOFILE
   if (nClustering && !strnicmp(SOFT_REG, "software\\emwac", 14)) {
     FileCreateKey(mMailSpoolDir, SOFT_REG);
   } else {
#endif
     RegCreateKey( (HKEY) HKEY_LOCAL_MACHINE, (LPCTSTR) SOFT_REG, &hKey);
     RegCloseKey((HKEY)hKey);
#ifdef REGTOFILE
   }
#endif
     sprintf(mReg, "%s\\Aliases", SOFT_REG);
#ifdef REGTOFILE
   if (nClustering && !strnicmp(mReg, "software\\emwac", 14)) {
     FileCreateKey(mMailSpoolDir, mReg);
   } else {
#endif
     RegCreateKey( (HKEY) HKEY_LOCAL_MACHINE, (LPCTSTR) mReg, &hKey);
     RegCloseKey((HKEY)hKey);
#ifdef REGTOFILE
   }
#endif
     sprintf(mReg, "%s\\Lists", SOFT_REG);
#ifdef REGTOFILE
   if (nClustering && !strnicmp(mReg, "software\\emwac", 14)) {
     FileCreateKey(mMailSpoolDir, mReg);
   } else {
#endif
     RegCreateKey( (HKEY) HKEY_LOCAL_MACHINE, (LPCTSTR) mReg, &hKey);
     RegCloseKey((HKEY)hKey);
#ifdef REGTOFILE
   }
   if (nClustering && !strnicmp(DOMAIN_REG, "software\\emwac", 14)) {
     FileCreateKey(mMailSpoolDir, DOMAIN_REG);
     FileCreateKey(mMailSpoolDir, DOMAIN_ACCOUNT);
     FileCreateKey(mMailSpoolDir, DOMAIN_SMTPIP);
     FileCreateKey(mMailSpoolDir, DOMAIN_POP3IP);
     FileCreateKey(mMailSpoolDir, DOMAIN_IMAP4IP);
     FileCreateKey(mMailSpoolDir, DOMAIN_FOLDER);
   } else {
#endif
     ///////////////////////////
     CreateProfile(NULL, DOMAIN_REG);
     CreateProfile(NULL, DOMAIN_ACCOUNT);
#ifdef V3
     CreateProfile(NULL, DOMAIN_SMTPIP);
     CreateProfile(NULL, DOMAIN_POP3IP);
     CreateProfile(NULL, DOMAIN_IMAP4IP);
     CreateProfile(NULL, DOMAIN_FOLDER);
#endif
#ifdef REGTOFILE
   }
#endif
	 //////////////////////////////////////////////////////////////
	 CreateProfile(NULL, SYSTEM_SMTPDS_REG);
	 WriteProfileIntEx(SYSTEM_SMTPDS_REG, "MaxThread", 30);   //デフォルトはシングルスレッド
	 WriteProfileIntEx(SYSTEM_SMTPDS_REG, "SendDataTimeout", 600);   // 60秒×10×1 = 10分
	 WriteProfileIntEx(SYSTEM_SMTPDS_REG, "RecvDataTimeout", 600);   // 60秒×10×1 = 10分
     //////////////////////////////////////////////////////////////
#ifdef REGTOFILE
     if (bUNC) // UNC接続の場合
	   sprintf(mMailbox, "%s\\mail\\inbox\\%%USERNAME%%", mSpool);
	 else
#ifdef UPDATE_20060827 // メールスプールフォルダドライブがメールボックスフォルダ・アカウントＤＢフォルダのドライブに反映されない
	   sprintf(mMailbox, "%c:\\mail\\inbox\\%%USERNAME%%", (char)cDrv);
#else
	   sprintf(mMailbox, "%c:\\mail\\inbox\\%%USERNAME%%", (char)(_getdrive() + 'A' - 1 ));
#endif
#else
#ifdef UPDATE_20060827 // メールスプールフォルダドライブがメールボックスフォルダ・アカウントＤＢフォルダのドライブに反映されない
	 sprintf(mMailbox, "%c:\\mail\\inbox\\%%USERNAME%%", (char)cDrv);
#else
	 sprintf(mMailbox, "%c:\\mail\\inbox\\%%USERNAME%%", (char)(_getdrive() + 'A' - 1 ));
#endif
#endif
	 //AfxMessageBox( mMailbox, MB_OK);
     WriteProfileStringEx(SOFT_REG,"MailInBoxDir", mMailbox); // メールボックスフォルダ
     //SMTP認証
	 //WriteProfileIntEx(SYSTEM_SMTPRS_REG, "SMTPAUTHOnly", 2);
     //WriteProfileStringEx(SYSTEM_SMTPRS_REG, "SMTPAUTHMode", "PLAIN LOGIN CRAM-MD5"); //mSMTPAUTHMODE
     //VRFY,EXPNへの応答の有無
	 WriteProfileIntEx(SOFT_REG, "Vrfy", FALSE);
     /////////////////////////////////	

     WriteProfileIntEx(SOFT_REG, "UserManager", (INT)(Wiz1.m_Sel == 0 ? 0 : 1));
	 if (Wiz1.m_Sel == 0) { // SoftAccount管理
       char mLongPath[256],  mShortPath[256];
#ifdef REGTOFILE
	 if (bUNC) { // UNC接続の場合
       sprintf(mLongPath, "%s\\db\\", mSpool);
	   _mkdir(mLongPath);
	 } else
 	   sprintf(mLongPath, "%s\\", mPath);
#else
 	   sprintf(mLongPath, "%s\\", mPath);
#endif
       WriteProfileStringEx(SOFT_REG,"Membership", mLongPath); // アカウントフォルダの指定
       mShortPath[0] = '\x0';
       GetShortPathName( mLongPath, mShortPath, sizeof(mShortPath));
	   if (mShortPath[0] && strcmp(mLongPath, mShortPath) != 0)
         WriteProfileStringEx(SOFT_REG,"Membership", mShortPath); // アカウントフォルダの指定(ショートパスに変換)
     } else if (Wiz1.m_Sel == 1) {  // Windows管理
       WriteProfileStringEx(SOFT_REG,"Membership", ""); // PDCのアカウントを有効にする。
       WriteProfileStringEx(SOFT_REG, "MailGroup", (char *)((const char *)Wiz11.m_LocalGroup));  // ローカルグループ設定
	   UserRight((char *)((const char *)Wiz11.m_LocalGroup), "", TRUE);                        // 「バッチジョブによるログオン権利設定」
	 } else if (Wiz1.m_Sel == 2) {
       WriteProfileStringEx(SOFT_REG,"Membership", (char *)((const char *)Wiz12.m_PDC)); // PDCのアカウントを有効にする。
       WriteProfileStringEx(SOFT_REG, "MailGroup", (char *)((const char *)Wiz12.m_LocalGroup));  // ローカルグループ設定
#ifdef UPDATE_20070124 // "<ドメイン名>\Domain Users"をローカルポリシーの「バッチジョブのログオン権限に定義」
	   if (Wiz12.m_PDC[0]) { // PDCにドメイン名があるなら
		 CString mDomainGroup = Wiz12.m_PDC + (CString)"\\Domain Users";
	     UserRight((char *)((const char *)mDomainGroup), NULL, TRUE);                        // 「バッチジョブによるログオン権利設定」
	   }
#endif
#ifdef UPDATE_20050128
       char mDGAccount[256];
	   sprintf(mDGAccount, "%s\\%s", Wiz12.m_PDC, Wiz12.m_LocalGroup);
	  //AfxMessageBox( mDGAccount, MB_OK);
       AddLocalGroupAccount(NULL, NULL, mDGAccount, (char *)((const char *)Wiz12.m_LocalGroup));
	   UserRight((char *)((const char *)Wiz12.m_LocalGroup), NULL, TRUE);                        // 「バッチジョブによるログオン権利設定」
#endif
	   UserRight((char *)((const char *)Wiz12.m_LocalGroup), (char *)((const char *)Wiz12.m_PDC), TRUE);                        // 「バッチジョブによるログオン権利設定」
	 }
	 CString mDNS = (CString) "";
	 if (Wiz2.m_DNS1 != (CString)"")
	   mDNS = Wiz2.m_DNS1;
	 if (Wiz2.m_DNS2 != (CString)"")
	   mDNS = mDNS + (CString)" " + Wiz2.m_DNS2;
	 if (Wiz2.m_DNS3 != (CString)"")
	   mDNS = mDNS + (CString)" " + Wiz2.m_DNS3;
	 WriteProfileStringEx((char *)"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters", "NameServer", (char *)((const char *)mDNS));

	 mDomain[0] = '\x0';
	 DWORD     n, nLen = 0, nL = 0;
	 CHAR      *pL[3], mListenIP[3][256] = {"", "", ""};
	 pL[0] = mListenIP[0];
	 pL[1] = mListenIP[1];
	 pL[2] = mListenIP[2];
	 if (Wiz3.m_Name1 != (CString)"") {
	   n = strlen(Wiz3.m_Name1);
	   strcpy(mDomain, Wiz3.m_Name1);
	   nLen = n;
	   if (Wiz3.m_IP1 != (CString)"") {
   	     sprintf(pL[0], "%s 25",  (char *)((const char *)Wiz3.m_IP1));
	     pL[0] += strlen(pL[0])+1;
   	     sprintf(pL[1], "%s 110",  (char *)((const char *)Wiz3.m_IP1));
	     pL[1] += strlen(pL[1])+1;
   	     sprintf(pL[2], "%s 143",  (char *)((const char *)Wiz3.m_IP1));
	     pL[2] += strlen(pL[2])+1;
	   }
       WriteProfileStringEx(DOMAIN_SMTPIP, (char *)((const char *)Wiz3.m_Name1), (char *)((const char *)Wiz3.m_IP1));    // 対象ドメインのSMTP応答ＩＰアドレス設定
       WriteProfileStringEx(DOMAIN_POP3IP, (char *)((const char *)Wiz3.m_Name1), (char *)((const char *)Wiz3.m_IP1));    // 対象ドメインのPOP3応答ＩＰアドレス設定
       WriteProfileStringEx(DOMAIN_IMAP4IP, (char *)((const char *)Wiz3.m_Name1), (char *)((const char *)Wiz3.m_IP1));    // 対象ドメインのIMAP4応答ＩＰアドレス設定
       WriteProfileStringEx(DOMAIN_FOLDER, (char *)((const char *)Wiz3.m_Name1), (Wiz3.m_IP1 == "" ? "" : (char *)((const char *)Wiz3.m_Name1)));  // 対象ドメイン宛のメールの保存先拡張名
	 }
	 if (Wiz3.m_Name2 != (CString)"") {
	   n = strlen(Wiz3.m_Name2);
	   strcpy(&mDomain[nLen+1], Wiz3.m_Name2);
	   nLen = nLen + 1 + n;
#ifdef UPDATE_20051130 // マルチドメイン設定で同じＩＰアドレスの場合重複してＩＰが登録されてしまう不具合。
	   if (Wiz3.m_IP1 != Wiz3.m_IP2 &&  // IPアドレスの重複設定禁止
		   Wiz3.m_IP2 != (CString)"") {
#else
	   if (Wiz3.m_IP2 != (CString)"") {
#endif
   	     sprintf(pL[0], "%s 25",  (char *)((const char *)Wiz3.m_IP2));
	     pL[0] += strlen(pL[0])+1;
   	     sprintf(pL[1], "%s 110",  (char *)((const char *)Wiz3.m_IP2));
	     pL[1] += strlen(pL[1])+1;
   	     sprintf(pL[2], "%s 143",  (char *)((const char *)Wiz3.m_IP2));
	     pL[2] += strlen(pL[2])+1;
	   }
       WriteProfileStringEx(DOMAIN_SMTPIP, (char *)((const char *)Wiz3.m_Name2), (char *)((const char *)Wiz3.m_IP2));    // 対象ドメインのSMTP応答ＩＰアドレス設定
       WriteProfileStringEx(DOMAIN_POP3IP, (char *)((const char *)Wiz3.m_Name2), (char *)((const char *)Wiz3.m_IP2));    // 対象ドメインのPOP3応答ＩＰアドレス設定
       WriteProfileStringEx(DOMAIN_IMAP4IP, (char *)((const char *)Wiz3.m_Name2), (char *)((const char *)Wiz3.m_IP2));    // 対象ドメインのIMAP4応答ＩＰアドレス設定
       WriteProfileStringEx(DOMAIN_FOLDER, (char *)((const char *)Wiz3.m_Name2), (char *)((const char *)Wiz3.m_Name2));  // 対象ドメイン宛のメールの保存先拡張名
	 }
	 if (Wiz3.m_Name3 != (CString)"") {
	   n = strlen(Wiz3.m_Name3);
	   strcpy(&mDomain[nLen+1], Wiz3.m_Name3);
	   nLen = nLen + 1 + n;
#ifdef UPDATE_20051130 // マルチドメイン設定で同じＩＰアドレスの場合重複してＩＰが登録されてしまう不具合。
	   if (Wiz3.m_IP3 != Wiz3.m_IP1 &&  // IPアドレスの重複設定禁止
		   Wiz3.m_IP3 != Wiz3.m_IP2 &&  // IPアドレスの重複設定禁止
	       Wiz3.m_IP3 != (CString)"") {
#else
	   if (Wiz3.m_IP3 != (CString)"") {
#endif
   	     sprintf(pL[0], "%s 25",  (char *)((const char *)Wiz3.m_IP3));
	     pL[0] += strlen(pL[0])+1;
   	     sprintf(pL[1], "%s 110",  (char *)((const char *)Wiz3.m_IP3));
	     pL[1] += strlen(pL[1])+1;
   	     sprintf(pL[2], "%s 143",  (char *)((const char *)Wiz3.m_IP3));
	     pL[2] += strlen(pL[2])+1;
	   }
       WriteProfileStringEx(DOMAIN_SMTPIP, (char *)((const char *)Wiz3.m_Name3), (char *)((const char *)Wiz3.m_IP3));    // 対象ドメインのSMTP応答ＩＰアドレス設定
       WriteProfileStringEx(DOMAIN_POP3IP, (char *)((const char *)Wiz3.m_Name3), (char *)((const char *)Wiz3.m_IP3));    // 対象ドメインのPOP3応答ＩＰアドレス設定
       WriteProfileStringEx(DOMAIN_IMAP4IP, (char *)((const char *)Wiz3.m_Name3), (char *)((const char *)Wiz3.m_IP3));    // 対象ドメインのIMAP4応答ＩＰアドレス設定
       WriteProfileStringEx(DOMAIN_FOLDER, (char *)((const char *)Wiz3.m_Name3), (char *)((const char *)Wiz3.m_Name3));  // 対象ドメイン宛のメールの保存先拡張名
	 }
     WriteProfileIntEx(SYSTEM_SMTPRS_REG, "ListenMode",  FALSE);
	 if (mListenIP[0][0]) {
       nL = (DWORD)pL[0] - (DWORD)&mListenIP[0]-1;
       pL[0]++;
	   CreateProfile(NULL, SYSTEM_SMTPRS_REG);
	   WriteProfileIntEx(SYSTEM_SMTPRS_REG, "ListenMode",  TRUE);
       WriteProfileStringExType(SYSTEM_SMTPRS_REG, "ListenIP", mListenIP[0], nL, REG_MULTI_SZ);
	 }
     if (nProductcode == 0) { // 製品がMail Serverならば設定する
       WriteProfileIntEx(SYSTEM_POP3_REG, "ListenMode",  FALSE);
	   if (mListenIP[1][0]) {
         nL = (DWORD)pL[1] - (DWORD)&mListenIP[1]-1;
         pL[1]++;
	     CreateProfile(NULL, SYSTEM_POP3_REG);
	     WriteProfileIntEx(SYSTEM_POP3_REG, "ListenMode",  TRUE);
         WriteProfileStringExType(SYSTEM_POP3_REG, "ListenIP", mListenIP[1], nL, REG_MULTI_SZ);
	   }
       WriteProfileIntEx(SYSTEM_IMAP4_REG, "ListenMode",  FALSE);
   	   if (mListenIP[2][0]) {
         nL = (DWORD)pL[2] - (DWORD)&mListenIP[2]-1;
         pL[2]++;
	     CreateProfile(NULL, SYSTEM_IMAP4_REG);
	     WriteProfileIntEx(SYSTEM_IMAP4_REG, "ListenMode",  TRUE);
         WriteProfileStringExType(SYSTEM_IMAP4_REG, "ListenIP", mListenIP[2], nL, REG_MULTI_SZ);
	   }
	 }
     WriteProfileBinaryEx(SOFT_REG, "DomainNamesAreLocal", (char *)((const char *)mDomain), nLen);
	 WriteProfileStringEx(SOFT_REG,"PostMaster", (char *)((const char *) Wiz4.m_Postmaster)); // 管理者アドレス
#ifdef QSEND
     WriteProfileStringEx(SYSTEM_SMTPDS_REG,"SendGateway", ((const char *) Wiz7.m_GATEIP));
	 WriteProfileIntEx(SYSTEM_SMTPDS_PARAM_REG, "PortNo", Wiz7.m_GATEPort);  //smtpds ポート番号
	 WriteProfileIntEx(SYSTEM_SMTPDS_REG, "MailForward", Wiz7.m_Gateway);  // 全受信をSMTP GateWayへ転送。
	 WriteProfileIntEx(SYSTEM_SMTPRS_REG, "MailBackup", Wiz7.m_Gateway); // TRUE:バックアップする FALSE:バックアップしない。
     sprintf(mFn, "%s\\qsend.ini", mPath);
	 if ((fp = fopen(mFn, "wt"))) {
	   fprintf(fp, "/q%s\n", Wiz7.m_ARCFolder); ///qi:\mail\backup
	   fprintf(fp, "/s%s\n", Wiz7.m_ARCIP);     ///s192.168.1.15
	   fprintf(fp, "/n%d\n", Wiz7.m_ARCPort);   ///N25
	   fclose(fp);
	 }
#endif
#ifdef REGTOFILE
  } else { // ウィザードキャンセル
     if (nClustering && !strnicmp(PRODUCTS_ROOT, "software\\emwac", 14)) {
       FileCreateKey(mMailSpoolDir, PRODUCTS_ROOT);
	 } else {
       CreateProfile(NULL, PRODUCTS_ROOT);
	 }
	  /// 旧設定からの以降
      HKEY hKeyRoot = HKEY_LOCAL_MACHINE;
      HKEY hKey;
	  DWORD  retCode;
        retCode = 
          RegOpenKeyEx(hKeyRoot,
                   (LPCTSTR)DOMAIN_REG,
                   0,
                   KEY_READ,
                   &hKey);
	  if (retCode != ERROR_SUCCESS) { // 未設定なら旧データの移行を行う
		RestoreFile(mFn);
        ///////////////////////////////////
	    CHAR *p1, mOLDPath[256], mOLDFiles[256], mNEWPath[256];
	    CHAR mSrc[256], mDest[256];
	    sprintf(mNEWPath, "%s\\", mPath);
        GetProfileStringEx("SYSTEM\\CurrentControlSet\\Services\\SPARS-PRO", "ImagePath", "", mOLDPath, sizeof(mOLDPath)); // 旧インストールパス
        if ((p1 = strrchr(mOLDPath, '\\')))
          *(p1+1) = '\x0';
        ///////////////////////////////////
        HANDLE             hF;
        WIN32_FIND_DATA    FD;
        BOOL bF = TRUE;

        sprintf(mOLDFiles, "%s*.dat", mOLDPath);
        hF = FindFirstFile(mOLDFiles, &FD);
        if (hF != INVALID_HANDLE_VALUE) {
          while (bF) {
	        sprintf(mSrc, "%s%s", mOLDPath, FD.cFileName);
	 	    sprintf(mDest, "%s%s", mNEWPath, FD.cFileName);
		    if (!CopyFile(mSrc, mDest, TRUE)) {
		       CString m;
		       CHAR    mMess[256];
               m.LoadString(IDS_STRING114);
               sprintf(mMess, (LPCSTR)m, FD.cFileName);
		       if (MessageBox(NULL, mMess, "EasyWiz", MB_YESNO | MB_ICONWARNING) == IDYES)
                 CopyFile(mSrc, mDest, FALSE);
		    }
            bF = FindNextFile( hF, &FD);
	      }
        }; 
        FindClose( hF ); 
        ///////////////////////////////////
        sprintf(mOLDFiles, "%s*.idx", mOLDPath);
        hF = FindFirstFile(mOLDFiles, &FD);
        if (hF != INVALID_HANDLE_VALUE) {
          while (bF) {
	        sprintf(mSrc, "%s%s", mOLDPath, FD.cFileName);
	 	    sprintf(mDest, "%s%s", mNEWPath, FD.cFileName);
		    if (!CopyFile(mSrc, mDest, TRUE)) {
		       CString m;
		       CHAR    mMess[256];
               m.LoadString(IDS_STRING114);
               sprintf(mMess, (LPCSTR)m, FD.cFileName);
		       if (MessageBox(NULL, mMess, "EasyWiz", MB_YESNO | MB_ICONWARNING) == IDYES)
                 CopyFile(mSrc, mDest, FALSE);
		    }
            bF = FindNextFile( hF, &FD);
	      }
        }; 
        FindClose( hF ); 
        ///////////////////////////////////
	  } else
	    RegCloseKey(hKey);
	/////////////////////////////////////////
#endif
  }
#ifdef E_POST
  WinExec("epstcontrol.exe", SW_SHOWNORMAL);
#else
  WinExec("spacontrol.exe", SW_SHOWNORMAL);
#endif
#ifdef QSEND
  WinExec("qset.exe", SW_SHOWNORMAL);
#endif
}

