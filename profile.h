////////////////////////////////////////////////////////////
// Profile.h Copyright K.kawakami
//   Get profile key and data header.
////////////////////////////////////////////////////////////
#ifndef _PROFILE_H
#define _PROFILE_H
#include <direct.h>
#include <lmaccess.h>

#define UPDATE_20070124 // "<ドメイン名>\Domain Users"をローカルポリシーの「バッチジョブのログオン権限に定義」
#define UPDATE_20060827 // メールスプールフォルダドライブがメールボックスフォルダ・アカウントＤＢフォルダのドライブに反映されない
#define UPDATE_20051130 // マルチドメイン設定で同じＩＰアドレスの場合重複してＩＰが登録されてしまう不具合。
//#define UPDATE_20050816
#define UPDATE_20050128

//#define FREEWARE 1
//#define FOR_BRIDGEGATE // ブリッジゲートＯＥＭ版

#define QSEND
#define E_POST   // @Solomon 海外バージョン
#define V4
#define V3
#define IPv6
#define Y2038_BUG

#define DOMAIN_MAX     512    // 最大表示ドメイン数
#define DOMAIN_MAXMEM  0x4000 // レジストリ：バイナリの限界値 4096

#ifdef E_POST
  #define REGTOFILE
#endif

#ifdef V3
  #define BEGINLOW  "V3BeginLow"
  #define BEGINHIGH "V3BeginHigh"
  #define LIMITKEY  "V3LimitKey"
#else
  #define BEGINLOW  "BeginLow"
  #define BEGINHIGH "BeginHigh"
  #define LIMITKEY  "LimitKey"
#endif

#ifdef E_POST
  #define ANNOUNCE        1
  #define TRADEMARK       "E-Post "
#ifdef FOR_BRIDGEGATE
  #define SHEET_TITLE     _T("E-Post Mail Control for BrideGate")
  #define COMPO_TITLE     _T("E-Post Mail Server for BrideGate")
#else
  #define SHEET_TITLE     _T("E-Post Mail Control")
  #define COMPO_TITLE     _T("E-Post Mail Server")
#endif
  #define SMTPRS_SERVICE  "EPSTRS"
  #define SMTPDS_SERVICE  "EPSTDS"
  #define POP3_SERVICE    "EPSTPOP3S"
  #define IMAP4_SERVICE   "EPSTIMAP4S"
  #define SOFT_REG        "SOFTWARE\\EMWAC\\IMS"
  #define PRODUCTS_ROOT   "SOFTWARE\\EPOST\\IMS"
  #define SECURITY        "SOFTWARE\\EMWAC\\IMS\\Secur" //"SOFTWARE\\EPOST\\IMS\\Secur"
  #define VIRUS           "SOFTWARE\\EPOST\\IMS\\Virus"
  #define DOMAIN_REG      "SOFTWARE\\EPOST\\IMS\\Domain"
  #define DOMAIN_PASSWORD "SOFTWARE\\EMWAC\\IMS\\Domain\\Operation" //"SOFTWARE\\EPOST\\IMS\\Domain\\Operation"
  #define DOMAIN_IP_REG   "SOFTWARE\\EPOST\\IMS\\Domain\\IP"
  #define DOMAIN_SMTPIP   "SOFTWARE\\EPOST\\IMS\\Domain\\SmtpIP"
  #define DOMAIN_POP3IP   "SOFTWARE\\EPOST\\IMS\\Domain\\Pop3IP"
  #define DOMAIN_IMAP4IP  "SOFTWARE\\EPOST\\IMS\\Domain\\Imap4IP"
  #define DOMAIN_FOLDER   "SOFTWARE\\EPOST\\IMS\\Domain\\Folder"
  #define DOMAIN_ACCOUNT  "SOFTWARE\\EPOST\\IMS\\Domain\\AcountNumber"
  #define MAIL_SPOOL      "%SystemRoot%\\SYSTEM32\\E-POST\\MAIL"
  #define DEFAULT_MAIL_GROUP "IMSUsers"
#ifdef IPv6
#ifdef V3
  #define VERSION         "3.02"
#else
  #define VERSION         "2.16"
#endif
#else
  #define VERSION         "1.00"
#endif
  #define COPYRIGHT       " Copyright 2000-2004 K.Kawakami" //E-Post Co.Ltd"
#else
  //#define ANNOUNCE        1
#ifdef V3
  #define TRADEMARK       "SPA-Pro "
#ifdef FREEWARE
  #define SHEET_TITLE     _T("SPA-Pro (free) Mail Control")
  #define COMPO_TITLE     _T("SPA-Pro (free) Mail Server")
