////////////////////////////////////////////////////////////
// Profile.c Copyright K.kawakami
// Get profile key and data.
////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "EasyWiz.h"
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include "profile.h"

#ifdef REGTOFILE
extern BOOL    nClustering;
extern char    mMailSpoolDir[];
#endif

void RestoreFile(char *pfn) {
   FILE *fp;
   int   c, i, j, n;
   CHAR  *p, *p2, *p3, *q, *q2, *pVname, *pVtype, *pValue, mKey[1024], mLine[1024], mLine2[1024];
   CHAR  mString[]="string";
   CHAR  mB[4096*3], mBin[4096];

   if ((fp = fopen(pfn, "rb"))){
     do {
       p = fgets(mLine, sizeof(mLine), fp);
	   if (p) {
		 strtok(mLine, "\r\n");
	     if (mLine[0] == '[') {  // レジストリキー
		   strtok(mLine, "]\n");
		   if ((q = strstr(mLine, "SPA-PRO")) ||
			   (q = strstr(mLine, "spa-pro"))) {
             q2 = q+7;
			 *q = '\x0';
			 sprintf(mKey, "%sEPOST%s", &mLine[1], q2);
		   } else if ((q = strstr(mLine, "SPARS-PRO")) ||
			          (q = strstr(mLine, "spars-pro"))) {
             q2 = q+9;
			 *q = '\x0';
			 sprintf(mKey, "%sEPSTRS%s", &mLine[1], q2);
		   } else if ((q = strstr(mLine, "SPADS-PRO")) ||
			          (q = strstr(mLine, "spads-pro"))) {
             q2 = q+9;
			 *q = '\x0';
			 sprintf(mKey, "%sEPSTDS%s", &mLine[1], q2);
		   } else if ((q = strstr(mLine, "SPAPOP3S-PRO")) ||
			          (q = strstr(mLine, "spapop3s-pro"))) {
             q2 = q+12;
			 *q = '\x0';
			 sprintf(mKey, "%sEPSTPOP3S%s", &mLine[1], q2);
		   } else if ((q = strstr(mLine, "SPAIMAP4S-PRO")) ||
			          (q = strstr(mLine, "spaimap4s-pro"))) {
             q2 = q+13;
			 *q = '\x0';
			 sprintf(mKey, "%sEPSTIMAP4S%s", &mLine[1], q2);
		   } else 
		     strcpy(mKey, &mLine[1]);
#ifdef REGTOFILE
		   if (nClustering && 
			   (!_strnicmp(&mKey[19], "software\\epost\\ims\\secur", 24) ||
			    !_strnicmp(&mKey[19], "software\\epost\\ims\\domain\\operation", 35))
			   ) {
             mKey[28] = 'E';
             mKey[29] = 'M';
             mKey[30] = 'W';
             mKey[31] = 'A';
             mKey[32] = 'C';
             FileCreateKey(mMailSpoolDir, &mKey[19]);
		   } else if (nClustering && !_strnicmp(&mLine[20], "software\\emwac", 14))
             FileCreateKey(mMailSpoolDir, &mKey[19]);
		   else
#endif
             CreateProfile(NULL, &mKey[19]);
		 } else {
		   if (!strstr(mLine, "V4") &&
			   !strstr(mLine, "V3") &&
			   !strstr(mLine, "ImagePath") &&
			   !strstr(mLine, "ObjectName") &&
			   !strstr(mLine, "DisplayName") &&
			   !strstr(mKey, "Enum") &&
			   !strstr(mKey, "Parameters") &&
			   !strstr(mKey, "Security") ) {
		     pVname = &mLine[1];
		     if ((pVtype = strpbrk(mLine, "="))){
		       strtok(pVname, "\"");
		       pVtype++;
		       if (*pVtype == '"') { // 文字列
			     pValue = pVtype;
			     pValue++;
			     if (*pValue == '"')
			       *pValue = '\x0';
			     else
			       strtok(pValue, "\"");
			     pVtype = mString;
			   } else {             // 数値
			     if ((pValue = strpbrk(pVtype, ":"))) {
			       *pValue = '\x0';
			      pValue++;
				 }
			   }
		       if (!_stricmp(pVtype, "string")) {
		         WriteProfileStringEx(&mKey[19], pVname, pValue); // 文字列書込み
			   } else if  (!_stricmp(pVtype, "dword")) {
		         WriteProfileIntEx(&mKey[19], pVname, (INT)strtoul( (const char *)pValue, (char **)(pValue+7), 16 ));  // 数値書込み
			   } else if  (!_strnicmp(pVtype, "hex", 3) && *pValue) {
			     p3 = strrchr(pValue, '\\'); // 続きがあるか
			     strtok(pValue, "\\\n");
			     strcpy(mB, pValue);
			     if (p3) {
			       do {
                     p2 = fgets(mLine2, sizeof(mLine2), fp);
			         if (!mLine2[0] || mLine2[0] == '\n')
				       break;
				     p3 = strrchr(mLine2, '\\'); // 続きがあるか
                     strtok(mLine2, "\\\n");
			         strcat(mB, &mLine2[2]);
				   } while(p3 && (p2 || !feof(fp)));
				 }
			     n = strlen(mB);
			     j = 0;
			     memset(mBin, 0, sizeof(mBin));
			     for (i = 0; i < n; i+=3) {
			       /// 上位
			       if (mB[i] >= '0' && mB[i] <= '9')
				     c = ((mB[i] - '0') << 4);
			       else if (mB[i] >= 'A' && mB[i] <= 'F')
				     c = ((mB[i] - 'A' + 10) << 4);
			       else if (mB[i] >= 'a' && mB[i] <= 'f')
				     c = ((mB[i] - 'a' + 10) << 4);
			       /// 下位
			       if (mB[i+1] >= '0' && mB[i+1] <= '9')
				     c += (mB[i+1] - '0');
			       else if (mB[i+1] >= 'A' && mB[i+1] <= 'F')
				     c += (mB[i+1] - 'A' + 10);
			       else if (mB[i+1] >= 'a' && mB[i+1] <= 'f')
				     c += (mB[i+1] - 'a' + 10);
			       mBin[j++] = (CHAR)c;
				 }
				 if (strstr(pVtype, "(7)"))
				   WriteProfileStringExType(&mKey[19], pVname, mBin, j, REG_MULTI_SZ);
				 else
                   WriteProfileBinaryEx(&mKey[19], pVname, mBin, j);  // バイナリ値書込み
			   }
			 }
		   }
		 }
	   }
	 } while(p || !feof(fp));
	 fclose(fp);
   }
}

