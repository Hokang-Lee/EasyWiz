///////////////////////////////////////////////////////////
// レジストリ情報の管理をフォルダで処理
// Copyright K-TEC Corp. K.Kawakami
///////////////////////////////////////////////////////////
#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <direct.h>
#include "profile.h"

#ifdef REGTOFILE
///////////////////////////////////////////////////////////
// レジストリ情報保管フォルダの作成
///////////////////////////////////////////////////////////
DWORD FileCreateKey(char *pKeyRoot, char *pKey) {
  char mKeyName[256];
  char *p;

  sprintf(mKeyName, "%s\\reg\\%s", pKeyRoot, pKey);
  p = strstr(mKeyName,":\\");
  if (p)
    p = strstr(p+2,"\\");
  else {
    if ((p = strstr(mKeyName,"\\\\")))
      if ((p = strstr(p+2,"\\")))
       p = strstr(p+1,"\\");
  }
  while(p) {
    *p = '\x0';
    _mkdir(mKeyName);         // 処理用フォルダ作成
    *p = '\\';
     p = strstr(p+1,"\\");
  }
  _mkdir(mKeyName);         // 処理用フォルダ作成

  return ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////
// 定義ファイルの排他アクセスを可能にする
///////////////////////////////////////////////////////////
DWORD OpenKeyFile(char *pKeyRoot, char *pKey, char *pValue, HANDLE *hFile) {
  char mKeyLock[256];
  HANDLE             hF;
  WIN32_FIND_DATA    FD;

  // フォルダの存在を確認
  sprintf(mKeyLock, "%s\\reg\\%s*", pKeyRoot, pKey);
  hF = FindFirstFile(mKeyLock, &FD);
  if (hF == INVALID_HANDLE_VALUE) // フォルダが無い＝存在しない
    return -1;
  FindClose( hF ); 

  sprintf(mKeyLock, "%s\\reg\\%s%s.lck", pKeyRoot, pKey, pValue);
  while ((*hFile = CreateFile((LPCTSTR)mKeyLock,
                        GENERIC_WRITE,
                        0,   // 排他アクセス = 0
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, 
                        NULL)) == INVALID_HANDLE_VALUE) {
    //if (bServiceTerminating) 
      //return -1;
    Sleep(0); //WAIT_TIME);
  } 
  return ERROR_SUCCESS;
}

DWORD KeyFileQueryValueEx(char *pKeyRoot, char *pKey, char *pValue, HANDLE hFile, DWORD dwType, LPBYTE pRet, DWORD *nSize) {
  DWORD nSts = -1;
  char mKeyName[256];
  FILE *fp;

    switch(dwType) {
       case REG_BINARY: memset(pRet, 0, *nSize); // クリア
                        sprintf(mKeyName, "%s\\reg\\%s%s.0", pKeyRoot, pKey, pValue);
                        if ((fp = fopen(mKeyName, "rb"))) {
                          *nSize = fread(pRet, sizeof(char), *nSize, fp);
                          fclose(fp);
                          nSts = ERROR_SUCCESS;
                        }
		    break;
       case REG_DWORD: sprintf(mKeyName, "%s\\reg\\%s%s.1", pKeyRoot, pKey, pValue);
                       if ((fp = fopen(mKeyName, "rt"))) {
                          fscanf(fp, "%lu", (DWORD *)pRet);
                          *nSize = sizeof(DWORD);
                          fclose(fp);
                          nSts = ERROR_SUCCESS;
                       }
 	            break;
       case REG_NONE:  // 無視
		    break;
       default:     sprintf(mKeyName, "%s\\reg\\%s%s.2", pKeyRoot, pKey, pValue);
                    if ((fp = fopen(mKeyName, "rt"))) { // REG_SZ, REG_EXPAND_SZ, REG_MULTI_SZ, 
                      fgets((char *)pRet, *nSize, fp);
                      *nSize = strlen((char *)pRet);
                      fclose(fp);
                      nSts = ERROR_SUCCESS;
                    }
		    break;
      }
  
  return nSts;
}

DWORD KeyFileSetValueEx(char *pKeyRoot, char *pKey, char *pValue, HANDLE hFile, DWORD  dwType, LPBYTE pData, DWORD nSize)
{
  DWORD nSts = -1, *pn;
  char mKeyName[256];
  FILE *fp;

    switch(dwType) {
       case REG_BINARY: sprintf(mKeyName, "%s\\reg\\%s%s.0", pKeyRoot, pKey, pValue);
                        if ((fp = fopen(mKeyName, "wb"))) {
                          fwrite(pData, sizeof(char), nSize, fp);
                          fclose(fp);
                          nSts = ERROR_SUCCESS;
                        } 
		    break;
       case REG_DWORD: sprintf(mKeyName, "%s\\reg\\%s%s.1", pKeyRoot, pKey, pValue);
		               pn = (unsigned long *)pData;
                       if ((fp = fopen(mKeyName, "wt"))) {
                          fprintf(fp, "%lu", *pn);
                          fclose(fp);
                          nSts = ERROR_SUCCESS;
                       }
 	            break;
       case REG_NONE:  // 無視
		    break;
       default:     sprintf(mKeyName, "%s\\reg\\%s%s.2", pKeyRoot, pKey, pValue);
                    if ((fp = fopen(mKeyName, "wt"))) { // REG_SZ, REG_EXPAND_SZ, REG_MULTI_SZ, 
                      fputs((char *)pData, fp);
                      fclose(fp);
                      nSts = ERROR_SUCCESS;
                    }
		    break;
      }
  
  return nSts;
}

DWORD KeyFileEnumValue(char *pKeyRoot, char *pKey, DWORD nIndex, char *pValue, DWORD  *dwType, LPBYTE pRet, DWORD *nSize)
{
  DWORD nSts = ERROR_NO_MORE_ITEMS;
  char mKeyName[256];
  DWORD n = 0;
  HANDLE             hF, hFile;
  WIN32_FIND_DATA    FD;
  BOOL bFile = TRUE;


  switch(*dwType) {
       case REG_BINARY: sprintf(mKeyName, "%s\\reg\\%s*.0", pKeyRoot, pKey);
		    break;
       case REG_DWORD: sprintf(mKeyName, "%s\\reg\\%s*.1", pKeyRoot, pKey);
 	            break;
       case REG_NONE:  sprintf(mKeyName, "%s\\reg\\%s*", pKeyRoot, pKey);
		    break;
       default:     sprintf(mKeyName, "%s\\reg\\%s*.2", pKeyRoot, pKey);
		    break;
 }

 hF = FindFirstFile(mKeyName, &FD);
 if (hF != INVALID_HANDLE_VALUE) {
    bFile = TRUE;
    while (bFile) {
	  if (!(!_stricmp(FD.cFileName, ".") ||
	        !_stricmp(FD.cFileName, "..") ||
	        !_stricmp(FD.cFileName, ".lck") ||
	        strstr(FD.cFileName, ".lck") ||
			FD.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)) {
        if (n == nIndex) {
		  strcpy(pValue, FD.cFileName);
		  *(pValue+strlen(pValue)-2) = '\x0';
		  ////////////////////////////////////////////
          if (*(pValue+strlen(pValue)-1) == '0')
			*dwType = REG_BINARY;
		  else if (*(pValue+strlen(pValue)-1) == '1')
			*dwType = REG_DWORD;
		  else 
			*dwType = REG_SZ;
		  ////////////////////////////////////////////
          OpenKeyFile(pKeyRoot, pKey, pValue, &hFile);
          if (hFile) {
            nSts =  KeyFileQueryValueEx(pKeyRoot, pKey, pValue, hFile, *dwType, pRet, nSize);
            CloseHandle(hFile);
		  }
          break; 
		}
        n++;
	  }
      bFile = FindNextFile( hF, &FD);
    }; 
    FindClose( hF ); 
  }

  return (bFile ? nSts : ERROR_NO_MORE_ITEMS);
}

DWORD KeyFileEnumKey(char *pKeyRoot, char *pKey, DWORD nIndex, char *pValue, DWORD *nSize)
{
  DWORD nSts = -1;
  char mKeyName[256];
  DWORD n = 0;
  HANDLE             hF;
  WIN32_FIND_DATA    FD;
  BOOL bFile = TRUE;


  sprintf(mKeyName, "%s\\reg\\%s*", pKeyRoot, pKey);

  if (pValue)
    *pValue = '\x0';
   hF = FindFirstFile(mKeyName, &FD);
   if (hF != INVALID_HANDLE_VALUE) {
      bFile = TRUE;
      while (bFile) {
		/// ディレクトリのみ表示
		if (FD.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
	      if (!(!_stricmp(FD.cFileName, ".") ||
	            !_stricmp(FD.cFileName, ".."))) {
            if (n == nIndex) {
              if (pValue) {
                strcpy(pValue, FD.cFileName);
                *nSize = strlen(pValue);
                nSts = ERROR_SUCCESS;
			  }
              break; 
			}
            n++;
		  }
		}
        bFile = FindNextFile( hF, &FD);
	  }; 
      FindClose( hF ); 
   }
   return nSts;
}

DWORD KeyFileDeleteValue(char *pKeyRoot, char *pKey, char *pValue)
{
  DWORD nSts = -1;
  char mKeyName[256];

  sprintf(mKeyName, "%s\\reg\\%s%s.0", pKeyRoot, pKey, pValue);
  DeleteFile(mKeyName);
  sprintf(mKeyName, "%s\\reg\\%s%s.1", pKeyRoot, pKey, pValue);
  DeleteFile(mKeyName);
  sprintf(mKeyName, "%s\\reg\\%s%s.2", pKeyRoot, pKey, pValue);
  DeleteFile(mKeyName);

  return ERROR_SUCCESS;
}

#endif
