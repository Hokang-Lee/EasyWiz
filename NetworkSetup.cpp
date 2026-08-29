#include "stdafx.h"
#include "profile.h"
#include "NetworkSetup.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <winsvc.h>
#include <iphlpapi.h>
#include <netfw.h>
#include <atlbase.h>
#include <shellapi.h>
#include <shlobj.h>

static CString Win32ErrorText(DWORD error)
{
    LPTSTR buffer = NULL;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error, 0, (LPTSTR)&buffer, 0, NULL);
    CString text = buffer ? buffer : "不明なエラー";
    if (buffer) LocalFree(buffer);
    text.TrimRight();
    return text;
}

static BOOL ValidateAdAccount(LPCTSTR authUser, LPCTSTR password,
    LPCTSTR adName, LPCTSTR mailGroup, CString& resolvedLogonId,
    CString& detail)
{
    resolvedLogonId.Empty();
    if (!authUser || !authUser[0] || !password || !password[0]) {
        detail = "ADユーザー名またはパスワードが入力されていません";
        return FALSE;
    }

    CString user(authUser), logonUser(authUser), logonDomain;
    int slash = user.Find('\\');
    if (slash > 0) {
        logonDomain = user.Left(slash);
        logonUser = user.Mid(slash + 1);
    } else if (user.Find('@') < 0 && adName && adName[0]) {
        CString selectedDomain(adName);
        // LogonUserのドメイン引数には通常NetBIOS名を渡す。DNS形式が
        // 選択された場合は、UPNへ変換してドメイン引数を省略する。
        if (selectedDomain.Find('.') >= 0)
            logonUser.Format("%s@%s", (LPCTSTR)user, (LPCTSTR)selectedDomain);
        else
            logonDomain = selectedDomain;
    }

    HANDLE token = NULL;
    if (!LogonUser(logonUser,
        logonDomain.IsEmpty() ? NULL : (LPCTSTR)logonDomain,
        password, LOGON32_LOGON_NETWORK, LOGON32_PROVIDER_DEFAULT, &token)) {
        DWORD error = GetLastError();
        detail.Format("AD資格情報を確認できません（認証ID: %s、エラー %lu: %s）",
            authUser, error, (LPCTSTR)Win32ErrorText(error));
        return FALSE;
    }

    // 認証済みトークンからWindowsが解決した正式なSAMアカウント名を得る。
    // 入力されたDNSドメイン名をNetBIOS名として推測してはならない。
    DWORD tokenSize = 0;
    GetTokenInformation(token, TokenUser, NULL, 0, &tokenSize);
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        PTOKEN_USER tokenUser = (PTOKEN_USER)LocalAlloc(LPTR, tokenSize);
        if (tokenUser && GetTokenInformation(token, TokenUser,
            tokenUser, tokenSize, &tokenSize)) {
            CHAR account[256] = {0};
            CHAR domain[256] = {0};
            DWORD accountSize = sizeof(account);
            DWORD tokenDomainSize = sizeof(domain);
            SID_NAME_USE accountType;
            if (LookupAccountSid(NULL, tokenUser->User.Sid, account,
                &accountSize, domain, &tokenDomainSize, &accountType)) {
                if (domain[0])
                    resolvedLogonId.Format("%s\\%s", domain, account);
                else
                    resolvedLogonId = account;
            }
        }
        if (tokenUser) LocalFree(tokenUser);
    }
    if (resolvedLogonId.IsEmpty())
        resolvedLogonId = authUser;

    if (!mailGroup || !mailGroup[0]) {
        CloseHandle(token);
        detail = "ADメールグループが指定されていません";
        return FALSE;
    }

    CString account(mailGroup);
    DWORD sidSize = 0, domainSize = 0;
    SID_NAME_USE sidType;
    LookupAccountName(NULL, account, NULL, &sidSize, NULL, &domainSize, &sidType);
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        PSID sid = (PSID)LocalAlloc(LPTR, sidSize);
        LPTSTR domain = (LPTSTR)LocalAlloc(LPTR, domainSize * sizeof(TCHAR));
        BOOL resolved = sid && domain && LookupAccountName(NULL, account, sid,
            &sidSize, domain, &domainSize, &sidType);
        BOOL member = FALSE;
        if (resolved) CheckTokenMembership(token, sid, &member);
        if (sid) LocalFree(sid);
        if (domain) LocalFree(domain);
        CloseHandle(token);
        if (!resolved) {
            detail.Format("ADメールグループ「%s」を確認できません", mailGroup);
            return FALSE;
        }
        if (!member) {
            detail.Format("%s はADメールグループ「%s」のメンバーではありません",
                authUser, mailGroup);
            return FALSE;
        }
    } else {
        CloseHandle(token);
        detail.Format("ADメールグループ「%s」を確認できません", mailGroup);
        return FALSE;
    }

    detail.Format("資格情報とADメールグループ所属を確認済み（入力ID: %s、Windows認証ID: %s、グループ: %s）",
        authUser, (LPCTSTR)resolvedLogonId, mailGroup);
    return TRUE;
}

CString GenerateRandomMailAddress(LPCTSTR domainName)
{
    GUID id = {0};
    if (FAILED(CoCreateGuid(&id))) {
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        id.Data1 = GetTickCount() ^ counter.LowPart;
        id.Data2 = (USHORT)(counter.HighPart ^ GetCurrentProcessId());
        id.Data3 = (USHORT)(GetCurrentThreadId() ^ counter.LowPart);
    }

    // 推測されやすい固定語を含めず、128bit GUIDの先頭64bitを小文字16進数化。
    CString localPart;
    localPart.Format("%08lx%04x%04x", id.Data1, id.Data2, id.Data3);
    localPart.MakeLower();
    return localPart + "@" + domainName;
}

static CString GenerateRandomPassword()
{
    GUID id = {0};
    CoCreateGuid(&id);
    CString password;
    password.Format("%08lx%04x%04x%02x%02x",
        id.Data1, id.Data2, id.Data3, id.Data4[0], id.Data4[1]);
    password.MakeLower();
    return password;
}

struct ManagerWindowSearch
{
    DWORD processId;
    HWND importDialog;
};

static BOOL CALLBACK FindManagerWindows(HWND window, LPARAM parameter)
{
    ManagerWindowSearch *search = reinterpret_cast<ManagerWindowSearch *>(parameter);
    DWORD processId = 0;
    GetWindowThreadProcessId(window, &processId);
    if (processId != search->processId || !IsWindowVisible(window))
        return TRUE;

    char className[64] = {0};
    GetClassName(window, className, sizeof(className));
    if (!strcmp(className, "#32770") &&
        (GetDlgItem(window, 0x0480) || GetDlgItem(window, 0x047C)))
        search->importDialog = window;
    return TRUE;
}

