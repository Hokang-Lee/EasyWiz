// ユーザーの権利を定義します。[バッチジョブとしてのログオン]
//  アカウントのアクセス権を解除する場合は、LsaRemoveAccountRights() を使用
//
#include "stdafx.h"
#ifndef UNICODE
#define UNICODE
#endif // UNICODE

#include <windows.h>
#include <stdio.h>
#include <lmaccess.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <locale.h>

#include "ntsecapi.h"
#include "profile.h"

NTSTATUS OpenPolicy(
      LPWSTR ServerName,          // ポリシーをオープンするマシン名 (Unicode)
      DWORD DesiredAccess,        // 必要なアクセス権 
      PLSA_HANDLE PolicyHandle    // 獲得したポリシー ハンドル 
      );

BOOL GetAccountSid(
      LPTSTR SystemName,          // アカウントを検索する場所
      LPTSTR AccountName,         // 対象とするアカウント
      PSID *Sid                   // SID を入れるバッファ
      );

NTSTATUS SetPrivilegeOnAccount(
      LSA_HANDLE PolicyHandle,    // オープンするポリシー ハンドル
      PSID AccountSid,            // アクセス権を与える SID
      LPWSTR PrivilegeName,       // 与えるアクセス権 (Unicode)
      BOOL bEnable                // 有効または無効
      );

void InitLsaString(
      PLSA_UNICODE_STRING LsaString, // 変換後の文字列
      LPWSTR String                  // 元の文字列 (Unicode)
      );

void DisplayNtStatus(
      LPSTR szAPI,                // 関数名へのポインタ (ANSI)
      NTSTATUS Status             // NTSTATUS エラーの値 
      );

void DisplayWinError(
      LPSTR szAPI,                // 関数名へのポインタ (ANSI)
      DWORD WinError              // DWORD WinError
      );

#define RTN_OK 0
#define RTN_USAGE 1
#define RTN_ERROR 13

// ddk がある場合は、ntstatus.h をインクルードしてください
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS  ((NTSTATUS)0x00000000L)
#endif

NET_API_STATUS AddLocalGroupAccount(CHAR *lpszContry,
 					                CHAR *lpszDomain,
                                    CHAR *lpszUser,
                                    CHAR *lpszLocalGroup )
{
    NET_API_STATUS            ePDC = 0, err = 0;
    LOCALGROUP_MEMBERS_INFO_3 localgroup_members;
    wchar_t wszDomain[65], wszUser[256], wszLocalGroup[256];
	LPWSTR  lpwszDomain;
    LPWSTR  lpwszPrimaryDC = NULL;
    
    if (!lpszLocalGroup)
	  return 0;
	if (*lpszLocalGroup == '\x0')
	  return 0;

    if (lpszContry)
      setlocale(LC_ALL, lpszContry);
    lpwszPrimaryDC = lpwszDomain = NULL;
    if (lpszDomain) {
	  if (*lpszDomain){
        mbstowcs(wszDomain,     lpszDomain,     65);
        lpwszDomain = wszDomain;
 	    ePDC = NetGetAnyDCName(NULL, lpwszDomain,(LPBYTE *)&lpwszPrimaryDC);
	  }
    }

    mbstowcs(wszUser,       lpszUser,       256);
    mbstowcs(wszLocalGroup, lpszLocalGroup, 256);

    localgroup_members.lgrmi3_domainandname = wszUser;

    err = NetLocalGroupAddMembers( lpwszPrimaryDC,       // PDC name
                                   wszLocalGroup,        // group name
                                   3,                    // passing in name
                                   (LPBYTE)&localgroup_members, // Buffer
                                   1 );                  // count passed in 

    switch ( err )
    {
    case 0:
        //printf("User successfully added to Local Group.\n");
        break;
    case ERROR_MEMBER_IN_ALIAS:
        printf("User already in Local Group.<BR>\n");
        err = 0;
        break;
    default:
        printf("Error adding User to Local Group: %d<BR>\n", err);
        break;
    }

    if (lpwszPrimaryDC)
      NetApiBufferFree(lpwszPrimaryDC);

    return( err );
}