void GetReg(FILE *fp, LPCTSTR lpAppName) {
  HKEY hKeyRoot = HKEY_LOCAL_MACHINE;
  HKEY hKey;
  DWORD  retCode;
  DWORD  i, j, *pn, dwIndex = 0;
  DWORD  lpcbName = 0, dwType = 0, dwcbData = 0;
  long   hFindFile;
  struct _finddata_t FindFileData;
  CHAR   c, mkey[1024], mNextKey[1024], lpName[1024];
  CHAR   mValue[4096];
#ifdef REGTOFILE
  HANDLE hFile;
#endif

  fprintf(fp, "\n[HKEY_LOCAL_MACHINE\\%s]\n", lpAppName);
  // OPEN THE KEY.
  sprintf(mkey,PROFILE_ROOT_TREE,lpAppName);
#ifdef REGTOFILE
	 if (nClustering && !_strnicmp(lpAppName, "software\\emwac", 14)) {
       retCode = 
         OpenKeyFile(mMailSpoolDir,
		             mkey,
					 "",
					 &hFile);

	 } else {
#endif
  retCode = 
      RegOpenKeyEx(hKeyRoot,       // Key handle at root level.
                   (LPCTSTR)mkey,  // Path name of child key.
                   0,              // Reserved.
                   KEY_READ,       // Requesting read access.
                   &hKey);         // Address of key to be returned.
#ifdef REGTOFILE
	 }
#endif
  if (retCode == ERROR_SUCCESS) {
	//////////////////////////////
    // READ THE VALUE DATA.
	 do {
	   lpcbName = sizeof(lpName);
	   dwcbData = sizeof(mValue);
	   lpName[0] = mValue[0] = '\x0';
#ifdef REGTOFILE
	 if (nClustering && !_strnicmp(lpAppName, "software\\emwac", 14)) {
       retCode =
		 KeyFileEnumValue(mMailSpoolDir,
		                  mkey,
						  dwIndex,
						  (char *)&lpName,
						  &dwType,
						  (LPBYTE)&mValue,
						  &dwcbData);

	 } else {
#endif
       retCode = RegEnumValue(hKey,               // 問い合わせ対象のキーのハンドル
                              dwIndex,            // 取得するべきレジストリエントリのインデックス番号
                              (LPTSTR)lpName,     // レジストリエントリ名が格納されるバッファ
                              (LPDWORD)&lpcbName, // レジストリエントリ名バッファのサイズ
                              NULL,               // 予約済み
                              &dwType,            // レジストリエントリのデータのタイプ
                              (LPBYTE)mValue,     // レジストリエントリのデータが格納されるバッファ
                              &dwcbData           // データバッファのサイズ
						      );
#ifdef REGTOFILE
	 }
#endif
	   if (retCode == ERROR_SUCCESS) {
	     switch(dwType) {
	       case REG_BINARY:
                fprintf(fp, "\"%s\"=hex:", lpName );
				j = strlen(lpName)+5;
				for (i = 0; i < dwcbData; i++) {
				  if (j > 74) {
					j = 0;
					fputs("\\\n", fp);
					if (i < dwcbData) {
					  fputs("  ", fp);
					  j += 2;
					}
				  }
				  c = (CHAR)mValue[i];
				  fprintf(fp, "%02x", (unsigned char)c);
				  j+=2;
				  if (i+1 < dwcbData) {
					fputs(",", fp);
					j++;
				  }
				}
			    // バイナリ出力
			    fputs("\n", fp);
			    break;
	       case REG_DWORD: 
			    pn = (DWORD *)&mValue;
                fprintf(fp, "\"%s\"=dword:%08x\n", lpName, *pn);
			    break;
		   case REG_NONE:  // 無視
			    break;
		   case REG_MULTI_SZ:
                fprintf(fp, "\"%s\"=hex(7):", lpName );
				j = strlen(lpName)+5;
				for (i = 0; i < dwcbData; i++) {
				  if (j > 74) {
					j = 0;
					fputs("\\\n", fp);
					if (i < dwcbData) {
					  fputs("  ", fp);
					  j += 2;
					}
				  }
				  c = (CHAR)mValue[i];
				  fprintf(fp, "%02x", (unsigned char)c);
				  j+=2;
				  if (i+1 < dwcbData) {
					fputs(",", fp);
					j++;
				  }
				}
			    // バイナリ出力
			    fputs("\n", fp);
			    break;
		   case REG_EXPAND_SZ:
		   default: // REG_SZ 
			   if (!_stricmp(lpName, "imagepath")) { // "ImagePath" の場合
				 if ((hFindFile = _findfirst(mValue, &FindFileData)) != -1L) { // ファイルが存在するか確認
                   _findclose(hFindFile);
                   fprintf(fp, "\"%s\"=\"%s\"\n", lpName, mValue);
				 }
			   } else {// "ImagePath" 以外は作成
                 fprintf(fp, "\"%s\"=\"%s\"\n", lpName, mValue);
			   }
			    break;
		 }
	   }
	   dwIndex++;
	} while (retCode != ERROR_NO_MORE_ITEMS);

	//////////////////////////////
    // READ THE KEY DATA.
	dwIndex = 0;
    do {
#ifdef REGTOFILE
	 if (nClustering && !_strnicmp(lpAppName, "software\\emwac", 14)) {
       retCode =
		 KeyFileEnumKey(mMailSpoolDir,
		                mkey,
						dwIndex, 
						(LPTSTR)lpName, 
						(unsigned long *)&lpcbName);
	 } else {
#endif
      retCode =
          RegEnumKey(hKey,	        // handle of key to query 
                     dwIndex,	    // index of subkey to query 
                     (LPTSTR)lpName, // address of buffer for subkey name  
                     (unsigned long)&lpcbName 	    // size of subkey buffer 
         );
#ifdef REGTOFILE
	 }
#endif
      if (retCode == ERROR_SUCCESS) {
		sprintf(mNextKey, "%s\\%s", lpAppName, lpName);
        GetReg(fp, mNextKey); // 次の階層を読み込み
	  }
      dwIndex++;
	} while (retCode == ERROR_SUCCESS);
#ifdef REGTOFILE
  if (nClustering && !_strnicmp(lpAppName, "software\\emwac", 14))
	CloseHandle(hFile);
  else
#endif
    RegCloseKey(hKey);
  }

}

