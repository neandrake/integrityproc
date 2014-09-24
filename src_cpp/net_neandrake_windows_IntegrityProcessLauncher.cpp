#include "net_neandrake_windows_IntegrityProcessLauncher.h"

#include <afx.h>

#ifndef SECURITY_MANDATORY_HIGH_RID
	#define SECURITY_MANDATORY_UNTRUSTED_RID            (0x00000000L)
	#define SECURITY_MANDATORY_LOW_RID                  (0x00001000L)
	#define SECURITY_MANDATORY_MEDIUM_RID               (0x00002000L)
	#define SECURITY_MANDATORY_HIGH_RID                 (0x00003000L)
	#define SECURITY_MANDATORY_SYSTEM_RID               (0x00004000L)
	#define SECURITY_MANDATORY_PROTECTED_PROCESS_RID    (0x00005000L)
#endif
 
typedef BOOL (WINAPI *defCreateProcessWithTokenW)
		(HANDLE,DWORD,LPCWSTR,LPWSTR,DWORD,LPVOID,LPCWSTR,LPSTARTUPINFOW,LPPROCESS_INFORMATION);




// Writes Integration Level of the process with the given ID into pu32_ProcessIL
// returns Win32 API error or 0 if succeeded
DWORD GetProcessIL(DWORD u32_PID, DWORD* pu32_ProcessIL)
{
	*pu32_ProcessIL = 0;
	
	HANDLE h_Process   = 0;
	HANDLE h_Token     = 0;
	DWORD  u32_Size    = 0;
	BYTE*  pu8_Count   = 0;
	DWORD* pu32_ProcIL = 0;
	TOKEN_MANDATORY_LABEL* pk_Label = 0;
 
	h_Process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, u32_PID);
	if (!h_Process)
		goto _CleanUp;
 
	if (!OpenProcessToken(h_Process, TOKEN_QUERY, &h_Token))
		goto _CleanUp;
				
	if (!GetTokenInformation(h_Token, TokenIntegrityLevel, NULL, 0, &u32_Size) &&
		 GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		goto _CleanUp;
						
	pk_Label = (TOKEN_MANDATORY_LABEL*) HeapAlloc(GetProcessHeap(), 0, u32_Size);
	if (!pk_Label)
		goto _CleanUp;
 
	if (!GetTokenInformation(h_Token, TokenIntegrityLevel, pk_Label, u32_Size, &u32_Size))
		goto _CleanUp;
 
	pu8_Count = GetSidSubAuthorityCount(pk_Label->Label.Sid);
	if (!pu8_Count)
		goto _CleanUp;
					
	pu32_ProcIL = GetSidSubAuthority(pk_Label->Label.Sid, *pu8_Count-1);
	if (!pu32_ProcIL)
		goto _CleanUp;
 
	*pu32_ProcessIL = *pu32_ProcIL;
	SetLastError(ERROR_SUCCESS);
 
	_CleanUp:
	DWORD u32_Error = GetLastError();
	if (pk_Label)  HeapFree(GetProcessHeap(), 0, pk_Label);
	if (h_Token)   CloseHandle(h_Token);
	if (h_Process) CloseHandle(h_Process);
	return u32_Error;
}
 
