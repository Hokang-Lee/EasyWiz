#ifndef EASYWIZ_NETWORK_SETUP_H
#define EASYWIZ_NETWORK_SETUP_H

// Windows Firewall、メールサービス起動、プロトコル疎通、テストメールを
// 一括して実行し、情シス担当者向けの日本語レポートを返す。
CString RunMailServerVerification(LPCTSTR serverAddress, LPCTSTR testAddress, BOOL includeMailboxProtocols);
CString GetPrimaryIPv4Address();
CString GenerateRandomMailAddress(LPCTSTR domainName);
DWORD GetMailServerDwordSetting64(LPCTSTR valueName, DWORD defaultValue);
CString GetMailServerStringSetting64(LPCTSTR valueName, LPCTSTR defaultValue);

#endif