char tochar(char *p) {
  int  i;
  char c = 0x00;
 
  for (i = 0; i < 2; i++) {
	switch (*(p+i)) {
	case '0': c |= 0x00 >> 4*i; break;
	case '1': c |= 0x10 >> 4*i; break;
	case '2': c |= 0x20 >> 4*i; break;
	case '3': c |= 0x30 >> 4*i; break;
	case '4': c |= 0x40 >> 4*i; break;
	case '5': c |= 0x50 >> 4*i; break;
	case '6': c |= 0x60 >> 4*i; break;
	case '7': c |= 0x70 >> 4*i; break;
	case '8': c |= 0x80 >> 4*i; break;
	case '9': c |= 0x90 >> 4*i; break;
	case 'a': 
	case 'A': c |= 0xA0 >> 4*i; break;
	case 'b': 
	case 'B': c |= 0xB0 >> 4*i; break;
	case 'c': 
	case 'C': c |= 0xC0 >> 4*i; break;
	case 'd': 
	case 'D': c |= 0xD0 >> 4*i; break;
	case 'e': 
	case 'E': c |= 0xE0 >> 4*i; break;
	case 'f': 
	case 'F': c |= 0xF0 >> 4*i; break;
	}
  }
  return c;
}

DWORD GetProfileIntEx(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nDefault) {
  HKEY hKeyRoot = HKEY_LOCAL_MACHINE;
  HKEY hKey;
  CHAR mkey[256];
  DWORD  cbValueName = MAX_VALUE_NAME;
  DWORD  retCode;
  DWORD  dwIndex = 0;	         // index of subkey to enumerate 
  DWORD  dwType;
  DWORD  nReturned;
  DWORD  nSize = sizeof(DWORD);
#ifdef REGTOFILE
  HANDLE hFile;
#endif

  // READ THE KEY DATA.
  nReturned = (DWORD)nDefault;
  sprintf(mkey,PROFILE_ROOT_TREE,lpAppName);
#ifdef REGTOFILE
	 if (nClustering && !_strnicmp(lpAppName, "software\\emwac", 14)) {
       retCode = 
         OpenKeyFile(mMailSpoolDir,
		             mkey,
					 (char *)lpKeyName,
					 &hFile);

	 } else {
#endif
  retCode = 
      RegOpenKeyEx(hKeyRoot,       // Key handle at root level.
                   (LPCTSTR)mkey,  // Path name of child key.
                   0,              // Reserved.
                   KEY_READ,       // Requesting read access.
                   &hKey);         // Address of key to be returned.
#ifdef REGTOFILE
	 }
#endif
  if (retCode == ERROR_SUCCESS) {
	dwType = REG_DWORD;
#ifdef REGTOFILE
	 if (nClustering && !_strnicmp(lpAppName, "software\\emwac", 14)) {
       retCode =
		 KeyFileQueryValueEx(mMailSpoolDir,
		                     mkey,
					         (char *)lpKeyName,
							 &hFile,
							 dwType,
							 (LPBYTE)&nReturned,
							 &nSize);
	     CloseHandle(hFile);
	 } else {
#endif
    retCode =
	   RegQueryValueEx(hKey,            // handle of key to query 
                      (LPSTR)lpKeyName, // address of name of value to query 
                      0,                // reserved 
                      &dwType,  // address of buffer for value type 
                      (LPBYTE)&nReturned,  // address of data buffer 
                      &nSize  // address of data buffer size 
                      ); 
    RegCloseKey(hKey);
#ifdef REGTOFILE
	 }
#endif
	if (retCode) {
      nReturned = (DWORD)nDefault;
	}
  } else 
    nReturned = (DWORD)nDefault;

  return nReturned;
} 