static void PumpWaitingMessages()
{
    MSG message;
    while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

struct ManagerGuidanceWindowData
{
    CString body;
    CString emphasis;
    HWND bodyWindow;
    HWND emphasisWindow;
    HFONT boldFont;
};

static LRESULT CALLBACK ManagerGuidanceWindowProc(HWND window, UINT message,
    WPARAM wParam, LPARAM lParam)
{
    ManagerGuidanceWindowData *data = reinterpret_cast<ManagerGuidanceWindowData *>(
        GetWindowLongPtr(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        CREATESTRUCT *create = reinterpret_cast<CREATESTRUCT *>(lParam);
        data = reinterpret_cast<ManagerGuidanceWindowData *>(create->lpCreateParams);
        SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(data));
    }
    if (message == WM_CREATE && data) {
        HWND icon = CreateWindow("STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_ICON,
            24, 30, 36, 36, window, NULL, AfxGetInstanceHandle(), NULL);
        SendMessage(icon, STM_SETICON,
            reinterpret_cast<WPARAM>(LoadIcon(NULL, IDI_INFORMATION)), 0);
        data->bodyWindow = CreateWindow("STATIC", data->body,
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            78, 28, 455, 174, window, NULL, AfxGetInstanceHandle(), NULL);
        data->emphasisWindow = CreateWindow("STATIC", data->emphasis,
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            78, 210, 455, 48, window, NULL, AfxGetInstanceHandle(), NULL);
        HWND ok = CreateWindow("BUTTON", "OK",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
            435, 278, 98, 32, window, reinterpret_cast<HMENU>(IDOK),
            AfxGetInstanceHandle(), NULL);
        HFONT normalFont = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        LOGFONT logFont = {0};
        GetObject(normalFont, sizeof(logFont), &logFont);
        logFont.lfWeight = FW_BOLD;
        data->boldFont = CreateFontIndirect(&logFont);
        SendMessage(data->bodyWindow, WM_SETFONT,
            reinterpret_cast<WPARAM>(normalFont), TRUE);
        SendMessage(data->emphasisWindow, WM_SETFONT,
            reinterpret_cast<WPARAM>(data->boldFont), TRUE);
        SendMessage(ok, WM_SETFONT, reinterpret_cast<WPARAM>(normalFont), TRUE);
        SetFocus(ok);
        return 0;
    }
    if (message == WM_COMMAND && LOWORD(wParam) == IDOK) {
        DestroyWindow(window);
        return 0;
    }
    if (message == WM_CLOSE) {
        DestroyWindow(window);
        return 0;
    }
    if (message == WM_DESTROY && data && data->boldFont) {
        DeleteObject(data->boldFont);
        data->boldFont = NULL;
        return 0;
    }
    return DefWindowProc(window, message, wParam, lParam);
}

static void ShowManagerGuidance(LPCTSTR title, LPCTSTR body, LPCTSTR emphasis)
{
    static LPCTSTR className = "EasyWiz2ManagerGuidanceWindow";
    static BOOL registered = FALSE;
    if (!registered) {
        WNDCLASS windowClass = {0};
        windowClass.lpfnWndProc = ManagerGuidanceWindowProc;
        windowClass.hInstance = AfxGetInstanceHandle();
        windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        windowClass.hIcon = LoadIcon(NULL, IDI_INFORMATION);
        windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
        windowClass.lpszClassName = className;
        registered = RegisterClass(&windowClass) != 0;
    }

    ManagerGuidanceWindowData data;
    data.body = body;
    data.emphasis = emphasis;
    data.bodyWindow = NULL;
    data.emphasisWindow = NULL;
    data.boldFont = NULL;
    HWND owner = GetActiveWindow();
    HWND window = CreateWindowEx(WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        className, title, WS_POPUP | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 575, 360, owner, NULL,
        AfxGetInstanceHandle(), &data);
    if (!window) {
        CString fallback(body);
        fallback += "\r\n\r\n";
        fallback += emphasis;
        MessageBox(owner, fallback, title,
            MB_OK | MB_ICONINFORMATION | MB_TOPMOST | MB_SETFOREGROUND);
        return;
    }
    RECT bounds, workArea;
    GetWindowRect(window, &bounds);
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
    int x = workArea.left + ((workArea.right - workArea.left) -
        (bounds.right - bounds.left)) / 2;
    int y = workArea.top + ((workArea.bottom - workArea.top) -
        (bounds.bottom - bounds.top)) / 2;
    SetWindowPos(window, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
    if (owner) EnableWindow(owner, FALSE);
    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);
    MSG message;
    while (IsWindow(window) && GetMessage(&message, NULL, 0, 0) > 0) {
        if (!IsDialogMessage(window, &message)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }
    if (owner) {
        EnableWindow(owner, TRUE);
        SetActiveWindow(owner);
    }
}

static BOOL FillManagerImportDialog(DWORD processId, LPCTSTR importPath)
{
    ManagerWindowSearch search = {processId, NULL};
    EnumWindows(FindManagerWindows, reinterpret_cast<LPARAM>(&search));
    if (!search.importDialog)
        return FALSE;

    // 旧形式のWindowsファイル選択ダイアログで、ファイル名欄へパスを設定。
    BOOL set = SetDlgItemText(search.importDialog, 0x0480, importPath);
    if (!set) {
        HWND fileNameCombo = GetDlgItem(search.importDialog, 0x047C);
        if (fileNameCombo)
            set = SetWindowText(fileNameCombo, importPath);
    }
    SetForegroundWindow(search.importDialog);
    return set;
}

static BOOL RunAccountManagerImport(LPCTSTR mailAddress,
    LPCTSTR suppliedPassword, BOOL adAccount, CString& detail)
{
    CString address(mailAddress);
    int at = address.Find('@');
    if (at <= 0 || at >= address.GetLength() - 1) {
        detail = "管理者メールアドレスが不正なため起動しません";
        return FALSE;
    }

    char modulePath[MAX_PATH] = {0};
    GetModuleFileName(NULL, modulePath, MAX_PATH);
    CString managerPath(modulePath);
    int separator = managerPath.ReverseFind('\\');
    if (separator >= 0)
        managerPath = managerPath.Left(separator + 1) + "Manager.exe";
    else
        managerPath = "Manager.exe";
    if (GetFileAttributes(managerPath) == INVALID_FILE_ATTRIBUTES) {
        detail = "Manager.exeがEasyWiz2と同じフォルダにありません";
        return FALSE;
    }

    CString confirmation = adAccount ?
        "既存のADユーザー用SMTP認証ファイルをManagerへ登録しますか？\r\n\r\n"
        "［はい］を選ぶと一時インポートファイルを作成してManagerを起動します。" :
        "ランダムな管理者アカウントをManagerへ登録しますか？\r\n\r\n"
        "［はい］を選ぶとインポートファイルを作成してManagerを起動します。";
    if (MessageBox(NULL, confirmation,
        adAccount ? "EasyWiz2 - AD認証ファイル登録" :
        "EasyWiz2 - 管理者アカウント登録",
        MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_SETFOREGROUND) != IDYES) {
        detail = "ユーザー操作により登録を省略";
        return FALSE;
    }

    CString account = address.Left(at);
    CString domain = address.Mid(at + 1);
    CString password = suppliedPassword && suppliedPassword[0] ?
        CString(suppliedPassword) : GenerateRandomPassword();
    char desktopPath[MAX_PATH] = {0};
    if (FAILED(SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY | CSIDL_FLAG_CREATE,
        NULL, SHGFP_TYPE_CURRENT, desktopPath))) {
        GetTempPath(MAX_PATH, desktopPath);
    }
    CString importDirectory = CString(desktopPath) + "\\EasyWiz2-Import";
    CreateDirectory(importDirectory, NULL);
    CString importPath = importDirectory + "\\" + account + "-IMAP-import.txt";

    CString record;
    // ManagerのIMAP用インポート形式: アカウント、パスワード、空欄、
    // ドメイン、空欄、プロトコル/状態フラグ5列。
    record.Format("%s\t%s\t\t%s\t\t1\t1\t0\t0\t1\n",
        (LPCTSTR)account, (LPCTSTR)password, (LPCTSTR)domain);
    HANDLE file = CreateFile(importPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_TEMPORARY, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        detail.Format("インポートファイルを作成できません (Win32 %lu)", GetLastError());
        return FALSE;
    }
    DWORD written = 0;
    BOOL wrote = WriteFile(file, (LPCSTR)record, record.GetLength(), &written, NULL);
    CloseHandle(file);
    if (!wrote || written != (DWORD)record.GetLength()) {
        DeleteFile(importPath);
        detail = "インポートファイルの書き込みに失敗";
        return FALSE;
    }

    CString guidance;
    CString guidanceEmphasis;
    if (adAccount) {
      guidance.Format(
        "Managerで次の既存ADユーザーをインポートし、SMTP認証ファイルを登録してください。\r\n\r\n"
        "アカウント: %s\r\n\r\n"
        "［アカウント］→［ユーザー］→［ユーザー インポート］を開くと、\r\n"
        "ファイル名をEasyWiz2が自動入力します。",
        (LPCTSTR)address);
      guidanceEmphasis =
        "インポートして登録後にManagerを閉じると、EasyWiz2がテスト送信を続けます。\r\n"
        "一時ファイルはManager終了後に削除されます。";
    } else {
      guidance.Format(
        "Managerで次のアカウントをインポートしてください。\r\n\r\n"
        "アカウント: %s\r\nパスワード: %s\r\n\r\n"
        "［アカウント］→［ユーザー］→［ユーザー インポート］を開くと、\r\n"
        "ファイル名をEasyWiz2が自動入力します。",
        (LPCTSTR)address, (LPCTSTR)password);
      guidanceEmphasis =
        "インポートして登録後にManagerを閉じると、EasyWiz2が登録確認とテスト送信を続けます。";
    }
    ShowManagerGuidance(
        adAccount ? "EasyWiz2 - AD認証ファイル登録" :
        "EasyWiz2 - Managerインポート", guidance, guidanceEmphasis);

    SHELLEXECUTEINFO execute = {0};
    execute.cbSize = sizeof(execute);
    execute.fMask = SEE_MASK_NOCLOSEPROCESS;
    execute.lpFile = managerPath;
    execute.nShow = SW_SHOWNORMAL;
    if (!ShellExecuteEx(&execute)) {
        DeleteFile(importPath);
        detail.Format("Manager.exeを起動できません (Win32 %lu)", GetLastError());
        return FALSE;
    }

    if (execute.hProcess) {
        WaitForInputIdle(execute.hProcess, 10000);
        DWORD managerProcessId = GetProcessId(execute.hProcess);
        BOOL pathFilled = FALSE;
        while (WaitForSingleObject(execute.hProcess, 100) == WAIT_TIMEOUT) {
            if (!pathFilled)
                pathFilled = FillManagerImportDialog(managerProcessId, importPath);
            PumpWaitingMessages();
        }
        CloseHandle(execute.hProcess);
        if (!pathFilled) {
            detail = "Managerでユーザーインポートが実行されませんでした";
            DeleteFile(importPath);
            return FALSE;
        }
    }
    DeleteFile(importPath);
    detail = "Managerを終了しました（登録結果はSMTPで確認）";
    return TRUE;
}

static BOOL DirectoryExists(const CString& path)
{
    DWORD attributes = GetFileAttributes(path);
    return attributes != INVALID_FILE_ATTRIBUTES &&
        (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

static BOOL ResolveInboxFolder(LPCTSTR mailAddress,
    LPCTSTR configuredInboxTemplate, CString& folder, CString& detail)
{
    CString address(mailAddress);
    int at = address.Find('@');
    if (at <= 0) {
        detail = "メールアドレスからアカウント名を取得できません";
        return FALSE;
    }
    CString account = address.Left(at);
    CString domain = address.Mid(at + 1);
    // ウィザードが今回実際に設定したパスを最優先する。クラスタ構成では
    // 共有設定ファイルが正で、ローカルレジストリが旧値のことがある。
    CString inboxTemplate = configuredInboxTemplate && configuredInboxTemplate[0] ?
        CString(configuredInboxTemplate) : GetMailServerStringSetting64(
            "MailInBoxDir", "C:\\mail\\inbox\\%USERNAME%");
    CString upperTemplate(inboxTemplate);
    upperTemplate.MakeUpper();
    int token = upperTemplate.Find("%USERNAME%");
    CString inboxRoot = token >= 0 ? inboxTemplate.Left(token) : inboxTemplate;
    inboxRoot.TrimRight("\\/");

    CString candidates[4];
    candidates[0] = inboxRoot + "\\" + domain + "\\" + account;
    candidates[1] = inboxRoot + "\\" + account + "\\" + domain;
    candidates[2] = inboxRoot + "\\" + account;
    candidates[3] = inboxRoot + "\\" + address;
    if (token >= 0) {
        candidates[2] = inboxTemplate.Left(token) + account +
            inboxTemplate.Mid(token + 10);
    }

    for (int attempt = 0; attempt < 60; ++attempt) {
        for (int i = 0; i < 4; ++i) {
            if (DirectoryExists(candidates[i])) {
                folder = candidates[i];
                detail.Format("受信フォルダを開きました: %s", (LPCTSTR)folder);
                return TRUE;
            }
        }
        Sleep(500);
    }

    CString domainFolder = inboxRoot + "\\" + domain;
    folder = DirectoryExists(domainFolder) ? domainFolder : inboxRoot;
    if (DirectoryExists(folder)) {
        detail.Format("アカウントフォルダが未作成のためinboxを開きました: %s",
            (LPCTSTR)folder);
    } else {
        folder.Empty();
        detail.Format("受信フォルダが見つかりません: %s", (LPCTSTR)inboxRoot);
    }
    return FALSE;
}

DWORD GetMailServerDwordSetting64(LPCTSTR valueName, DWORD defaultValue)
{
    HKEY key = NULL;
    DWORD value = defaultValue;
    DWORD type = 0;
    DWORD size = sizeof(value);
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SOFT_REG, 0,
        KEY_QUERY_VALUE | KEY_WOW64_64KEY, &key) == ERROR_SUCCESS) {
        if (RegQueryValueEx(key, valueName, NULL, &type,
            reinterpret_cast<LPBYTE>(&value), &size) != ERROR_SUCCESS ||
            type != REG_DWORD)
            value = defaultValue;
        RegCloseKey(key);
    }
    return value;
}

CString GetMailServerStringSetting64(LPCTSTR valueName, LPCTSTR defaultValue)
{
    HKEY key = NULL;
    char value[512] = {0};
    DWORD type = 0;
    DWORD size = sizeof(value) - 1;
    CString result(defaultValue);
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SOFT_REG, 0,
        KEY_QUERY_VALUE | KEY_WOW64_64KEY, &key) == ERROR_SUCCESS) {
        if (RegQueryValueEx(key, valueName, NULL, &type,
            reinterpret_cast<LPBYTE>(value), &size) == ERROR_SUCCESS &&
            (type == REG_SZ || type == REG_EXPAND_SZ)) {
            value[sizeof(value) - 1] = '\0';
            result = value;
        }
        RegCloseKey(key);
    }
    return result;
}