NET_API_STATUS NewLocalGroup(CHAR *lpszPrimaryDC,
							 CHAR *lpszLocalGroup,
 					         CHAR *lpszLGComment)

{
    LOCALGROUP_INFO_1         LGInfo;
    DWORD parm_err;
    wchar_t  wszPDC[24], wszLG[256], wszLGC[256];
    wchar_t  wszMachine[24];
	LPBYTE  pbuff;
    NET_API_STATUS nSts;

	wsprintfW(wszPDC, L"%hS", lpszPrimaryDC);
	wsprintfW(wszLG,  L"%hS", lpszLocalGroup);
	wsprintfW(wszLGC, L"%hS", lpszLGComment);
	LGInfo.lgrpi1_name = wszLG;
    LGInfo.lgrpi1_comment = wszLGC;

//AfxMessageBox( lpszPrimaryDC, MB_OK);
//AfxMessageBox( lpszLocalGroup, MB_OK);
//AfxMessageBox( lpszLGComment, MB_OK);

	if (*lpszPrimaryDC) {
      //mbstowcs( wDom, lpszPrimaryDC, 65);
 	  nSts = NetGetAnyDCName(NULL, 
                             wszPDC,
                             &pbuff);
	  if (nSts == NERR_Success) {
	    wcscpy(wszMachine, (const wchar_t *)pbuff);
        NetApiBufferFree(pbuff);
	  }
	}

#ifdef UPDATE_20050128
	if (*lpszPrimaryDC) {  // PDCの指定ならローカルにもユーザグループ作成
      nSts = NetLocalGroupAdd(NULL,  // Machine name
                             1,                          // level
                            (LPBYTE)&LGInfo,   // input buffer
                             &parm_err );                // parm in error
	}
#endif
#ifdef UPDATE_20050816     // W2K以降のADの場合
	if (strchr(lpszPrimaryDC, '.'))
      nSts = NetGroupAdd((lpszPrimaryDC ? (*lpszPrimaryDC ? wszMachine : NULL) : NULL),  // 実行対象のリモートサーバー
                          1,           // 情報レベル
                         (LPBYTE)&LGInfo,   // 情報を保持しているバッファ
                          &parm_err);       // エラーの発生場所を示すインデックス
	else 
#endif
    nSts = NetLocalGroupAdd((*lpszPrimaryDC ? wszMachine : NULL),                 // Machine name
                             1,                          // level
                            (LPBYTE)&LGInfo,   // input buffer
                             &parm_err );                // parm in error
	return nSts;
}

int UserRight(char *Account, char *Machine, BOOL bAction) {

  LSA_HANDLE PolicyHandle;
  WCHAR wComputerName[256]=L"";   // マシン名用の static なバッファ
  CHAR  mComputerName[256]="";
  WCHAR wszPDC[65];
  TCHAR AccountName[256];         // アカウント名用の static なバッファ
  PSID pSid;
  NTSTATUS Status;
  int iRetVal=RTN_ERROR;          // メインからエラーが戻るとする
  NET_API_STATUS nSts;
  LPBYTE  pbuff;

   wsprintf(AccountName, TEXT("%hS"), Account);
   mComputerName[0] = '\x0';
   if (Machine) {
	 if (*Machine) {
        wsprintfW(wszPDC, L"%hS", Machine);
 	    nSts = NetGetAnyDCName(NULL, 
                             wszPDC,
                             &pbuff);
	    if (nSts == NERR_Success) {
	      wcscpy(wComputerName, (const wchar_t *)pbuff);
          wcstombs( (char *)mComputerName, (wchar_t *)wComputerName, sizeof(mComputerName));
          NetApiBufferFree(pbuff);
		}
	  }
	}
    //if (*Machine) 
	 //wsprintfW(wComputerName, L"%hS", Machine);

    // 対象マシンのポリシーをオープンする
    if((Status=OpenPolicy(
                  wComputerName,      // 対象マシンの名前
                  POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES,
                  &PolicyHandle       // 得られたポリシーのハンドル
                  )) != STATUS_SUCCESS) {
          //DisplayNtStatus("OpenPolicy", Status);
          return RTN_ERROR;
      }

      //
      // ユーザーまたはグループの SID を取得する
      // ここではマシンを特定していませんが、そうすることもできることに注意して
      // ください 
      // 対象のマシンが以下の順番で SID を検索するように NULL を指定します
      // ローカル、プライマリ ドメイン、
      // 信頼関係のドメイン
      //
      if(GetAccountSid(
		      //((*Machine) ? Machine : NULL),  // デフォルトの検索方法
		      ((mComputerName[0]) ? mComputerName : NULL),  // デフォルトの検索方法
              AccountName,// SID を設定するアカウント
              &pSid       // 得られた SID を格納するためのバッファ
              )) {
          // pSid で示されるユーザーに SeServiceLogonRight を与える
          if((Status=SetPrivilegeOnAccount(
                      PolicyHandle,           // ポリシー ハンドル
                      pSid,                   // アクセス権を与える SID
                      L"SeBatchLogonRight",   // Unicode のアクセス権 L"SeServiceLogonRight",
                      bAction //TRUE          // 有効なアクセス権
                      )) == STATUS_SUCCESS)
              iRetVal=RTN_OK;
         // else
             // DisplayNtStatus("AddUserRightToAccount", Status);
      }
      else {
          // SID 取得時のエラー
          //DisplayWinError("GetAccountSid", GetLastError());
      }

      // ポリシー ハンドルをクルーズする
      LsaClose(PolicyHandle);
      // SID 用に割当てたメモリを解放する
      if(pSid != NULL)
		HeapFree(GetProcessHeap(), 0, pSid);

      return iRetVal;
}