DWORD GetProfileStringEx(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize) {
  HKEY hKeyRoot = HKEY_LOCAL_MACHINE;
  HKEY hKey;
  CHAR mkey[256];
  DWORD  cbValueName = MAX_VALUE_NAME;
  DWORD  retCode;
  DWORD  dwIndex = 0;	         // index of subkey to enumerate 
  DWORD  dwType;
#ifdef REGTOFILE
  HANDLE hFile;
#endif

  if (strstr(lpAppName, "HKEY_CLASSES_ROOT"))
    hKeyRoot = HKEY_CLASSES_ROOT;
  // READ THE KEY DATA.
  memset(lpReturnedString, 0, nSize);
  strcpy(lpReturnedString, lpDefault);
  if (strstr(lpAppName, "HKEY_CLASSES_ROOT")) {
    sprintf(mkey,PROFILE_ROOT_TREE,(lpAppName+18));
  } else {
    sprintf(mkey,PROFILE_ROOT_TREE,lpAppName);
  }
#ifdef REGTOFILE
	 if (nClustering && !_strnicmp(lpAppName, "software\\emwac", 14)) {
       retCode = 
         OpenKeyFile(mMailSpoolDir,
		             mkey,
					 (char *)lpKeyName,
					 &hFile);

	 } else {
#endif
  retCode = 
      RegOpenKeyEx(hKeyRoot,       // Key handle at root level.
                   (LPCTSTR)mkey,  // Path name of child key.
                   0,              // Reserved.
                   KEY_READ,       // Requesting read access.
                   &hKey);         // Address of key to be returned.
#ifdef REGTOFILE
	 }
#endif
  if (retCode == ERROR_SUCCESS) {
	dwType = REG_SZ;
#ifdef REGTOFILE
	 if (nClustering && !_strnicmp(lpAppName, "software\\emwac", 14)) {
       retCode =
		 KeyFileQueryValueEx(mMailSpoolDir,
		                     mkey,
					         (char *)lpKeyName,
							 &hFile,
							 dwType,
							 (LPBYTE)lpReturnedString,
							 &nSize);
	     CloseHandle(hFile);
	 } else {
#endif
    retCode =
	   RegQueryValueEx(hKey,            // handle of key to query 
                      (LPSTR)lpKeyName, // address of name of value to query 
                      0,                // reserved 
                      &dwType,  // address of buffer for value type 
                      (LPBYTE)lpReturnedString,  // address of data buffer 
                      &nSize  // address of data buffer size 
                      ); 
    RegCloseKey(hKey);
#ifdef REGTOFILE
	 }
#endif
	if (retCode != ERROR_SUCCESS)
      strcpy(lpReturnedString, lpDefault);
  } else
    strcpy(lpReturnedString, lpDefault);

  return nSize;
}