#ifndef PBM_SETMARQUEE
#define PBM_SETMARQUEE (WM_USER + 10)
#endif

class CVerificationProgress
{
public:
    CVerificationProgress() : m_created(FALSE) {}
    ~CVerificationProgress() { Close(); }

    BOOL Create(BOOL includeMailboxProtocols)
    {
        CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW,
            LoadCursor(NULL, IDC_WAIT), (HBRUSH)(COLOR_BTNFACE + 1), NULL);
        const DWORD style = WS_POPUP | WS_CAPTION | WS_VISIBLE;
        // Managerのインポート画面など、ユーザーが操作すべき外部画面を
        // 隠さないよう、進捗画面は常時最前面にはしない。
        const DWORD exStyle = WS_EX_DLGMODALFRAME;
        const int clientWidth = 430;
        const int clientHeight = 125;
        CRect windowRect(0, 0, clientWidth, clientHeight);
        AdjustWindowRectEx(&windowRect, style, FALSE, exStyle);
        int width = windowRect.Width();
        int height = windowRect.Height();
        int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
        int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
        CString progressTitle = includeMailboxProtocols ?
            "EasyWiz2 - メールサーバーを確認しています" :
            "EasyWiz2 - SMTPサーバーを確認しています";
        if (!m_window.CreateEx(exStyle, className, progressTitle,
            style, x, y, width, height, NULL, 0))
            return FALSE;

