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
#include "NetworkSetup.h"
#include <tlhelp32.h>
#include <lmcons.h>
#include <lm.h>
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
CString g_AdSelectedDnsDomain;
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

static BOOL g_IsMailServerProduct = FALSE;

static CString GetJoinedWindowsDomainName()
{
  LPWSTR joinedName = NULL;
  NETSETUP_JOIN_STATUS joinStatus = NetSetupUnknownStatus;
  CString result;
  if (NetGetJoinInformation(NULL, &joinedName, &joinStatus) == NERR_Success &&
      joinStatus == NetSetupDomainName && joinedName && joinedName[0]) {
    CHAR domain[256] = {0};
    WideCharToMultiByte(CP_ACP, 0, joinedName, -1,
      domain, sizeof(domain), NULL, NULL);
    result = domain;
  }
  if (joinedName) NetApiBufferFree(joinedName);
  return result;
}

static BOOL IsSmtpSafeAccountName(LPCTSTR account)
{
   if (!account || !account[0])
      return FALSE;
   for (LPCTSTR p = account; *p; ++p) {
      unsigned char c = (unsigned char)*p;
      if (!(isalnum(c) || c == '.' || c == '_' || c == '-'))
         return FALSE;
   }
   return TRUE;
}

static BOOL EnsureWindowsMailGroup(LPCTSTR groupName, LPCTSTR accountName,
   CString& detail)
{
   if (!groupName || !groupName[0] || !accountName || !accountName[0]) {
      detail = "ドメイン名またはWindowsユーザー名が空です";
      return FALSE;
   }
   WCHAR wideGroup[256] = {0};
   WCHAR wideAccount[UNLEN + 1] = {0};
   MultiByteToWideChar(CP_ACP, 0, groupName, -1, wideGroup, 256);
   MultiByteToWideChar(CP_ACP, 0, accountName, -1, wideAccount, UNLEN + 1);

   LPBYTE groupInfo = NULL;
   NET_API_STATUS status = NetLocalGroupGetInfo(NULL, wideGroup, 1, &groupInfo);
   BOOL created = FALSE;
   if (status == NERR_GroupNotFound) {
      LOCALGROUP_INFO_1 newGroup = {0};
      newGroup.lgrpi1_name = wideGroup;
      newGroup.lgrpi1_comment = L"E-POST mail domain users";
      DWORD parameterError = 0;
      status = NetLocalGroupAdd(NULL, 1, (LPBYTE)&newGroup, &parameterError);
      if (status != NERR_Success && status != NERR_GroupExists) {
         detail.Format("ローカルグループ %s を作成できません (%lu)",
            groupName, status);
         return FALSE;
      }
      created = TRUE;
   } else if (status != NERR_Success) {
      if (groupInfo) NetApiBufferFree(groupInfo);
      detail.Format("ローカルグループ %s を確認できません (%lu)",
         groupName, status);
      return FALSE;
   }
   if (groupInfo) NetApiBufferFree(groupInfo);

   BOOL memberExists = FALSE;
   DWORD resume = 0;
   NET_API_STATUS enumStatus;
   do {
      LPLOCALGROUP_MEMBERS_INFO_1 members = NULL;
      DWORD read = 0, total = 0;
      enumStatus = NetLocalGroupGetMembers(NULL, wideGroup, 1,
         (LPBYTE *)&members, MAX_PREFERRED_LENGTH, &read, &total, &resume);
      if (enumStatus == NERR_Success || enumStatus == ERROR_MORE_DATA) {
         for (DWORD i = 0; i < read; ++i) {
            if (members[i].lgrmi1_sidusage != SidTypeUser ||
                !members[i].lgrmi1_name)
               continue;
            WCHAR *memberName = wcsrchr(members[i].lgrmi1_name, L'\\');
            memberName = memberName ? memberName + 1 : members[i].lgrmi1_name;
            if (_wcsicmp(memberName, wideAccount) == 0) {
               memberExists = TRUE;
               break;
            }
         }
      }
      if (members) NetApiBufferFree(members);
   } while (!memberExists && enumStatus == ERROR_MORE_DATA);

   if (memberExists) {
      detail.Format("グループ %s とユーザー %s は登録済みのため、既存設定を再利用しました",
         groupName, accountName);
      return TRUE;
   }

   LOCALGROUP_MEMBERS_INFO_3 member = {0};
   member.lgrmi3_domainandname = wideAccount;
   status = NetLocalGroupAddMembers(NULL, wideGroup, 3, (LPBYTE)&member, 1);
   if (status != NERR_Success && status != ERROR_MEMBER_IN_ALIAS) {
      detail.Format("ユーザー %s をグループ %s に追加できません (%lu)",
         accountName, groupName, status);
      return FALSE;
   }
   if (created)
      detail.Format("グループ %s を作成し、ユーザー %s を追加しました",
         groupName, accountName);
   else if (status == ERROR_MEMBER_IN_ALIAS)
      detail.Format("グループ %s にユーザー %s は登録済みのため、既存設定を再利用しました",
         groupName, accountName);
   else
      detail.Format("グループ %s にユーザー %s を追加しました",
         groupName, accountName);
   return TRUE;
}

