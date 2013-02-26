/* Mini utilit… per disconnetersi, spegnere e riavviare semplicemente */
/* Codice comune per Win9x e WinNT, ma richiede COMPILAZIONI SEPARATE! */

#pragma comment(linker,"/ENTRY:EWX")
#pragma comment(linker,"/OPT:nowin98")
#pragma comment(linker,"/SUBSYSTEM:windows")
#pragma comment(linker,"/RELEASE")
#pragma comment(linker,"/MERGE:.rdata=.text")
#pragma comment(linker,"/MERGE:.data=.text")

#pragma comment(linker,"/DEFAULTLIB:ADVAPI32.lib")
#pragma comment(linker,"/DEFAULTLIB:USER32.lib")
#pragma comment(linker,"/DEFAULTLIB:KERNEL32.lib")
#ifdef MADE_FOR_NT
#pragma comment(linker,"/DEFAULTLIB:SHELL32.lib")
#define strstr StrStr
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>

#ifdef MADE_FOR_NT
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
#endif // MADE_FOR_NT

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
#ifndef MADE_FOR_NT
/* Vedi modulo EWX16.cpp che richiama opportunamente la API 16-bit */
	if ( strstr(GetCommandLine(),"-restart") )
		return ExitWindows16();
	if ( strstr(GetCommandLine(),"-suspend") )
		return SetSystemPowerState(0,0);
#endif // !MADE_FOR_NT

#ifdef MADE_FOR_NT
	if ( !(op&EWX_LOGOFF) && !NT_Set_Privilege())
		return MessageBox(0,"Non si possiedono i necessari privilegi.","Impossibile!",MB_ICONSTOP);
#endif // MADE_FOR_NT

/* Senza parametri, non succede altro! */
	return ExitWindowsEx(op,0);
} 