        m_status.Create("設定を反映しています...", WS_CHILD | WS_VISIBLE | SS_LEFT,
            CRect(22, 24, clientWidth - 22, 52), &m_window);
        m_progress.Create(WS_CHILD | WS_VISIBLE | PBS_MARQUEE,
            CRect(22, 66, clientWidth - 22, 92), &m_window, 1);
        HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        m_status.SendMessage(WM_SETFONT, (WPARAM)font, TRUE);
        m_progress.SendMessage(PBM_SETMARQUEE, TRUE, 35);
        m_created = TRUE;
        PumpMessages();
        return TRUE;
    }

    void SetStatus(LPCTSTR text)
    {
        if (!m_created) return;
        m_status.SetWindowText(text);
        m_window.SetForegroundWindow();
        PumpMessages();
    }

    void Close()
    {
        if (!m_created) return;
        m_progress.SendMessage(PBM_SETMARQUEE, FALSE, 0);
        m_window.DestroyWindow();
        m_created = FALSE;
    }

private:
    void PumpMessages()
    {
        MSG message;
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        m_window.UpdateWindow();
    }

    CWnd m_window;
    CStatic m_status;
    CProgressCtrl m_progress;
    BOOL m_created;
};

static CString HResultText(HRESULT hr)
{
    CString text;
    text.Format("0x%08lX", (DWORD)hr);
    return text;
}

static BOOL CopyRegistryValuesTo64BitView(LPCTSTR keyPath, CString& detail)
{
    SYSTEM_INFO systemInfo = {0};
    GetNativeSystemInfo(&systemInfo);
    if (systemInfo.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_AMD64 &&
        systemInfo.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_ARM64) {
        detail = "32bit OSのため同期不要";
        return TRUE;
    }

    HKEY source = NULL;
    HKEY destination = NULL;
    LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyPath, 0,
        KEY_QUERY_VALUE | KEY_WOW64_32KEY, &source);
    if (result != ERROR_SUCCESS) {
        // クラスタリング構成では基本設定が共有ファイルまたは64bit側だけに
        // 存在する。64bitキーが有効なら32bitキー欠如はエラーにしない。
        HKEY existing64 = NULL;
        LONG result64 = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyPath, 0,
            KEY_QUERY_VALUE | KEY_WOW64_64KEY, &existing64);
        if (result64 == ERROR_SUCCESS) {
            RegCloseKey(existing64);
            detail = "64bit設定を使用（32bit設定は不要）";
            return TRUE;
        }
        detail.Format("32bit設定を開けません (%ld)", result);
        return FALSE;
    }
    result = RegCreateKeyEx(HKEY_LOCAL_MACHINE, keyPath, 0, NULL, 0,
        KEY_SET_VALUE | KEY_WOW64_64KEY, NULL, &destination, NULL);
    if (result == ERROR_SUCCESS) {
        DWORD valueCount = 0;
        DWORD maxNameLength = 0;
        DWORD maxDataLength = 0;
        result = RegQueryInfoKey(source, NULL, NULL, NULL, NULL, NULL, NULL,
            &valueCount, &maxNameLength, &maxDataLength, NULL, NULL);
        if (result == ERROR_SUCCESS) {
            char *name = (char *)malloc(maxNameLength + 2);
            BYTE *data = (BYTE *)malloc(maxDataLength + 2);
            if (!name || !data) {
                result = ERROR_NOT_ENOUGH_MEMORY;
            } else {
                for (DWORD index = 0; index < valueCount; ++index) {
                    DWORD nameLength = maxNameLength + 1;
                    DWORD dataLength = maxDataLength + 1;
                    DWORD type = 0;
                    result = RegEnumValue(source, index, name, &nameLength, NULL,
                        &type, data, &dataLength);
                    if (result != ERROR_SUCCESS) break;
                    result = RegSetValueEx(destination, name, 0, type, data, dataLength);
                    if (result != ERROR_SUCCESS) break;
                }
            }
            if (name) free(name);
            if (data) free(data);
        }
    }
    if (destination) RegCloseKey(destination);
    RegCloseKey(source);

    if (result != ERROR_SUCCESS) {
        detail.Format("64bit設定への同期に失敗 (%ld)", result);
        return FALSE;
    }
    detail = "32bit/64bit設定値を同期済み";
    return TRUE;
}