static CString GetLocalGroupTestAccount(LPCTSTR groupName)
{
   // The local-account page may leave the group text empty when the standard
   // Users group is selected.  Use that group explicitly so that SMTP
   // verification never falls back to the randomly generated Soft Account.
   LPCTSTR effectiveGroup = (groupName && groupName[0]) ? groupName : _T("Users");
   WCHAR wideGroup[256] = {0};
   MultiByteToWideChar(CP_ACP, 0, effectiveGroup, -1, wideGroup, 256);
   LPBYTE buffer = NULL;
   DWORD read = 0, total = 0, resume = 0;
   NET_API_STATUS status = NetLocalGroupGetMembers(NULL, wideGroup, 1,
      &buffer, MAX_PREFERRED_LENGTH, &read, &total, &resume);
   CString result;
   if (status == NERR_Success || status == ERROR_MORE_DATA) {
      LOCALGROUP_MEMBERS_INFO_1 *members =
         (LOCALGROUP_MEMBERS_INFO_1 *)buffer;
      for (DWORD i = 0; i < read; ++i) {
         if (members[i].lgrmi1_sidusage != SidTypeUser ||
             !members[i].lgrmi1_name)
            continue;
         CHAR account[UNLEN + 1] = {0};
         WideCharToMultiByte(CP_ACP, 0, members[i].lgrmi1_name, -1,
            account, sizeof(account), NULL, NULL);
         CHAR *separator = strrchr(account, '\\');
         LPCTSTR localName = separator ? separator + 1 : account;
         if (IsSmtpSafeAccountName(localName)) {
            result = localName;
            break;
         }
      }
   }
   if (buffer)
      NetApiBufferFree(buffer);
   if (result.IsEmpty() && _stricmp((LPCTSTR)effectiveGroup, "Users") != 0)
      return GetLocalGroupTestAccount(_T("Users"));
   return result;
}