#else
  #define SHEET_TITLE     _T("SPA-Pro Mail Control")
  #define COMPO_TITLE     _T("SPA-Pro Mail Server")
#endif
  #define SMTPRS_SERVICE  "SPARS-PRO"
  #define SMTPDS_SERVICE  "SPADS-PRO"
  #define POP3_SERVICE    "SPAPOP3S-PRO"
  #define IMAP4_SERVICE   "SPAIMAP4S-PRO"
#else
  #define TRADEMARK       "SPA "
#ifdef FREEWARE
  #define SHEET_TITLE     _T("SPA (free) Mail Control")
  #define COMPO_TITLE     _T("SPA (free) Mail Server")
#else
  #define SHEET_TITLE     _T("SPA Mail Control")
  #define COMPO_TITLE     _T("SPA Mail Server")
#endif
  #define SMTPRS_SERVICE  "SPARS"
  #define SMTPDS_SERVICE  "SPADS"
  #define POP3_SERVICE    "SPAPOP3S"
  #define IMAP4_SERVICE   "SPAIMAP4S"
#endif
  #define SOFT_REG        "SOFTWARE\\EMWAC\\IMS"
#ifdef V3
  #define PRODUCTS_ROOT   "SOFTWARE\\SPA-Pro\\IMS"
  #define SECURITY        "SOFTWARE\\SPA-Pro\\IMS\\Secur"
  #define VIRUS           "SOFTWARE\\SPA-Pro\\IMS\\Virus"
  #define DOMAIN_REG      "SOFTWARE\\SPA-Pro\\IMS\\Domain"
  #define DOMAIN_PASSWORD "SOFTWARE\\SPA-Pro\\IMS\\Domain\\Operation"
  #define DOMAIN_SMTPIP   "SOFTWARE\\SPA-Pro\\IMS\\Domain\\SmtpIP"
  #define DOMAIN_POP3IP   "SOFTWARE\\SPA-Pro\\IMS\\Domain\\Pop3IP"
  #define DOMAIN_IMAP4IP  "SOFTWARE\\SPA-Pro\\IMS\\Domain\\Imap4IP"
  #define DOMAIN_FOLDER   "SOFTWARE\\SPA-Pro\\IMS\\Domain\\Folder"
  #define DOMAIN_ACCOUNT  "SOFTWARE\\SPA-Pro\\IMS\\Domain\\AcountNumber"
#else
  #define PRODUCTS_ROOT   "SOFTWARE\\SPA\\IMS"
  #define SECURITY        "SOFTWARE\\SPA\\IMS\\Secur"
  #define VIRUS           "SOFTWARE\\SPA\\IMS\\Virus"
  #define DOMAIN_REG      "SOFTWARE\\SPA\\IMS\\Domain"
  #define DOMAIN_PASSWORD "SOFTWARE\\SPA\\IMS\\Domain\\Operation"
  #define DOMAIN_SMTPIP   "SOFTWARE\\SPA\\IMS\\Domain\\SmtpIP"
  #define DOMAIN_POP3IP   "SOFTWARE\\SPA\\IMS\\Domain\\Pop3IP"
  #define DOMAIN_IMAP4IP  "SOFTWARE\\SPA\\IMS\\Domain\\Imap4IP"
  #define DOMAIN_FOLDER   "SOFTWARE\\SPA\\IMS\\Domain\\Folder"
  #define DOMAIN_ACCOUNT  "SOFTWARE\\SPA\\IMS\\Domain\\AcountNumber"
#endif
  #define MAIL_SPOOL      "%SystemRoot%\\SYSTEM32\\SPA\\MAIL"
  #define DEFAULT_MAIL_GROUP "IMSUsers"
#ifdef IPv6
#ifdef V3
  #define VERSION         "3.16"
#else
  #define VERSION         "2.29"
#endif
#else
  #define VERSION         "1.51"
#endif
  #define COPYRIGHT       " Copyright 2007 e-POST Inc. All rights reserved." //" Copyright 2000-2003 K.Kawakami"
#endif

#define EMWAC_SMTPRS_SERVICE  "SMTPRS"
#define EMWAC_SMTPDS_SERVICE  "SMTPDS"
#define EMWAC_POP3_SERVICE    "POP3S"
#define MS_SIMPLE_SMTP_SERVICE "SMTPSVC"
//#define EMWAC_SMTPRS_NAME     "IMS SMTP Receiver"
//#define EMWAC_SMTPDS_NAME     "IMS SMTP Delivry Agent"
//#define EMWAC_POP3S_NAME      "IMS POP3 Server"