DWORD GetProfileBinaryEx(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize) {
  HKEY hKeyRoot = HKEY_LOCAL_MACHINE;
  HKEY hKey;
  CHAR mkey[256];
  DWORD  cbValueName = MAX_VALUE_NAME;
  DWORD  retCode;
  DWORD  dwIndex = 0;	         // index of subkey to enumerate 
  DWORD  dwType;
#ifdef REGTOFILE
  HANDLE hFile;
#endif

  // READ THE KEY DATA.
  memset(lpReturnedString, 0, nSize);
  strcpy(lpReturnedString, lpDefault);
  sprintf(mkey,PROFILE_ROOT_TREE,lpAppName);
#ifdef REGTOFILE
	 if (nClustering && !_strnicmp(lpAppName, "software\\emwac", 14)) {
       retCode = 
         OpenKeyFile(mMailSpoolDir,
		             mkey,
					 (char *)lpKeyName,
					 &hFile);

	 } else {
#endif
  retCode = 
      RegOpenKeyEx(hKeyRoot,       // Key handle at root level.
                   (LPCTSTR)mkey,  // Path name of child key.
                   0,              // Reserved.
                   KEY_READ,       // Requesting read access.
                   &hKey);         // Address of key to be returned.
#ifdef REGTOFILE
	 }
#endif
  if (retCode == ERROR_SUCCESS) {
	dwType = REG_BINARY;
#ifdef REGTOFILE
	 if (nClustering && !_strnicmp(lpAppName, "software\\emwac", 14)) {
       retCode =
		 KeyFileQueryValueEx(mMailSpoolDir,
		                     mkey,
					         (char *)lpKeyName,
							 &hFile,
							 dwType,
							 (LPBYTE)lpReturnedString,
							 &nSize);
	     CloseHandle(hFile);
	 } else {
#endif
    retCode =
	     RegQueryValueEx(hKey,            // handle of key to query 
                      (LPSTR)lpKeyName, // address of name of value to query 
                      0,                // reserved 
                      &dwType,  // address of buffer for value type 
                      (LPBYTE)lpReturnedString,  // address of data buffer 
                      &nSize  // address of data buffer size 
                      ); 
    RegCloseKey(hKey);
#ifdef REGTOFILE
	 }
#endif
	if (retCode != ERROR_SUCCESS)
      strcpy(lpReturnedString, "");
  } else
    strcpy(lpReturnedString, lpDefault);

  return nSize;
}