static BOOL DetectMailServerProduct()
{
   CHAR modulePath[MAX_PATH] = {0};
   if (!GetModuleFileName(NULL, modulePath, MAX_PATH))
      return FALSE;
   CHAR *slash = strrchr(modulePath, '\\');
   if (slash)
      strcpy(slash + 1, "Epstpop3s.exe");
   else
      strcpy(modulePath, "Epstpop3s.exe");
   DWORD attributes = GetFileAttributes(modulePath);
   return attributes != INVALID_FILE_ATTRIBUTES &&
      !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

BOOL IsMailServerProduct()
{
   return g_IsMailServerProduct;
}

CString GetProductDisplayName()
{
   return g_IsMailServerProduct ? "メールサーバー" : "SMTPサーバー";
}

CString GetWizardTitleFormat()
{
   CString title;
   title.Format("EasyWiz2 - %s設定（ステップ %%s / 7）",
      (LPCTSTR)GetProductDisplayName());
   return title;
}

static BOOL CALLBACK CloseManagerWindowProc(HWND window, LPARAM processId)
{
   DWORD windowProcessId = 0;
   GetWindowThreadProcessId(window, &windowProcessId);
   if (windowProcessId == (DWORD)processId)
      PostMessage(window, WM_CLOSE, 0, 0);
   return TRUE;
}

BOOL CloseRunningManagerProcesses(CString& detail)
{
   CHAR modulePath[MAX_PATH] = {0};
   if (!GetModuleFileName(NULL, modulePath, MAX_PATH)) {
      detail = "Manager.exeの場所を取得できませんでした。";
      return FALSE;
   }
   CHAR *separator = strrchr(modulePath, '\\');
   if (separator)
      strcpy(separator + 1, "Manager.exe");
   else
      strcpy(modulePath, "Manager.exe");

   HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
   if (snapshot == INVALID_HANDLE_VALUE) {
      detail.Format("実行プロセスを確認できませんでした（エラー %lu）。",
         GetLastError());
      return FALSE;
   }

   PROCESSENTRY32 entry;
   ZeroMemory(&entry, sizeof(entry));
   entry.dwSize = sizeof(entry);
   int closedCount = 0;
   BOOL allClosed = TRUE;
   if (Process32First(snapshot, &entry)) {
      do {
         if (_stricmp(entry.szExeFile, "Manager.exe") != 0)
            continue;
         HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION |
            SYNCHRONIZE | PROCESS_TERMINATE, FALSE, entry.th32ProcessID);
         if (!process)
            continue;

         CHAR processPath[MAX_PATH] = {0};
         DWORD pathLength = MAX_PATH;
         BOOL isEasyWizManager = QueryFullProcessImageName(process, 0,
            processPath, &pathLength) &&
            _stricmp(processPath, modulePath) == 0;
         if (!isEasyWizManager) {
            CloseHandle(process);
            continue;
         }

         EnumWindows(CloseManagerWindowProc, (LPARAM)entry.th32ProcessID);
         DWORD waitResult = WaitForSingleObject(process, 5000);
         if (waitResult == WAIT_TIMEOUT) {
            if (!TerminateProcess(process, 0) ||
                WaitForSingleObject(process, 3000) != WAIT_OBJECT_0)
               allClosed = FALSE;
         } else if (waitResult != WAIT_OBJECT_0) {
            allClosed = FALSE;
         }
         if (allClosed)
            ++closedCount;
         CloseHandle(process);
      } while (Process32Next(snapshot, &entry));
   }
   CloseHandle(snapshot);

   if (!allClosed) {
      detail = "Manager.exeを終了できませんでした。手動で終了してから、もう一度［完了］を押してください。";
      return FALSE;
   }
   if (closedCount > 0)
      detail.Format("実行中のManager.exeを終了しました（%d件）。", closedCount);
   else
      detail = "Manager.exeは実行されていません。";
   return TRUE;
}

