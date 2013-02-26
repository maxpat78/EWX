/* Mini utilit… per disconnetersi, spegnere e riavviare semplicemente */

#pragma comment(linker,"/ENTRY:EWX")
#pragma comment(linker,"/SUBSYSTEM:windows")
#pragma comment(linker,"/RELEASE")

#pragma comment(linker,"/DEFAULTLIB:ADVAPI32.lib")
#pragma comment(linker,"/DEFAULTLIB:USER32.lib")
#pragma comment(linker,"/DEFAULTLIB:KERNEL32.lib")
#pragma comment(linker,"/DEFAULTLIB:SHLWAPI.lib")
#pragma comment(linker,"/DEFAULTLIB:POWRPROF.lib")
#define strstr StrStr

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#include <powrprof.h>

/* Su NT bisogna essere "privilegiati" per chiudere la sessione... */
BOOL NT_Set_Privilege()
{
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

BOOL EWX()
{
	int op = 0;
	if ( strstr(GetCommandLine(),"-logoff") )
		op |= EWX_LOGOFF;
	if ( strstr(GetCommandLine(),"-force") )
		op |= EWX_FORCE;
/* Mentre su NT EWX_POWEROFF fa proprio quel che promette, su Win98 no!*/
/* Ivi solo EWX_SHUTDOWN spegne il sistema togliendo la corrente. */
	if ( strstr(GetCommandLine(),"-poweroff") )
		op |= EWX_POWEROFF;
	if ( strstr(GetCommandLine(),"-shutdown") )
		op |= EWX_SHUTDOWN;
	if ( strstr(GetCommandLine(),"-reboot") )
		op |= EWX_REBOOT;
/* Chiude la Barra delle Applicazioni (Explorer): si guadagna RAM? */
	if ( strstr(GetCommandLine(),"-exit") )
		return PostMessage(FindWindow("Progman","Program Manager"),WM_QUIT,0,0);
	if ( strstr(GetCommandLine(),"-suspend") )
		return SetSuspendState(0, 0, 0);
	if ( strstr(GetCommandLine(),"-hibernate") )
		return SetSuspendState(1, 0, 0);

	if ( !(op&EWX_LOGOFF) && !NT_Set_Privilege())
		return MessageBox(0,"Mancano i necessari privilegi.","Impossibile!",MB_ICONSTOP);

/* Senza parametri, non succede altro! */
	return ExitWindowsEx(op,0);
} 