CString GetPrimaryIPv4Address()
{
    ULONG size = 16 * 1024;
    PIP_ADAPTER_ADDRESSES adapters = (PIP_ADAPTER_ADDRESSES)malloc(size);
    if (!adapters) return "";
    ULONG result = GetAdaptersAddresses(AF_INET,
        GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER,
        NULL, adapters, &size);
    if (result == ERROR_BUFFER_OVERFLOW) {
        free(adapters);
        adapters = (PIP_ADAPTER_ADDRESSES)malloc(size);
        if (!adapters) return "";
        result = GetAdaptersAddresses(AF_INET,
            GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER,
            NULL, adapters, &size);
    }

    CString address;
    if (result == NO_ERROR) {
        for (PIP_ADAPTER_ADDRESSES adapter = adapters; adapter; adapter = adapter->Next) {
            if (adapter->OperStatus != IfOperStatusUp ||
                adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK ||
                adapter->IfType == IF_TYPE_TUNNEL)
                continue;
            for (PIP_ADAPTER_UNICAST_ADDRESS item = adapter->FirstUnicastAddress; item; item = item->Next) {
                if (!item->Address.lpSockaddr || item->Address.lpSockaddr->sa_family != AF_INET)
                    continue;
                char text[INET_ADDRSTRLEN] = {0};
                sockaddr_in *ipv4 = (sockaddr_in *)item->Address.lpSockaddr;
                if (InetNtopA(AF_INET, &ipv4->sin_addr, text, sizeof(text))) {
                    address = text;
                    break;
                }
            }
            if (!address.IsEmpty()) break;
        }
    }
    free(adapters);
    return address;
}

static HRESULT AddFirewallRule(LPCTSTR name, long port)
{
    CComPtr<INetFwPolicy2> policy;
    CComPtr<INetFwRule> rule;
    HRESULT hr = policy.CoCreateInstance(__uuidof(NetFwPolicy2));
    if (FAILED(hr)) return hr;
    hr = rule.CoCreateInstance(__uuidof(NetFwRule));
    if (FAILED(hr)) return hr;

    CString portText;
    portText.Format("%ld", port);
    CComBSTR ruleName(name);
    CComBSTR ports(portText);
    CComBSTR group(L"EasyWiz2 Mail Server");

    rule->put_Name(ruleName);
    rule->put_Description(CComBSTR(L"EasyWiz2がサーバー設定時に作成した受信規則"));
    rule->put_Protocol(NET_FW_IP_PROTOCOL_TCP);
    rule->put_LocalPorts(ports);
    rule->put_Direction(NET_FW_RULE_DIR_IN);
    rule->put_Action(NET_FW_ACTION_ALLOW);
    rule->put_Profiles(NET_FW_PROFILE2_DOMAIN | NET_FW_PROFILE2_PRIVATE);
    rule->put_Grouping(group);
    rule->put_Enabled(VARIANT_TRUE);

    CComPtr<INetFwRules> rules;
    hr = policy->get_Rules(&rules);
    if (FAILED(hr)) return hr;
    rules->Remove(ruleName); // 再実行時は同名規則を置き換える。
    return rules->Add(rule);
}

static BOOL RestartServiceForSettings(LPCTSTR serviceName, CString& detail)
{
    SC_HANDLE manager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!manager) {
        detail.Format("サービス管理を開けません (%lu)", GetLastError());
        return FALSE;
    }
    SC_HANDLE service = OpenService(manager, serviceName,
        SERVICE_START | SERVICE_STOP | SERVICE_QUERY_STATUS);
    if (!service) {
        detail.Format("サービス %s が見つかりません (%lu)", serviceName, GetLastError());
        CloseServiceHandle(manager);
        return FALSE;
    }

    SERVICE_STATUS_PROCESS status = {0};
    DWORD needed = 0;
    BOOL ok = QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO,
        (LPBYTE)&status, sizeof(status), &needed);
    if (ok && status.dwCurrentState == SERVICE_RUNNING) {
        SERVICE_STATUS stopStatus = {0};
        if (!ControlService(service, SERVICE_CONTROL_STOP, &stopStatus)) {
            detail.Format("サービス %s を停止できません (%lu)", serviceName, GetLastError());
            CloseServiceHandle(service);
            CloseServiceHandle(manager);
            return FALSE;
        }
        for (int i = 0; i < 120; ++i) {
            Sleep(500);
            if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO,
                (LPBYTE)&status, sizeof(status), &needed)) break;
            if (status.dwCurrentState == SERVICE_STOPPED) break;
        }
        if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO,
            (LPBYTE)&status, sizeof(status), &needed) ||
            status.dwCurrentState != SERVICE_STOPPED) {
            detail.Format("サービス %s の停止完了を確認できません（状態 %lu）",
                serviceName, status.dwCurrentState);
            CloseServiceHandle(service);
            CloseServiceHandle(manager);
            return FALSE;
        }
    }

    ok = QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO,
        (LPBYTE)&status, sizeof(status), &needed);
    if (ok && status.dwCurrentState != SERVICE_RUNNING) {
        if (!StartService(service, 0, NULL) && GetLastError() != ERROR_SERVICE_ALREADY_RUNNING) {
            detail.Format("サービス %s を開始できません (%lu)", serviceName, GetLastError());
            CloseServiceHandle(service);
            CloseServiceHandle(manager);
            return FALSE;
        }
        for (int i = 0; i < 120; ++i) {
            Sleep(500);
            if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO,
                (LPBYTE)&status, sizeof(status), &needed)) break;
            if (status.dwCurrentState == SERVICE_RUNNING) break;
            if (status.dwCurrentState == SERVICE_STOPPED) break;
        }
    }
    ok = QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO,
        (LPBYTE)&status, sizeof(status), &needed) && status.dwCurrentState == SERVICE_RUNNING;
    if (ok)
        detail = "設定を反映して再起動済み";
    else
        detail.Format("再起動を確認できません（状態 %lu、サービス終了コード %lu）",
            status.dwCurrentState, status.dwWin32ExitCode);
    CloseServiceHandle(service);
    CloseServiceHandle(manager);
    return ok;
}