static BOOL SaveVerificationEvidence(const CString& verification,
                                     CString& savedPath, CString& errorDetail)
{
   CHAR modulePath[MAX_PATH] = {0};
   if (!GetModuleFileName(NULL, modulePath, MAX_PATH)) {
      errorDetail = "実行ファイルの場所を取得できませんでした。";
      return FALSE;
   }
   CHAR *slash = strrchr(modulePath, '\\');
   if (!slash) {
      errorDetail = "ログ保存先を決定できませんでした。";
      return FALSE;
   }
   *slash = '\0';

   CString logDirectory;
   logDirectory.Format("%s\\EasyWiz2-Logs", modulePath);
   if (!CreateDirectory(logDirectory, NULL) &&
       GetLastError() != ERROR_ALREADY_EXISTS) {
      errorDetail.Format("ログフォルダーを作成できませんでした（エラー %lu）。",
         GetLastError());
      return FALSE;
   }

   SYSTEMTIME now;
   GetLocalTime(&now);
   CString productCode = IsMailServerProduct() ? "MailServer" : "SMTPServer";
   savedPath.Format("%s\\EasyWiz2-%s-%04d%02d%02d-%02d%02d%02d-%03d.txt",
      (LPCTSTR)logDirectory, (LPCTSTR)productCode,
      now.wYear, now.wMonth, now.wDay,
      now.wHour, now.wMinute, now.wSecond, now.wMilliseconds);

   CHAR computerName[MAX_COMPUTERNAME_LENGTH + 1] = {0};
   DWORD computerLength = MAX_COMPUTERNAME_LENGTH + 1;
   if (!GetComputerName(computerName, &computerLength))
      strcpy(computerName, "unknown");
   CHAR userName[256] = {0};
   DWORD userLength = sizeof(userName);
   if (!GetUserName(userName, &userLength))
      strcpy(userName, "取得できませんでした");

   CString evidence;
   evidence.Format(
      "EasyWiz2 設定実行証跡\r\n"
      "========================================\r\n"
      "実行日時: %04d/%02d/%02d %02d:%02d:%02d.%03d\r\n"
      "コンピューター名: %s\r\n"
      "実行ユーザー: %s\r\n"
      "製品種別: %s\r\n"
      "操作記録: ユーザーがウィザードの［完了］を選択した後、設定処理を実行しました。\r\n"
      "========================================\r\n\r\n%s",
      now.wYear, now.wMonth, now.wDay,
      now.wHour, now.wMinute, now.wSecond, now.wMilliseconds,
      computerName, userName, (LPCTSTR)GetProductDisplayName(),
      (LPCTSTR)verification);

   HANDLE file = CreateFile(savedPath, GENERIC_WRITE, FILE_SHARE_READ, NULL,
      CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
   if (file == INVALID_HANDLE_VALUE) {
      errorDetail.Format("ログファイルを作成できませんでした（エラー %lu）。",
         GetLastError());
      return FALSE;
   }

   int wideLength = MultiByteToWideChar(CP_ACP, 0, evidence, -1, NULL, 0);
   WCHAR *wideText = new WCHAR[wideLength];
   MultiByteToWideChar(CP_ACP, 0, evidence, -1, wideText, wideLength);
   int utf8Length = WideCharToMultiByte(CP_UTF8, 0, wideText, wideLength - 1,
      NULL, 0, NULL, NULL);
   CHAR *utf8Text = new CHAR[utf8Length];
   WideCharToMultiByte(CP_UTF8, 0, wideText, wideLength - 1,
      utf8Text, utf8Length, NULL, NULL);
   delete [] wideText;

   const BYTE bom[] = {0xEF, 0xBB, 0xBF};
   DWORD written = 0;
   BOOL success = WriteFile(file, bom, sizeof(bom), &written, NULL) &&
      written == sizeof(bom) &&
      WriteFile(file, utf8Text, utf8Length, &written, NULL) &&
      written == (DWORD)utf8Length;
   delete [] utf8Text;
   CloseHandle(file);
   if (!success) {
      errorDetail = "ログファイルへの書き込みを完了できませんでした。";
      return FALSE;
   }
   return TRUE;
}

struct CSelectableResultWindowData
{
   CString text;
   HWND edit;
};

static LRESULT CALLBACK SelectableResultWindowProc(HWND hwnd, UINT message,
                                                   WPARAM wParam, LPARAM lParam)
{
   CSelectableResultWindowData *data =
      (CSelectableResultWindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

   if (message == WM_CREATE) {
      CREATESTRUCT *create = (CREATESTRUCT *)lParam;
      data = (CSelectableResultWindowData *)create->lpCreateParams;
      SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data);

      data->edit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", data->text,
         WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL |
         ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
         18, 18, 640, 430, hwnd, (HMENU)1001, AfxGetInstanceHandle(), NULL);
      HWND ok = CreateWindow("BUTTON", "OK",
         WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
         560, 465, 98, 30, hwnd, (HMENU)IDOK, AfxGetInstanceHandle(), NULL);
      HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
      SendMessage(data->edit, WM_SETFONT, (WPARAM)font, TRUE);
      SendMessage(ok, WM_SETFONT, (WPARAM)font, TRUE);
      SetFocus(data->edit);
      return 0;
   }

   if (message == WM_SIZE && data) {
      int width = LOWORD(lParam);
      int height = HIWORD(lParam);
      MoveWindow(data->edit, 18, 18, width - 36, height - 76, TRUE);
      HWND ok = GetDlgItem(hwnd, IDOK);
      MoveWindow(ok, width - 116, height - 48, 98, 30, TRUE);
      return 0;
   }

   if (message == WM_COMMAND && LOWORD(wParam) == IDOK) {
      DestroyWindow(hwnd);
      return 0;
   }
   if (message == WM_CLOSE) {
      DestroyWindow(hwnd);
      return 0;
   }
   return DefWindowProc(hwnd, message, wParam, lParam);
}