void WriteProfileIntEx(CHAR * KeyPath, CHAR * ValuePath, DWORD ValueInt) {
  HKEY   hKeyRoot = HKEY_LOCAL_MACHINE;
  HKEY   hKey;
  DWORD  retCode;
  int    mach = 1;
  DWORD  cbValueName = 80;
  DWORD  dwIndex = 0;	         // index of subkey to enumerate 
  DWORD  lpData = 0;
  DWORD  cbData = 80;
  CHAR   RegPath[80];
#ifdef REGTOFILE
  HANDLE hFile;
#endif

  sprintf(RegPath, PROFILE_ROOT_TREE, KeyPath);
  lpData = (unsigned long)ValueInt;
  cbData = sizeof(DWORD);

#ifdef REGTOFILE
	 if (nClustering && !_strnicmp(KeyPath, "software\\emwac", 14)) {
       retCode = 
         OpenKeyFile(mMailSpoolDir,
		             RegPath,
					 (char *)ValuePath,
					 &hFile);

	 } else {
#endif
    retCode = 
      RegOpenKeyEx(hKeyRoot,    // Key handle at root level.
                   RegPath,     // Path name of child key.
                   0,           // Reserved.
                   KEY_WRITE,  // KEY_READ | KEY_WRITE, // Requesting read access.
                   &hKey);      // Address of key to be returned.
#ifdef REGTOFILE
	 }
#endif
	if (retCode == ERROR_SUCCESS) {
#ifdef REGTOFILE
	 if (nClustering && !_strnicmp(KeyPath, "software\\emwac", 14)) {
       retCode =
		 KeyFileSetValueEx(mMailSpoolDir,
		                     RegPath,
					         (char *)ValuePath,
							 &hFile,
							 REG_DWORD,	// type of value 
							 (LPBYTE)&lpData,
							 cbData);
	     CloseHandle(hFile);
	 } else {
#endif
      retCode =
         RegSetValueEx(hKey,	// handle of key to set value for  
                       ValuePath,	// address of value to set 
                       0,
                       REG_DWORD,	// type of value 
                       (CONST BYTE *)&lpData,	// address of value data 
                       (DWORD)cbData 	// size of value data 
                      );	
      RegCloseKey((HKEY)hKey);
#ifdef REGTOFILE
	 }
#endif
	}
}