static SOCKET ConnectServer(LPCTSTR address, u_short port)
{
    SOCKET socketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketHandle == INVALID_SOCKET) return INVALID_SOCKET;
    // SMTP Receiverは接続元の逆引き等で挨拶応答に時間が掛かる場合がある。
    DWORD timeout = 30000;
    setsockopt(socketHandle, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
    setsockopt(socketHandle, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout));

    sockaddr_in target = {0};
    target.sin_family = AF_INET;
    target.sin_port = htons(port);
    if (InetPtonA(AF_INET, address, &target.sin_addr) != 1 ||
        connect(socketHandle, (sockaddr *)&target, sizeof(target)) == SOCKET_ERROR) {
        closesocket(socketHandle);
        return INVALID_SOCKET;
    }
    return socketHandle;
}

static BOOL ReceivePositiveReply(SOCKET socketHandle, CString& reply)
{
    char buffer[1024] = {0};
    int received = recv(socketHandle, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
        if (received == 0)
            reply = "接続先から切断されました";
        else
            reply.Format("応答待ちタイムアウトまたは受信エラー (Winsock %d)", WSAGetLastError());
        return FALSE;
    }
    buffer[received] = 0;
    reply = buffer;
    reply.TrimRight();
    return buffer[0] == '2' || buffer[0] == '3' || buffer[0] == '+' || buffer[0] == '*';
}

static BOOL SendSmtpCommand(SOCKET socketHandle, const CString& command, CString& reply)
{
    CString wire = command + "\r\n";
    if (send(socketHandle, wire, wire.GetLength(), 0) != wire.GetLength()) {
        reply = "送信に失敗";
        return FALSE;
    }
    return ReceivePositiveReply(socketHandle, reply);
}

static BOOL SendSmtpData(SOCKET socketHandle, const CString& message, CString& reply)
{
    // DATA終端の「.」は本文ではなくSMTPプロトコル上の終端記号として送る。
    CString wire = message + "\r\n.\r\n";
    if (send(socketHandle, wire, wire.GetLength(), 0) != wire.GetLength()) {
        reply = "メール本文の送信に失敗";
        return FALSE;
    }
    return ReceivePositiveReply(socketHandle, reply);
}

static CString FormatRfc2822Date()
{
    static const char *weekdays[] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };
    static const char *months[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    SYSTEMTIME local = {0};
    GetLocalTime(&local);
    TIME_ZONE_INFORMATION zone = {0};
    DWORD zoneState = GetTimeZoneInformation(&zone);
    LONG bias = zone.Bias;
    if (zoneState == TIME_ZONE_ID_DAYLIGHT)
        bias += zone.DaylightBias;
    else if (zoneState == TIME_ZONE_ID_STANDARD)
        bias += zone.StandardBias;
    LONG offset = -bias;
    char sign = offset < 0 ? '-' : '+';
    if (offset < 0) offset = -offset;
    CString value;
    value.Format("%s, %02u %s %04u %02u:%02u:%02u %c%02ld%02ld",
        weekdays[local.wDayOfWeek], local.wDay,
        months[local.wMonth - 1], local.wYear,
        local.wHour, local.wMinute, local.wSecond,
        sign, offset / 60, offset % 60);
    return value;
}

static CString Base64Encode(LPCTSTR value)
{
    static const char alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const unsigned char *source = (const unsigned char *)(value ? value : "");
    int length = value ? strlen(value) : 0;
    CString result;
    for (int i = 0; i < length; i += 3) {
        unsigned long block = ((unsigned long)source[i]) << 16;
        int remaining = length - i;
        if (remaining > 1) block |= ((unsigned long)source[i + 1]) << 8;
        if (remaining > 2) block |= source[i + 2];
        result += alphabet[(block >> 18) & 0x3f];
        result += alphabet[(block >> 12) & 0x3f];
        result += remaining > 1 ? alphabet[(block >> 6) & 0x3f] : '=';
        result += remaining > 2 ? alphabet[block & 0x3f] : '=';
    }
    return result;
}

static CString Base64EncodeBytes(const unsigned char *source, int length)
{
    static const char alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    CString result;
    for (int i = 0; i < length; i += 3) {
        unsigned long block = ((unsigned long)source[i]) << 16;
        int remaining = length - i;
        if (remaining > 1) block |= ((unsigned long)source[i + 1]) << 8;
        if (remaining > 2) block |= source[i + 2];
        result += alphabet[(block >> 18) & 0x3f];
        result += alphabet[(block >> 12) & 0x3f];
        result += remaining > 1 ? alphabet[(block >> 6) & 0x3f] : '=';
        result += remaining > 2 ? alphabet[block & 0x3f] : '=';
    }
    return result;
}

static BOOL ProbeProtocol(LPCTSTR address, u_short port, CString& detail)
{
    SOCKET socketHandle = ConnectServer(address, port);
    if (socketHandle == INVALID_SOCKET) {
        detail.Format("%s:%u に接続できません", address, port);
        return FALSE;
    }
    BOOL ok = ReceivePositiveReply(socketHandle, detail);
    closesocket(socketHandle);
    return ok;
}