void InitLsaString(
      PLSA_UNICODE_STRING LsaString,
      LPWSTR String
      )
  {
      DWORD StringLength;

      if (String == NULL) {
          LsaString->Buffer = NULL;
          LsaString->Length = 0;
          LsaString->MaximumLength = 0;
          return;
      }

      StringLength = wcslen(String);
      LsaString->Buffer = String;
      LsaString->Length = (USHORT) StringLength * sizeof(WCHAR);
      LsaString->MaximumLength=(USHORT)(StringLength+1) * sizeof(WCHAR);
}

NTSTATUS OpenPolicy(
      LPWSTR ServerName,
      DWORD DesiredAccess,
      PLSA_HANDLE PolicyHandle
      )
  {
      LSA_OBJECT_ATTRIBUTES ObjectAttributes;
      LSA_UNICODE_STRING ServerString;
      PLSA_UNICODE_STRING Server = NULL;

      // 必ずオブジェクトの属性をすべて 0 で初期化する
      ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

      if (ServerName != NULL) {
          // 指定された LPWSTR から LSA_UNICODE_STRING を作成する
          InitLsaString(&ServerString, ServerName);
          Server = &ServerString;
      }
      // ポリシーをオープンする
      return LsaOpenPolicy(
                  Server,
                  &ObjectAttributes,
                  DesiredAccess,
                  PolicyHandle
                  );
}

  /*++
  この関数は、指定されたアカウントで表される SID を獲得します。

  獲得に成功した場合、TRUE が返却され、指定されたアカウントで表される SID が
  バッファに格納されます。このバッファは使用しなくなった時点で次のように解放
  しなければなりません。
  HeapFree(GetProcessHeap(), 0, buffer)

  獲得に失敗した場合、FALSE が返却されます。詳細なエラー情報を得るには 
  GetLastError() を呼び出します。

  --*/