static void ShowSelectableResultWindow(LPCTSTR title, LPCTSTR text)
{
   static LPCTSTR className = "EasyWiz2SelectableResultWindow";
   static BOOL registered = FALSE;
   if (!registered) {
      WNDCLASS wc;
      ZeroMemory(&wc, sizeof(wc));
      wc.lpfnWndProc = SelectableResultWindowProc;
      wc.hInstance = AfxGetInstanceHandle();
      wc.hCursor = LoadCursor(NULL, IDC_ARROW);
      wc.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
      wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
      wc.lpszClassName = className;
      registered = RegisterClass(&wc) != 0;
   }

   CSelectableResultWindowData data;
   data.text = text;
   data.edit = NULL;
   HWND owner = GetActiveWindow();
   HWND hwnd = CreateWindowEx(WS_EX_DLGMODALFRAME | WS_EX_TOPMOST, className, title,
      WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
      CW_USEDEFAULT, CW_USEDEFAULT, 700, 560, owner, NULL,
      AfxGetInstanceHandle(), &data);
   if (!hwnd) {
      MessageBox(owner, text, title, MB_OK | MB_ICONINFORMATION);
      return;
   }

   RECT windowRect;
   RECT workArea;
   GetWindowRect(hwnd, &windowRect);
   SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
   int x = workArea.left + ((workArea.right - workArea.left) -
      (windowRect.right - windowRect.left)) / 2;
   int y = workArea.top + ((workArea.bottom - workArea.top) -
      (windowRect.bottom - windowRect.top)) / 2;
   SetWindowPos(hwnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
   if (owner)
      EnableWindow(owner, FALSE);
   ShowWindow(hwnd, SW_SHOW);
   UpdateWindow(hwnd);

   MSG msg;
   while (IsWindow(hwnd) && GetMessage(&msg, NULL, 0, 0) > 0) {
      if (!IsDialogMessage(hwnd, &msg)) {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
   }
   if (owner) {
      EnableWindow(owner, TRUE);
      SetActiveWindow(owner);
   }
}

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

#if _MSC_VER < 1300 && defined(_AFXDLL)
	Enable3dControls();			// 共有 DLL 内で MFC を使う場合はここをコールしてください。
#elif _MSC_VER < 1300
	Enable3dControlsStatic();	// MFC と静的にリンクする場合はここをコールしてください。
#endif

    StartSheet();
	// ダイアログが閉じられてからアプリケーションのメッセージ ポンプを開始するよりは、
	// アプリケーションを終了するために FALSE を返してください。
	return FALSE;
}

void CEasyWizApp::StartSheet()
{
   g_IsMailServerProduct = DetectMailServerProduct();
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
   CString sheetTitle;
   sheetTitle.Format("EasyWiz2 - %s設定", (LPCTSTR)GetProductDisplayName());
   CPropertySheet cPropSheet(sheetTitle);
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
  Wiz11.m_LocalUser = (CString)"";
  Wiz12.m_PDC = (CString)"";
  Wiz12.m_LocalGroup = (CString)"";
  Wiz12.m_ADUser = (CString)"";
  Wiz12.m_ADPassword = (CString)"";
  Wiz2.m_DNS1 = (CString)"";
  Wiz2.m_DNS2 = (CString)""; 
  Wiz2.m_DNS3 = (CString)"";
  Wiz3.m_Name1 = (CString)"test-sample.home.local";
  Wiz3.m_Name2 = (CString)"";
  Wiz3.m_Name3 = (CString)"";
  Wiz3.m_IP1 = GetPrimaryIPv4Address();
  Wiz3.m_IP2 = (CString)"";
  Wiz3.m_IP3 = (CString)"";
  Wiz4.m_Postmaster = (CString)"administrator@test-sample.home.local";
  //////////////////////////////////////////////////////
#ifdef E_POST
  FILE *fp;
  if ((p = strrchr(mPath, '\\'))) {
    *p = '\x0';
    sprintf(mFn, "%s\\epstms.chg", mPath);
	sprintf(mMMLISTFn, "%s\\mmlist.dat", mPath);
  } else {
    strcpy(mFn, "epstms.chg");
	strcpy(mMMLISTFn, "mmlist.dat");
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
   CString mailSpool64 = GetMailServerStringSetting64("MailSpoolDir", "");
   if (!mailSpool64.IsEmpty()) {
     strncpy(mMailSpoolDir, (LPCTSTR)mailSpool64, sizeof(mMailSpoolDir) - 1);
     mMailSpoolDir[sizeof(mMailSpoolDir) - 1] = '\0';
   }
   if (!mMailSpoolDir[0])
	 strcpy(mMailSpoolDir, mSpool);
   Wiz8.m_MailSpoolDir = (CString)mMailSpoolDir;
   Wiz8.m_Computername = (CString)"";
   ///// 製品コード取得
   nProductcode = GetMailServerDwordSetting64("Productcode",
     GetProfileIntEx(SOFT_REG, "Productcode", (int)0)); // 0:Mail Server, 1:SMTP Server
   ///// クラスタ対応モードはレジストリから取得
   nClustering = GetMailServerDwordSetting64("Clustering",
     GetProfileIntEx(SOFT_REG, "Clustering", (int)0));
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
		   MessageBox(NULL, mMess, "EasyWiz2", MB_ICONSTOP | MB_OK);
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
   if (nClustering && !_strnicmp(SOFT_REG, "software\\emwac", 14)) {
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
   if (nClustering && !_strnicmp(mReg, "software\\emwac", 14)) {
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
   if (nClustering && !_strnicmp(mReg, "software\\emwac", 14)) {
     FileCreateKey(mMailSpoolDir, mReg);
   } else {
#endif
     RegCreateKey( (HKEY) HKEY_LOCAL_MACHINE, (LPCTSTR) mReg, &hKey);
     RegCloseKey((HKEY)hKey);
#ifdef REGTOFILE
   }
   if (nClustering && !_strnicmp(DOMAIN_REG, "software\\emwac", 14)) {
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
	 //AD連携の自己送受信テストはSMTP AUTHを使わず、ローカル宛の
	 //通常受信として確認する。中継制限はE-POSTの別設定で維持される。
	 if (Wiz1.m_Sel == 2) {
	   WriteProfileIntEx(SYSTEM_SMTPRS_REG, "SMTPAUTHOnly", 0);
	   WriteProfileStringEx(SYSTEM_SMTPRS_REG, "SMTPAUTHMode", "PLAIN LOGIN CRAM-MD5");
	 }
     //VRFY,EXPNへの応答の有無
	 WriteProfileIntEx(SOFT_REG, "Vrfy", FALSE);
     /////////////////////////////////	

     CString windowsGroupDetail;
     BOOL windowsGroupReady = TRUE;
     if (Wiz1.m_Sel == 1) {
       windowsGroupReady = EnsureWindowsMailGroup(Wiz3.m_Name1,
          Wiz11.m_LocalUser, windowsGroupDetail);
       Wiz11.m_LocalGroup = Wiz3.m_Name1;
     }
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
	   // 画面では分かりやすいDNSドメイン名を選択可能にするが、E-POSTの
	   // Windowsアカウント参照には参加先のNetBIOSドメイン名を保存する。
	   CString windowsDomain = GetJoinedWindowsDomainName();
	   if (windowsDomain.IsEmpty()) windowsDomain = Wiz12.m_PDC;
       WriteProfileStringEx(SOFT_REG,"Membership", (char *)((const char *)windowsDomain)); // PDCのアカウントを有効にする。
       WriteProfileStringEx(SOFT_REG, "MailGroup", (char *)((const char *)Wiz12.m_LocalGroup));  // ローカルグループ設定
#ifdef UPDATE_20070124 // "<ドメイン名>\Domain Users"をローカルポリシーの「バッチジョブのログオン権限に定義」
	   if (Wiz12.m_PDC[0]) { // PDCにドメイン名があるなら
		 CString mDomainGroup = Wiz12.m_PDC + (CString)"\\Domain Users";
	     UserRight((char *)((const char *)mDomainGroup), NULL, TRUE);                        // 「バッチジョブによるログオン権利設定」
	   }
#endif
#ifdef UPDATE_20050128
       char mDGAccount[256];
	   sprintf(mDGAccount, "%s\\%s", (LPCSTR)Wiz12.m_PDC, (LPCSTR)Wiz12.m_LocalGroup);
	  //AfxMessageBox( mDGAccount, MB_OK);
       AddLocalGroupAccount(NULL, NULL, mDGAccount, (char *)((const char *)Wiz12.m_LocalGroup));
	   UserRight((char *)((const char *)Wiz12.m_LocalGroup), NULL, TRUE);                        // 「バッチジョブによるログオン権利設定」
#endif
	   UserRight((char *)((const char *)Wiz12.m_LocalGroup), (char *)((const char *)windowsDomain), TRUE);                        // 「バッチジョブによるログオン権利設定」
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
	   fprintf(fp, "/q%s\n", (LPCSTR)Wiz7.m_ARCFolder); ///qi:\mail\backup
	   fprintf(fp, "/s%s\n", (LPCSTR)Wiz7.m_ARCIP);     ///s192.168.1.15
	   fprintf(fp, "/n%d\n", Wiz7.m_ARCPort);   ///N25
	   fclose(fp);
	 }
#endif

     // 設定後の作業を自動化する。Mail Server製品ではPOP3/IMAPも確認する。
     CString verifyAddress = Wiz3.m_IP1.IsEmpty() ? CString("127.0.0.1") : Wiz3.m_IP1;
     CString testAddress;
     if (Wiz1.m_Sel == 0) {
       // Soft Account用のテストユーザーは、管理者アドレスと分離する。
       testAddress = GenerateRandomMailAddress(Wiz3.m_Name1);
     } else if (Wiz1.m_Sel == 1) {
       CString accountName = Wiz11.m_LocalUser;
       if (windowsGroupReady && !accountName.IsEmpty())
         testAddress.Format("%s@%s", (LPCTSTR)accountName,
            (LPCTSTR)Wiz3.m_Name1);
     } else if (Wiz1.m_Sel == 2) {
       // 認証対象としてユーザーが指定したADアカウントと、テストメールの
       // ローカル部を一致させる（ウィザード実行者とは限らない）。
       CString accountName = Wiz12.m_ADUser;
       int slash = accountName.ReverseFind('\\');
       if (slash >= 0) accountName = accountName.Mid(slash + 1);
       CString adMailDomain = Wiz3.m_Name1;
       int at = accountName.Find('@');
       if (at > 0) {
         adMailDomain = accountName.Mid(at + 1);
         accountName = accountName.Left(at);
       }
       accountName.TrimLeft();
       accountName.TrimRight();
       if (!accountName.IsEmpty())
         testAddress.Format("%s@%s", (LPCTSTR)accountName,
            (LPCTSTR)adMailDomain);
     }
     CString verification = RunMailServerVerification(verifyAddress,
        testAddress, IsMailServerProduct(), Wiz1.m_Sel,
        Wiz1.m_Sel == 2 ? (LPCTSTR)Wiz12.m_ADUser : NULL,
        Wiz1.m_Sel == 2 ? (LPCTSTR)Wiz12.m_ADPassword : NULL,
        Wiz1.m_Sel == 2 ? (LPCTSTR)Wiz12.m_PDC : NULL,
        Wiz1.m_Sel == 2 ? (LPCTSTR)Wiz12.m_LocalGroup : NULL,
        mMailbox);
     if (!Wiz12.m_ADPassword.IsEmpty()) {
       LPTSTR passwordBuffer = Wiz12.m_ADPassword.GetBuffer(Wiz12.m_ADPassword.GetLength());
       SecureZeroMemory(passwordBuffer, Wiz12.m_ADPassword.GetLength());
       Wiz12.m_ADPassword.ReleaseBuffer(0);
     }
     if (Wiz1.m_Sel == 1) {
       CString groupResult;
       groupResult.Format("[%s] Windowsメールグループ: %s\r\n",
          windowsGroupReady ? "OK" : "要確認", (LPCTSTR)windowsGroupDetail);
       int headerEnd = verification.Find("\r\n\r\n");
       if (headerEnd >= 0)
          verification.Insert(headerEnd + 4, groupResult);
       else
          verification = groupResult + verification;
     }
     CString verificationTitle;
     verificationTitle.Format("EasyWiz2 - %s設定・疎通テスト結果", (LPCTSTR)GetProductDisplayName());
     CString evidencePath;
     CString evidenceError;
     BOOL evidenceSaved = SaveVerificationEvidence(verification, evidencePath, evidenceError);
     if (evidenceSaved) {
       verification += "\r\n[ログ保存] 設定結果を保存しました:\r\n";
       verification += evidencePath;
       verification += "\r\n";
     } else {
       verification += "\r\n[要確認] 設定結果ログを保存できませんでした: ";
       verification += evidenceError;
       verification += "\r\n";
     }
     ShowSelectableResultWindow(verificationTitle, verification);
     if (evidenceSaved) {
       CString explorerArguments;
       explorerArguments.Format("/select,\"%s\"", (LPCTSTR)evidencePath);
       ShellExecute(NULL, "open", "explorer.exe", explorerArguments,
          NULL, SW_SHOWNORMAL);
     }
#ifdef REGTOFILE
  } else { // ウィザードキャンセル
     if (nClustering && !_strnicmp(PRODUCTS_ROOT, "software\\emwac", 14)) {
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
		       if (MessageBox(NULL, mMess, "EasyWiz2", MB_YESNO | MB_ICONWARNING) == IDYES)
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
		       if (MessageBox(NULL, mMess, "EasyWiz2", MB_YESNO | MB_ICONWARNING) == IDYES)
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