void WriteProfileStringEx(CHAR * KeyPath, CHAR * ValuePath, LPCTSTR ValueString) {
  WriteProfileStringExType(KeyPath, ValuePath, ValueString, strlen(ValueString), REG_SZ);
}

void WriteProfileStringExType(CHAR * KeyPath, CHAR * ValuePath, LPCTSTR ValueString, DWORD Length, DWORD nType) {
  HKEY   hKeyRoot = HKEY_LOCAL_MACHINE;
  HKEY   hKey;
  DWORD  retCode;
  int    mach = 1;
  DWORD  cbValueName = 80;
  DWORD  dwIndex = 0;	         // index of subkey to enumerate 
  DWORD  lpData = 0;
  DWORD  cbData = 80;
  CHAR   RegPath[80];
#ifdef REGTOFILE
  HANDLE hFile;
#endif

  sprintf(RegPath, PROFILE_ROOT_TREE, KeyPath);
  lpData = (unsigned long)ValueString;
  cbData =  Length; //strlen(ValueString);

#ifdef REGTOFILE
	 if (nClustering && !_strnicmp(KeyPath, "software\\emwac", 14)) {
       retCode = 
         OpenKeyFile(mMailSpoolDir,
		             RegPath,
					 (char *)ValuePath,
					 &hFile);

	 } else {
#endif
    retCode = 
      RegOpenKeyEx(hKeyRoot,    // Key handle at root level.
                   RegPath,     // Path name of child key.
                   0,           // Reserved.
                   KEY_WRITE,   // KEY_READ | KEY_WRITE, // Requesting read access.
                   &hKey);      // Address of key to be returned.
#ifdef REGTOFILE
	 }
#endif
	if (retCode == ERROR_SUCCESS) {
#ifdef REGTOFILE
	 if (nClustering && !_strnicmp(KeyPath, "software\\emwac", 14)) {
       retCode =
		 KeyFileSetValueEx(mMailSpoolDir,
		                   RegPath,
					       ValuePath,
						   &hFile,
						   nType,
						   (LPBYTE)lpData,
						   (DWORD)cbData);
	     CloseHandle(hFile);
	 } else {
#endif
      retCode =
         RegSetValueEx(hKey,	// handle of key to set value for  
                       ValuePath,	// address of value to set 
                       0,
					   nType,	// type of value 
                       //REG_SZ,	// type of value 
                       (CONST BYTE *)lpData,	// address of value data 
                       (DWORD)cbData 	// size of value data 
                      );	
      RegCloseKey((HKEY)hKey);
#ifdef REGTOFILE
	 }
#endif
	}
}