BOOL GetAccountSid(
      LPTSTR SystemName,
      LPTSTR AccountName,
      PSID *Sid
      )
  {
      LPTSTR ReferencedDomain=NULL;
      DWORD cbSid=128;    // 割当てるサイズの初期値  
      DWORD cchReferencedDomain=16; // 割当てるサイズの初期値
      SID_NAME_USE peUse;
      BOOL bSuccess=FALSE; // 初期値は失敗

      __try {

      // 初回のメモリ割当て
      if((*Sid=HeapAlloc(
                      GetProcessHeap(),
                      0,
                      cbSid
                      )) == NULL) __leave;

      if((ReferencedDomain=(LPTSTR)HeapAlloc(
                      GetProcessHeap(),
                      0,
                      cchReferencedDomain * sizeof(TCHAR)
                      )) == NULL) __leave;

      // 指定されたシステム上の指定されたアカウントの SID を獲得する
      while(!LookupAccountName(
                      SystemName,         // アカウントを検索するマシン
                      AccountName,        // 検索するアカウント
                      *Sid,               // 対象の SID
                      &cbSid,             // SID のサイズ
                      ReferencedDomain,   // ドメインアカウントが検出された場所
                      &cchReferencedDomain,
                      &peUse
                      )) {
          if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
              // reallocate memory
              if((*Sid=HeapReAlloc(
                          GetProcessHeap(),
                          0,
                         *Sid,
                          cbSid
                          )) == NULL) __leave;

              if((ReferencedDomain=(LPTSTR)HeapReAlloc(
                          GetProcessHeap(),
                          0,
                          ReferencedDomain,
                          cchReferencedDomain * sizeof(TCHAR)
                          )) == NULL) __leave;
          }
          else __leave;
      }

      // 成功を設定する
      bSuccess=TRUE;

      } // 後処理 
      __finally { 

      // クリーンアップして、失敗を設定する
      HeapFree(GetProcessHeap(), 0, ReferencedDomain);

      if(!bSuccess) {
          if(*Sid != NULL) {
              HeapFree(GetProcessHeap(), 0, *Sid);
              *Sid = NULL;
          }
      }

      } // 後処理 

      return bSuccess;
}

NTSTATUS
  SetPrivilegeOnAccount(
      LSA_HANDLE PolicyHandle,    // ポリシー ハンドルをオープンする
      PSID AccountSid,            // アクセス権を与える SID
      LPWSTR PrivilegeName,       // 与えるアクセス権 (Unicode)
      BOOL bEnable                // 有効または無効
      )
  {
      LSA_UNICODE_STRING PrivilegeString;

      // アクセス権の名前を LSA_UNICODE_STRING で作成する
      InitLsaString(&PrivilegeString, PrivilegeName);

      // 指定に従って、アクセス権を設定または解除する
      if(bEnable) {
          return LsaAddAccountRights(
                  PolicyHandle,       // ポリシー ハンドルをオープンする
                  AccountSid,         // 対象の SID
                  &PrivilegeString,   // アクセス権
                  1                   // アクセス権の数
                  );
      }
      else {
          return LsaRemoveAccountRights(
                  PolicyHandle,       // ポリシー ハンドルをオープンする
                  AccountSid,         // 対象の SID
                  FALSE,             // すべてのアクセス権を無効にするのではない
                  &PrivilegeString,   // アクセス権
                  1                   // アクセス権の数
                  );
      }
}

/*
  void
  DisplayNtStatus(
      LPSTR szAPI,
      NTSTATUS Status
      )
  {
      //
      // NTSTATUS を Winerror に変換し、DisplayWinError() を呼び出す
      //
      DisplayWinError(szAPI, LsaNtStatusToWinError(Status));
  }

  void
  DisplayWinError(
      LPSTR szAPI,
      DWORD WinError
      )
  {
      LPSTR MessageBuffer;
      DWORD dwBufferLength;

      //
      // TODO: 次の fprintf はデバッグ用です。削除してください !
      //
      fprintf(stderr,"%s error!\n", szAPI);

      if(dwBufferLength=FormatMessageA(
                          FORMAT_MESSAGE_ALLOCATE_BUFFER |
                          FORMAT_MESSAGE_FROM_SYSTEM,
                          NULL,
                          WinError,
                          GetUserDefaultLangID(),
                          (LPSTR) &MessageBuffer,
                          0,
                          NULL
                          ))
      {
          DWORD dwBytesWritten; // 使用しません

          // 
          // stderr にメッセージを出力する
          //
          WriteFile(
              GetStdHandle(STD_ERROR_HANDLE),
              MessageBuffer,
              dwBufferLength,
              &dwBytesWritten,
              NULL
              );

          //
          // システムが割当てたバッファを解放する
          //
          LocalFree(MessageBuffer);
      }
  }

  */