#pragma comment(linker,"/DEFAULTLIB:ADVAPI32.lib")
#pragma comment(linker,"/DEFAULTLIB:KERNEL32.lib")
#pragma comment(linker,"/DEFAULTLIB:POWRPROF.lib")
#pragma comment(linker,"/DEFAULTLIB:SHELL32.lib")
#pragma comment(linker,"/DEFAULTLIB:SHLWAPI.lib")
#pragma comment(linker,"/DEFAULTLIB:USER32.lib")
#pragma comment(linker,"/DEFAULTLIB:GDI32.lib")

#pragma comment(linker,"/ENTRY:EWX")
#pragma comment(linker,"/SUBSYSTEM:windows")

#include <windows.h>
#include <shlwapi.h>
#include <powrprof.h>

UINT uTimeout = 0;

LRESULT
CALLBACK
WndProc1(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam) {
   switch(msg) {
	  case WM_COMMAND:
         if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == 0x101)
            ExitProcess(0);
         break;

      case WM_TIMER: {
            char text[128];
            if (wParam != 0x102) return 0;
            if (!uTimeout) DestroyWindow(hWnd);
            wsprintf(text, "Esecuzione tra %d secondi...", uTimeout--);
            SetDlgItemText(hWnd,0x100,text);
            return 0;
      }
      
      case WM_DESTROY:
         PostQuitMessage(0);
         break;

      case WM_CREATE: {
            HWND hw;
            HFONT hFont = (HFONT) GetStockObject(ANSI_VAR_FONT);
         
            hw = CreateWindowEx(0,"STATIC",0,WS_CHILD|WS_VISIBLE,2,4,308,20,hWnd,(HMENU)0x100,0,0);
            SendMessage(hw, WM_SETFONT, (WPARAM)hFont, 1);
         
            hw = CreateWindowEx(0,"BUTTON","Annulla",WS_CHILD|WS_VISIBLE,135,32,50,20,hWnd,(HMENU)0x101,0,0);
            SendMessage(hw, WM_SETFONT, (WPARAM)hFont, 1);
         
            SetTimer(hWnd, 0x102, 1000, 0);
            SendMessage(hWnd, WM_TIMER, 0x102, 0);
         
            return 0;
      }
      
      default:
         return(DefWindowProc(hWnd,msg,wParam,lParam));
   }

   return 0;
}


void PauseWindow(UINT uiSeconds) {
   WNDCLASSEX wc;
   MSG msg;
   HWND hmyWnd;
   UINT cx, cy;

   wc.cbSize = sizeof(WNDCLASSEX);
   wc.lpszClassName = "EWXCLASS";
   wc.lpfnWndProc = WndProc1;
   wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
   wc.hInstance = 0;
   wc.hIcon = ExtractIcon(0, "SHELL32.dll", 167); // "Hearth with small clock"
   wc.hIconSm = 0;
   wc.hCursor = 0;
   wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
   wc.lpszMenuName = 0;
   wc.cbClsExtra = 0;
   wc.cbWndExtra = 0;

   RegisterClassEx(&wc);

   cx = GetSystemMetrics(SM_CXSCREEN);
   cy = GetSystemMetrics(SM_CYSCREEN);

   uTimeout = uiSeconds;
   
   hmyWnd = CreateWindowEx(0,wc.lpszClassName,"ExitWindowsEx",0,cx/2-160,cy/2-50,320,100,0,0,0,0);
   ShowWindow(hmyWnd,SW_SHOWNORMAL);

   while(GetMessage(&msg,0,0,0) != 0) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }
}

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

static char* Revision = "$Revision: 1.03";

static struct {wchar_t* Op; UINT Opcode;} Ops[] = {
	{L"exit", 0x80000000},
	{L"force", EWX_FORCE},
	{L"forceifhung", EWX_FORCEIFHUNG},
	{L"hibernate", 0x40000000},
	{L"hybrid", EWX_HYBRID_SHUTDOWN},
	{L"logoff", EWX_LOGOFF},
	{L"poweroff", EWX_POWEROFF},
	{L"reboot", EWX_REBOOT},
	{L"restartapps", EWX_RESTARTAPPS},
	{L"shutdown", EWX_SHUTDOWN},
	{L"suspend", 0x20000000}
};

BOOL EWX() {
#ifdef DEBUG
    wchar_t s[512];
#endif
	int nArgs;
	LPWSTR *Arglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	UINT op = 0; // EWX_LOGOFF, default
    UINT uSeconds = 0;

	for (int i=1; i < nArgs; i++) {
		if (Arglist[i][0] != L'/' && Arglist[i][0] != L'-') continue;
        if (Arglist[i][1] == L't' && Arglist[i+1])
            uSeconds = StrToIntW(Arglist[i+1]);
		for (int j=0; j < 11; j++) {
			if (!lstrcmpW(Ops[j].Op, Arglist[i]+1))
				op |= Ops[j].Opcode;
		}
	}
	if (op & 0x80000000)
		return PostMessage(FindWindow("Progman","Program Manager"),WM_QUIT,0,0);
	if (op & 0x20000000)
		return SetSuspendState(0, 0, 0);
	if (op & 0x40000000)
		return SetSuspendState(1, 0, 0);
	if ( !(op&EWX_LOGOFF) && !NT_Set_Privilege())
		return MessageBox(0,"Non si dispone del privilegio SeShutdownName!","Errore",MB_ICONSTOP);

#ifdef DEBUG
	wsprintfW(s, L"nArgs=%d, Arg[1]=\"%ws\", op=%08X", nArgs, Arglist[1], op);
	MessageBoxW(0, s, L"Debug", MB_OK);
#endif
    if (uSeconds)
      PauseWindow(uSeconds);  
    
	return ExitWindowsEx(op,SHTDN_REASON_FLAG_PLANNED);
} 