// Creates a new process u16_Path with the integration level of the Explorer process (MEDIUM IL)
// If you need this function in a service you must replace FindWindow() with another API to find Explorer process
// The parent process of the new process will be svchost.exe if this EXE was run "As Administrator"
// returns Win32 API error or 0 if succeeded
DWORD CreateProcessMediumIL(WCHAR* u16_Path, WCHAR* u16_CmdLine)
{
	HANDLE h_Process = 0;
	HANDLE h_Token   = 0;
	HANDLE h_Token2  = 0;
	PROCESS_INFORMATION k_ProcInfo    = {0};
	STARTUPINFOW        k_StartupInfo = {0};
 
	BOOL b_UseToken = FALSE;
 
	// Detect Windows Vista, 2008, Windows 7 and higher
	if (GetProcAddress(GetModuleHandleA("Kernel32"), "GetProductInfo"))
	{
		DWORD u32_CurIL;
		DWORD u32_Err = GetProcessIL(GetCurrentProcessId(), &u32_CurIL);
		if (u32_Err)
			return u32_Err;
 
		if (u32_CurIL > SECURITY_MANDATORY_MEDIUM_RID)
			b_UseToken = TRUE;
	}
 
	// Create the process normally (before Windows Vista or if current process runs with a medium IL)
	if (!b_UseToken)
	{
		if (!CreateProcessW(u16_Path, u16_CmdLine, 0, 0, FALSE, 0, 0, 0, &k_StartupInfo, &k_ProcInfo))
			return GetLastError();
 
		CloseHandle(k_ProcInfo.hThread);
		CloseHandle(k_ProcInfo.hProcess); 
		return ERROR_SUCCESS;
	}
 
	defCreateProcessWithTokenW f_CreateProcessWithTokenW = 
		(defCreateProcessWithTokenW) GetProcAddress(GetModuleHandleA("Advapi32"), "CreateProcessWithTokenW");
 
	if (!f_CreateProcessWithTokenW) // This will never happen on Vista!
		return ERROR_INVALID_FUNCTION; 
	
	HWND h_Progman = ::GetShellWindow();
 
	DWORD u32_ExplorerPID = 0;		
	GetWindowThreadProcessId(h_Progman, &u32_ExplorerPID);
 
	// ATTENTION:
	// If UAC is turned OFF all processes run with SECURITY_MANDATORY_HIGH_RID, also Explorer!
	// But this does not matter because to start the new process without UAC no elevation is required.
	h_Process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, u32_ExplorerPID);
	if (!h_Process)
		goto _CleanUp;
 
	if (!OpenProcessToken(h_Process, TOKEN_DUPLICATE, &h_Token))
		goto _CleanUp;
 
	if (!DuplicateTokenEx(h_Token, TOKEN_ALL_ACCESS, 0, SecurityImpersonation, TokenPrimary, &h_Token2))
		goto _CleanUp;
 
	if (!f_CreateProcessWithTokenW(h_Token2, 0, u16_Path, u16_CmdLine, 0, 0, 0, &k_StartupInfo, &k_ProcInfo))
		goto _CleanUp;
 
	SetLastError(ERROR_SUCCESS);
 
	_CleanUp:
	DWORD u32_Error = GetLastError();
	if (h_Token)   CloseHandle(h_Token);
	if (h_Token2)  CloseHandle(h_Token2);
	if (h_Process) CloseHandle(h_Process);
	CloseHandle(k_ProcInfo.hThread);
	CloseHandle(k_ProcInfo.hProcess); 
	return u32_Error;
}


JNIEXPORT void JNICALL Java_net_neandrake_windows_IntegrityProcessLauncher_createProcessWithExplorerIntegrity
  (JNIEnv *env, jobject obj, jstring processPath, jstring processArgs) {
	const char* jchrProcessPath = (processPath == NULL ? NULL : env->GetStringUTFChars(processPath, NULL));
	const char* jchrProcessArgs = (processArgs == NULL ? NULL : env->GetStringUTFChars(processArgs, NULL));
	
	CString* cstrProcessPath = NULL;
	CString* cstrProcessArgs = NULL;
	
	WCHAR* wszProcessPath = NULL;
	WCHAR* wszProcessArgs = NULL;
	
	if (jchrProcessPath != NULL) {
		cstrProcessPath = new CString(jchrProcessPath);
		wszProcessPath = cstrProcessPath->GetBuffer();
	}
	
	if (jchrProcessArgs != NULL) {
		cstrProcessArgs = new CString(jchrProcessArgs);
		wszProcessArgs = cstrProcessArgs->GetBuffer();
	}
	
	if (wszProcessPath == NULL && wszProcessArgs == NULL) {
		goto _JClean;
	}
	
	CreateProcessMediumIL(wszProcessPath, wszProcessArgs);
	
	_JClean:
	
	if (cstrProcessPath != NULL) {
		cstrProcessPath->ReleaseBuffer();
		delete cstrProcessPath;
	}
	if (cstrProcessArgs != NULL) {
		cstrProcessArgs->ReleaseBuffer();
		delete cstrProcessArgs;
	}
	
	env->ReleaseStringUTFChars(processArgs, jchrProcessArgs);
	env->ReleaseStringUTFChars(processPath, jchrProcessPath);
}