static BOOL SendTestMail(LPCTSTR address, LPCTSTR recipient, CString& detail,
    BOOL& recipientNotRegistered, LPCTSTR authUser, LPCTSTR authPassword)
{
    recipientNotRegistered = FALSE;
    SOCKET socketHandle = ConnectServer(address, 25);
    if (socketHandle == INVALID_SOCKET) {
        detail = "SMTPポートへ接続できません";
        return FALSE;
    }

    CString reply;
    BOOL ok = ReceivePositiveReply(socketHandle, reply);
    char computer[MAX_COMPUTERNAME_LENGTH + 1] = "localhost";
    DWORD computerLength = MAX_COMPUTERNAME_LENGTH + 1;
    GetComputerName(computer, &computerLength);

    if (ok) ok = SendSmtpCommand(socketHandle, CString("EHLO ") + computer, reply);
    if (ok && authUser && authUser[0]) {
        if (!authPassword || !authPassword[0]) {
            detail = "SMTP認証パスワードが入力されていません";
            closesocket(socketHandle);
            return FALSE;
        }
        // Windows/ADアカウント連携では、E-POSTへ RFC 4616 の
        // AUTH PLAIN（authzid省略）で完全なメールアドレスとパスワードを渡す。
        int userLength = strlen(authUser);
        int passwordLength = strlen(authPassword);
        int plainLength = 1 + userLength + 1 + passwordLength;
        unsigned char *plain = new unsigned char[plainLength];
        plain[0] = 0;
        memcpy(plain + 1, authUser, userLength);
        plain[1 + userLength] = 0;
        memcpy(plain + 1 + userLength + 1, authPassword, passwordLength);
        CString encodedPlain = Base64EncodeBytes(plain, plainLength);
        SecureZeroMemory(plain, plainLength);
        delete [] plain;

        ok = SendSmtpCommand(socketHandle,
            CString("AUTH PLAIN ") + encodedPlain, reply);
        // サーバーが初期応答を別行で要求する実装にも対応する。
        if (ok && reply.Left(3) == "334")
            ok = SendSmtpCommand(socketHandle, encodedPlain, reply);
        if (!encodedPlain.IsEmpty()) {
            LPTSTR secret = encodedPlain.GetBuffer(encodedPlain.GetLength());
            SecureZeroMemory(secret, encodedPlain.GetLength());
            encodedPlain.ReleaseBuffer(0);
        }
        if (!ok) {
            detail.Format("SMTP認証エラー（PLAIN、認証ID: %s）: %s",
                authUser, (LPCTSTR)reply);
            CString ignored;
            SendSmtpCommand(socketHandle, "QUIT", ignored);
            closesocket(socketHandle);
            return FALSE;
        }
    }
    BOOL usedEmptyReturnPath = FALSE;
    if (ok) {
        // 通常は管理者アドレスを送信元にする。旧版E-POSTがローカルアカウント
        // 未作成を理由に拒否した場合は、RFC準拠の空Return-Pathで再試行する。
        if (!SendSmtpCommand(socketHandle, CString("MAIL FROM:<") + recipient + ">", reply)) {
            CString resetReply;
            if (SendSmtpCommand(socketHandle, "RSET", resetReply) &&
                SendSmtpCommand(socketHandle, "MAIL FROM:<>", reply)) {
                usedEmptyReturnPath = TRUE;
            } else {
                ok = FALSE;
            }
        }
    }
    if (ok) {
        ok = SendSmtpCommand(socketHandle, CString("RCPT TO:<") + recipient + ">", reply);
        if (!ok) {
            CString lowerReply(reply);
            lowerReply.MakeLower();
            recipientNotRegistered = lowerReply.Find("user unknown") >= 0 ||
                lowerReply.Find("unknown user") >= 0 ||
                lowerReply.Find("no such user") >= 0;
        }
    }
    if (ok) ok = SendSmtpCommand(socketHandle, "DATA", reply);
    if (ok) {
        CString message;
        CString sentDate = FormatRfc2822Date();
        message.Format(
            "From: EasyWiz2 <%s>\r\nTo: <%s>\r\nSubject: EasyWiz2 mail server test\r\n"
            "Date: %s\r\nMIME-Version: 1.0\r\n"
            "Content-Type: text/plain; charset=us-ascii\r\n"
            "Content-Transfer-Encoding: 7bit\r\nX-Mailer: EasyWiz2\r\n\r\n"
            "EasyWiz2 successfully completed the mail server self-delivery test.\r\n\r\n"
            "Test account: %s\r\n"
            "Test time: %s\r\n\r\n"
            "No action is required.\r\n",
            recipient, recipient, (LPCTSTR)sentDate,
            recipient, (LPCTSTR)sentDate);
        ok = SendSmtpData(socketHandle, message, reply);
    }
    CString ignored;
    SendSmtpCommand(socketHandle, "QUIT", ignored);
    closesocket(socketHandle);
    if (ok)
        detail = usedEmptyReturnPath ?
            "テストメールを受け付けました（空Return-Pathを使用）" :
            "テストメールを受け付けました";
    else
        detail = CString("SMTPエラー: ") + reply;
    return ok;
}

static void AppendResult(CString& report, LPCTSTR label, BOOL ok, const CString& detail)
{
    CString line;
    line.Format("%s %s: %s\r\n", ok ? "[OK]" : "[要確認]", label, (LPCTSTR)detail);
    report += line;
}

static void AppendSkipped(CString& report, LPCTSTR label, const CString& detail)
{
    CString line;
    line.Format("[スキップ] %s: %s\r\n", label, (LPCTSTR)detail);
    report += line;
}