#define MAIL_BOX        "%HOME%\\INETMAIL\\INBOX"
#define SMTPRS_NAME     TRADEMARK "SMTP Receiver"
#define SMTPDS_NAME     TRADEMARK "SMTP Delivry Agent"
#define POP3S_NAME      TRADEMARK "POP3 Server"
#define IMAP4S_NAME     TRADEMARK "IMAP4rev1 Server"

#define SMTP_SERVICE          SMTPRS_SERVICE
#define SOFTNAME              "SMTP Receiver"
#define SMTP_NAME             TRADEMARK SOFTNAME
#define SOFT_ALIASES_REG      SOFT_REG "\\Aliases"
#define SYSTEM_SMTPRS_REG     "SYSTEM\\CurrentControlSet\\Services\\" SMTPRS_SERVICE
#define SYSTEM_SMTPDS_REG     "SYSTEM\\CurrentControlSet\\Services\\" SMTPDS_SERVICE
#define SYSTEM_POP3_REG       "SYSTEM\\CurrentControlSet\\Services\\" POP3_SERVICE
#define SYSTEM_IMAP4_REG      "SYSTEM\\CurrentControlSet\\Services\\" IMAP4_SERVICE
//#define SYSTEM_REG_TCPIP6   "SYSTEM\\CurrentControlSet\\Services\\Tcpip6"
#define SYSTEM_SMTPRS_PARAM_REG   SYSTEM_SMTPRS_REG "\\Parameters"
#define SYSTEM_SMTPDS_PARAM_REG   SYSTEM_SMTPDS_REG "\\Parameters"
#define SYSTEM_POP3_PARAM_REG     SYSTEM_POP3_REG "\\Parameters"
#define SYSTEM_IMAP4_PARAM_REG    SYSTEM_IMAP4_REG "\\Parameters"
//#define SYSTEM_TCPIP6_PARAM_REG   SYSTEM_REG_TCPIP6 "\\Parameters"
//#define POP3_NAME            TRADEMARK SOFTNAME
//#define POP3_DEBUG_MESS      POP3_NAME " %s %s %s"

#define PROFILE_ROOT_TREE "%s\\"
#define MAX_VALUE_NAME    128

void  SPA_Encode(char *pSrc, char *pDest);
void  SPA_Decode(char *pSrc, char *pDest);
int   GetRemoteData(FILE *fp, char *pURL);
BOOL  OperationPassword(char *pDomain);
BOOL  GetMailBoxSize(char *user, BOOL bLocal, BOOL bSubLocal);
//DWORD GetAliases(LPCTSTR lpAppName);
BOOL  GetMLists(LPCTSTR lpAppName, char *uid, BOOL *bRequest);
BOOL  GetPostMaster(char *uid);
DWORD GetProfileIntEx(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nDefault);
DWORD GetProfileStringEx(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize);
DWORD GetProfileBinaryEx(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize);
void  WriteProfileIntEx(CHAR * KeyPath, CHAR * ValuePath, DWORD ValueInt);
void  WriteProfileStringEx(CHAR * KeyPath, CHAR * ValuePath, LPCTSTR ValueString);
void  WriteProfileBinaryEx(CHAR * KeyPath, CHAR * ValuePath, LPCTSTR ValueString, DWORD nSize);
void  WriteProfileStringExType(CHAR * KeyPath, CHAR * ValuePath, LPCTSTR ValueString, DWORD Length, DWORD nType);
void  DeleteProfile(HKEY hKeyRoot, CHAR * KeyPath, CHAR * ValuePath);
void  CreateProfile(HKEY hKeyRoot, CHAR * KeyPath);
#ifdef UPDATE_20050128
NET_API_STATUS NewLocalGroup(CHAR *lpszPrimaryDC, CHAR *lpszLocalGroup, CHAR *lpszLGComment);
#endif

#ifdef REGTOFILE
DWORD FileCreateKey(char *pKeyRoot, char *pKey);
DWORD OpenKeyFile(char *pKeyRoot, char *pKey, char *pValue, HANDLE *hFile);
DWORD KeyFileQueryValueEx(char *pKeyRoot, char *pKey, char *pValue, HANDLE hFile, DWORD dwType, LPBYTE pRet, DWORD *nSize);
DWORD KeyFileSetValueEx(char *pKeyRoot, char *pKey, char *pValue, HANDLE hFile, DWORD  dwType, LPBYTE pData, DWORD nSize);
DWORD KeyFileEnumValue(char *pKeyRoot, char *pKey, DWORD nIndex, char *pValue, DWORD  *dwType, LPBYTE pRet, DWORD *nSize);
DWORD KeyFileEnumKey(char *pKeyRoot, char *pKey, DWORD nIndex, char *pValue, DWORD *nSize);
DWORD KeyFileDeleteValue(char *pKeyRoot, char *pKey, char *pValue);
#endif

#endif