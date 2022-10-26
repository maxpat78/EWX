#pragma comment(linker,"/DEFAULTLIB:ADVAPI32.lib")
#pragma comment(linker,"/DEFAULTLIB:KERNEL32.lib")
#pragma comment(linker,"/DEFAULTLIB:POWRPROF.lib")
#pragma comment(linker,"/DEFAULTLIB:SHELL32.lib")
#pragma comment(linker,"/DEFAULTLIB:SHLWAPI.lib")
#pragma comment(linker,"/DEFAULTLIB:USER32.lib")

#pragma comment(linker,"/ENTRY:EWX")
#pragma comment(linker,"/SUBSYSTEM:windows")

#include <windows.h>
#include <shlwapi.h>
#include <powrprof.h>

BOOL NT_Set_Privilege() {
	HANDLE hToken; 
	LUID ShutdownNameValue; 
	TOKEN_PRIVILEGES tkp; 

	if (!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken)) return 0;
	if (!LookupPrivilegeValue((LPSTR)0,SE_SHUTDOWN_NAME,&ShutdownNameValue)) return 0;

	tkp.PrivilegeCount = 1; 
	tkp.Privileges[0].Luid = ShutdownNameValue; 
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

	AdjustTokenPrivileges(hToken,0,&tkp,sizeof(TOKEN_PRIVILEGES),(PTOKEN_PRIVILEGES)0,(PDWORD)0);
	if (GetLastError() != ERROR_SUCCESS) return 0;  

	return 1;
}

BOOL EWX() {
	UINT op = 0;
	char *s = GetCommandLine();

	if ( StrStr(s,"-exit") )
		return PostMessage(FindWindow("Progman","Program Manager"),WM_QUIT,0,0);

	if ( StrStr(s,"-force") )
		op |= EWX_FORCE;

	if ( StrStr(s,"-forceifhung") )
		op |= EWX_FORCEIFHUNG;

	if ( StrStr(s,"-hibernate") )
		return SetSuspendState(1, 0, 0);

	if ( StrStr(s,"-hybrid") )
		op |= EWX_HYBRID_SHUTDOWN;

	if ( StrStr(s,"-logoff") )
		op |= EWX_LOGOFF;

	if ( StrStr(s,"-poweroff") )
		op |= EWX_POWEROFF;

	if ( StrStr(s,"-reboot") )
		op |= EWX_REBOOT;

	if ( StrStr(s,"-restartapps") )
		op |= EWX_RESTARTAPPS;

	if ( StrStr(s,"-shutdown") )
		op |= EWX_SHUTDOWN;

	if ( StrStr(s,"-suspend") )
		return SetSuspendState(0, 0, 0);
	
	if ( !(op&EWX_LOGOFF) && !NT_Set_Privilege())
		return MessageBox(0,"ShutdownNameValue privilege not held!","Can't do that...",MB_ICONSTOP);

	return ExitWindowsEx(op,0);
} 