CString RunMailServerVerification(LPCTSTR serverAddress, LPCTSTR testAddress,
    BOOL includeMailboxProtocols, int accountManagementMode,
    LPCTSTR smtpAuthUser, LPCTSTR smtpAuthPassword,
    LPCTSTR activeDirectoryName, LPCTSTR activeDirectoryMailGroup,
    LPCTSTR configuredInboxTemplate)
{
    CString managerDetail;
    BOOL managerRan = FALSE;
    CVerificationProgress progress;
    progress.Create(includeMailboxProtocols);
    CString report = includeMailboxProtocols ?
        "メールサーバー 設定結果\r\n\r\n" :
        "SMTPサーバー 設定結果\r\n\r\n";
    if (accountManagementMode == 1) {
        if (testAddress && testAddress[0]) {
            CString accountDetail;
            accountDetail.Format("SMTPで登録状態を確認します: %s", testAddress);
            AppendResult(report, "Windowsアカウント確認", TRUE, accountDetail);
        } else {
            AppendSkipped(report, "Windowsアカウント確認",
                "選択グループにSMTP確認に使用できる英数字のローカルユーザーがいません");
        }
    }
    BOOL adAccountReady = TRUE;
    CString resolvedAdLogonId;
    if (accountManagementMode == 2) {
        CString accountDetail;
        adAccountReady = ValidateAdAccount(smtpAuthUser, smtpAuthPassword,
            activeDirectoryName, activeDirectoryMailGroup,
            resolvedAdLogonId, accountDetail);
        AppendResult(report, "ADアカウント事前確認", adAccountReady, accountDetail);
        CString authSetting;
        CString windowsDomain = resolvedAdLogonId;
        int domainSeparator = windowsDomain.Find('\\');
        if (domainSeparator > 0) windowsDomain = windowsDomain.Left(domainSeparator);
        authSetting.Format("ADアカウント参照を使用（AD参照: %s、ローカル宛自己送受信はSMTP AUTHなし）",
            (LPCTSTR)windowsDomain);
        AppendResult(report, "SMTP認証設定", TRUE, authSetting);
    }
    if (accountManagementMode == 0) {
        managerRan = RunAccountManagerImport(testAddress, NULL, FALSE, managerDetail);
        if (managerRan)
            AppendResult(report, "管理者アカウント登録", TRUE, managerDetail);
        else
            AppendSkipped(report, "管理者アカウント登録", managerDetail);
    } else if (accountManagementMode == 2 && adAccountReady) {
        AppendSkipped(report, "Managerインポート",
            "AD連携は既存ADアカウントの自己送受信で確認するため不要です");
    } else if (accountManagementMode == 1) {
        AppendSkipped(report, "Managerインポート",
            "Windows Serverローカルアカウント連携のため、Managerインポートは不要です");
    } else {
        AppendSkipped(report, "Managerインポート",
            "ADアカウント事前確認に失敗しましたが、AD連携では使用しません");
    }
    HRESULT initResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    BOOL uninitialize = SUCCEEDED(initResult);
    CString inboxFolderToOpen;

    CString detail;
    progress.SetStatus("x64版メールサービスへ設定を同期しています...");
    BOOL softSync = CopyRegistryValuesTo64BitView(SOFT_REG, detail);
    AppendResult(report, "x64基本設定", softSync, detail);
    BOOL productSync = TRUE;
    const char *domainKeys[] = {
        DOMAIN_SMTPIP, DOMAIN_POP3IP, DOMAIN_IMAP4IP, DOMAIN_FOLDER, DOMAIN_ACCOUNT
    };
    for (int keyIndex = 0; keyIndex < 5; ++keyIndex) {
        CString keyDetail;
        if (!CopyRegistryValuesTo64BitView(domainKeys[keyIndex], keyDetail)) {
            productSync = FALSE;
            detail = keyDetail;
            break;
        }
        detail = keyDetail;
    }
    AppendResult(report, "x64ドメイン設定", productSync, detail);

    struct FirewallPort { LPCTSTR name; long port; } ports[] = {
        { _T("EasyWiz2 SMTP (TCP 25)"), 25 },
        { _T("EasyWiz2 POP3 (TCP 110)"), 110 },
        { _T("EasyWiz2 IMAP (TCP 143)"), 143 }
    };
    int firewallCount = includeMailboxProtocols ? 3 : 1;
    progress.SetStatus("Windows Firewall を設定しています...");
    for (int i = 0; i < firewallCount; ++i) {
        HRESULT hr = AddFirewallRule(ports[i].name, ports[i].port);
        AppendResult(report, ports[i].name, SUCCEEDED(hr), SUCCEEDED(hr) ? CString("許可済み") : HResultText(hr));
    }

    progress.SetStatus("SMTP受信サービスへ設定を反映しています...");
    BOOL smtpReceiver = RestartServiceForSettings(SMTPRS_SERVICE, detail);
    AppendResult(report, "SMTP受信サービス", smtpReceiver, detail);
    progress.SetStatus("SMTP配送サービスへ設定を反映しています...");
    BOOL smtpDelivery = RestartServiceForSettings(SMTPDS_SERVICE, detail);
    AppendResult(report, "SMTP配送サービス", smtpDelivery, detail);
    if (includeMailboxProtocols) {
        progress.SetStatus("POP3サービスへ設定を反映しています...");
        BOOL pop = RestartServiceForSettings(POP3_SERVICE, detail);
        AppendResult(report, "POP3サービス", pop, detail);
        progress.SetStatus("IMAPサービスへ設定を反映しています...");
        BOOL imap = RestartServiceForSettings(IMAP4_SERVICE, detail);
        AppendResult(report, "IMAPサービス", imap, detail);
    }

    WSADATA winsock = {0};
    if (WSAStartup(MAKEWORD(2, 2), &winsock) == 0) {
        progress.SetStatus("SMTPの応答を確認しています...");
        BOOL smtp = ProbeProtocol(serverAddress, 25, detail);
        AppendResult(report, "SMTP疎通", smtp, detail);
        if (includeMailboxProtocols) {
            progress.SetStatus("POP3の応答を確認しています...");
            BOOL pop = ProbeProtocol(serverAddress, 110, detail);
            AppendResult(report, "POP3疎通", pop, detail);
            progress.SetStatus("IMAPの応答を確認しています...");
            BOOL imap = ProbeProtocol(serverAddress, 143, detail);
            AppendResult(report, "IMAP疎通", imap, detail);
        }
        // 挨拶確認がタイムアウトしても、別接続でテストメール送信を試す。
        if (!smtpReceiver) {
            AppendSkipped(report, "テストメール",
                "SMTP受信サービスの再起動を確認できないため送信していません");
        } else if (!testAddress || !testAddress[0]) {
            AppendSkipped(report, "テストメール",
                "送信対象のWindowsローカルユーザーを取得できないため送信していません");
        } else if (accountManagementMode == 2 && !adAccountReady) {
            AppendSkipped(report, "テストメール",
                "AD資格情報またはメールグループ所属を確認できないため送信していません");
        } else {
          progress.SetStatus("テストメールを送信しています（最大30秒）...");
          BOOL recipientNotRegistered = FALSE;
          // AD連携は既存UPNの自己送受信をローカル配送として確認する。
          // AD資格情報はWindowsで事前確認済みのためSMTP AUTHは行わない。
          LPCTSTR smtpLoginId = smtpAuthUser;
          if (accountManagementMode == 2)
              smtpLoginId = NULL;
          BOOL mail = SendTestMail(serverAddress, testAddress, detail,
              recipientNotRegistered, smtpLoginId, smtpAuthPassword);
          if (recipientNotRegistered) {
            CString skipped;
            skipped.Format("%s は未登録のため送信していません", testAddress);
            AppendSkipped(report, "テストメール", skipped);
          } else {
            AppendResult(report, "テストメール", mail, detail);
            if (mail) {
                progress.SetStatus("テストメールの受信フォルダを確認しています...");
                BOOL inboxFound = ResolveInboxFolder(testAddress,
                    configuredInboxTemplate, inboxFolderToOpen, detail);
                AppendResult(report, "受信フォルダ", inboxFound, detail);
            }
          }
        }
        WSACleanup();
    } else {
        AppendResult(report, "ネットワーク診断", FALSE, "Winsockを初期化できません");
    }

    if (uninitialize) CoUninitialize();
    report += "\r\n[要確認]がある場合は、サービスのインストール状態とネットワーク設定を確認してください。";
    progress.Close();
    if (!inboxFolderToOpen.IsEmpty())
        ShellExecute(NULL, "open", inboxFolderToOpen, NULL, NULL, SW_SHOWNORMAL);
    return report;
}