void WriteProfileBinaryEx(CHAR * KeyPath, CHAR * ValuePath, LPCTSTR ValueString, DWORD nSize) {
  HKEY   hKeyRoot = HKEY_LOCAL_MACHINE;
  HKEY   hKey;
  DWORD  retCode;
  int    mach = 1;
  DWORD  cbValueName = 80;
  DWORD  dwIndex = 0;	         // index of subkey to enumerate 
  DWORD  lpData = 0;
  DWORD  cbData = 80;
  CHAR   RegPath[80];
#ifdef REGTOFILE
  HANDLE hFile;
#endif

  sprintf(RegPath, PROFILE_ROOT_TREE, KeyPath);
  lpData = (unsigned long)ValueString;
  cbData = nSize;

#ifdef REGTOFILE
	 if (nClustering && !_strnicmp(KeyPath, "software\\emwac", 14)) {
       retCode = 
         OpenKeyFile(mMailSpoolDir,
		             RegPath,
					 ValuePath,
					 &hFile);

	 } else {
#endif
    retCode = 
      RegOpenKeyEx(hKeyRoot,    // Key handle at root level.
                   RegPath,     // Path name of child key.
                   0,           // Reserved.
                   KEY_WRITE,   // KEY_READ | KEY_WRITE, // Requesting read access.
                   &hKey);      // Address of key to be returned.
#ifdef REGTOFILE
	 }
#endif
	if (retCode == ERROR_SUCCESS) {
#ifdef REGTOFILE
	 if (nClustering && !_strnicmp(KeyPath, "software\\emwac", 14)) {
       retCode =
		 KeyFileSetValueEx(mMailSpoolDir,
		                     RegPath,
					         ValuePath,
							 &hFile,
							 REG_BINARY,
							 (LPBYTE)lpData,
							 (DWORD)cbData);
	     CloseHandle(hFile);
	 } else {
#endif
      retCode =
         RegSetValueEx(hKey,	// handle of key to set value for  
                       ValuePath,	// address of value to set 
                       0,
                       REG_BINARY,	// type of value 
                       (CONST BYTE *)lpData,	// address of value data 
                       (DWORD)cbData 	// size of value data 
                      );	
      RegCloseKey((HKEY)hKey);
#ifdef REGTOFILE
	 }
#endif
	}
}
 
void DeleteProfile(HKEY hKeyRoot, CHAR * KeyPath, CHAR * ValuePath) {
  //HKEY   hKeyRoot = HKEY_LOCAL_MACHINE;
  HKEY   hKey;
  DWORD  retCode;
  int    mach = 1;
  DWORD  cbValueName = 80;
  DWORD  dwIndex = 0;	         // index of subkey to enumerate 
  CHAR   RegPath[80];
#ifdef REGTOFILE
  HANDLE hFile;
#endif

  sprintf(RegPath, PROFILE_ROOT_TREE, KeyPath);

#ifdef REGTOFILE
	 if (nClustering && !_strnicmp(KeyPath, "software\\emwac", 14)) {
       retCode = 
         OpenKeyFile(mMailSpoolDir,
		             RegPath,
					 ValuePath,
					 &hFile);

	 } else {
#endif
    retCode = 
      RegOpenKeyEx((hKeyRoot ? hKeyRoot : HKEY_LOCAL_MACHINE),  // Key handle at root level.
                   RegPath,     // Path name of child key.
                   0,           // Reserved.
                   KEY_SET_VALUE,  // KEY_READ | KEY_WRITE, // Requesting read access.
                   &hKey);      // Address of key to be returned.
#ifdef REGTOFILE
	 }
#endif
	if (retCode == ERROR_SUCCESS) {
#ifdef REGTOFILE
	 if (nClustering && !_strnicmp(KeyPath, "software\\emwac", 14)) {
	   retCode =
         KeyFileDeleteValue(mMailSpoolDir,
		                    RegPath,
					        ValuePath);
       CloseHandle(hFile);
	 } else {
#endif
	  retCode =
         RegDeleteValue(hKey,	    // handle of key to set value for  
                        ValuePath);	// address of value to set 
      RegCloseKey((HKEY)hKey);
#ifdef REGTOFILE
	 }
#endif
	}
}

void CreateProfile(HKEY hKeyRoot, CHAR * KeyPath) {
  //HKEY   hKeyRoot = HKEY_LOCAL_MACHINE;
  HKEY   hKey;
  DWORD  retCode;

#ifdef REGTOFILE
   if (nClustering && !_strnicmp(KeyPath, "software\\emwac", 14)) {
     retCode =
        FileCreateKey(mMailSpoolDir,
		              KeyPath);
   } else {
#endif
  retCode =
     RegCreateKey((hKeyRoot ? hKeyRoot : HKEY_LOCAL_MACHINE),  // handle of an open key 
                  KeyPath,   // address of name of subkey to open 
                  &hKey);   // address of buffer for opened handle 
  if (retCode == ERROR_SUCCESS) {
    RegCloseKey(hKey);
  }
#ifdef REGTOFILE
	 }
#endif
}
