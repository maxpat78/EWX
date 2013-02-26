//
// Codice Win32 che usa una libreria Win16 passando parametri numerici.
//
// Sfrutta 3 ordinali nascosti nel modulo KERNEL32 per il collegamento alle
// librerie a 16-bit e QT_Thunk
//
// ESPORTA: int ExitWindows16(void);
//
#pragma comment(linker,"/DEFAULTLIB:THUNK32.lib")

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

extern "C"{
#include <wownt32.h>
extern void QT_Thunk();
}

/* KERNEL32 Ordinal 23h (35) */
typedef WINBASEAPI HINSTANCE (WINAPI* PLOADLIBRARY16)(LPCSTR);
PLOADLIBRARY16 pLoadLibrary16;

/* KERNEL32 Ordinal 25h (36) */
typedef WINBASEAPI FARPROC (WINAPI* PGETPROCADDRESS16)(HMODULE,LPCSTR);
PGETPROCADDRESS16 pGetProcAddress16;

/* KERNEL32 Ordinal 24h (37) */
typedef WINBASEAPI BOOL (WINAPI* PFREELIBRARY16)(HMODULE);
PFREELIBRARY16 pFreeLibrary16;

FARPROC GetDLLProcAddress(HMODULE,LPCSTR);

#define NTSIGNATURE(ptr) ((LPVOID)((BYTE *)(ptr) + ((PIMAGE_DOS_HEADER)(ptr))->e_lfanew))
#define SIZE_OF_NT_SIGNATURE (sizeof(DWORD))
#define PEFHDROFFSET(ptr) ((LPVOID)((BYTE *)(ptr)+((PIMAGE_DOS_HEADER)(ptr))->e_lfanew+SIZE_OF_NT_SIGNATURE))
#define OPTHDROFFSET(ptr) ((LPVOID)((BYTE *)(ptr)+((PIMAGE_DOS_HEADER)(ptr))->e_lfanew+SIZE_OF_NT_SIGNATURE+sizeof(IMAGE_FILE_HEADER)))
#define SECHDROFFSET(ptr) ((LPVOID)((BYTE *)(ptr)+((PIMAGE_DOS_HEADER)(ptr))->e_lfanew+SIZE_OF_NT_SIGNATURE+sizeof(IMAGE_FILE_HEADER)+sizeof(IMAGE_OPTIONAL_HEADER)))
#define RVATOVA(base,offset) ((LPVOID)((DWORD)(base)+(DWORD)(offset)))
#define VATORVA(base,offset) ((LPVOID)((DWORD)(offset)-(DWORD)(base)))

#define SIZE_OF_PARAMETER_BLOCK 4096
#define IMAGE_PARAMETER_MAGIC 0xCDC31337
#define MAX_DLL_PROCESSES 256
#define DLL_ATTACH 0
#define DLL_DETACH 1

FARPROC GetDLLProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	if(hModule==NULL) return NULL;
	
	// Get header
	PIMAGE_OPTIONAL_HEADER   poh;
	poh = (PIMAGE_OPTIONAL_HEADER)OPTHDROFFSET (hModule);
    
	// Get number of image directories in list
	int nDirCount;
	nDirCount=poh->NumberOfRvaAndSizes;
	if(nDirCount<16) return FALSE;

	// - Sift through export table -----------------------------------------------
	if(poh->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size==0) return NULL;

	// Good, we have an export table. Lets get it.
	PIMAGE_EXPORT_DIRECTORY ped;
	ped=(IMAGE_EXPORT_DIRECTORY *)RVATOVA(hModule,poh->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);	
	
	// Get ordinal of desired function
	int nOrdinal;
	
	if(HIWORD((DWORD)lpProcName)==0) {
		nOrdinal=(LOWORD((DWORD)lpProcName)) - ped->Base;
	} else {
		// Go through name table and find appropriate ordinal
		int i,count;
		DWORD *pdwNamePtr;
		WORD *pwOrdinalPtr;
		
		count=ped->NumberOfNames;
		pdwNamePtr=(DWORD *)RVATOVA(hModule,ped->AddressOfNames);
		pwOrdinalPtr=(WORD *)RVATOVA(hModule,ped->AddressOfNameOrdinals);
		
		for(i=0;i<count;i++) {
			
			// XXX should be a binary search, but, again, fuck it.
			char *svName;
			svName=(char *)RVATOVA(hModule,*pdwNamePtr);
			
			if(lstrcmp(svName,lpProcName)==0) {
				nOrdinal=*pwOrdinalPtr;
				break;
			}
			
			pdwNamePtr++;
			pwOrdinalPtr++;
		}
		if(i==count) return NULL;
	}
	
	// Look up RVA of this ordinal
	DWORD *pAddrTable;
	DWORD dwRVA;
	pAddrTable=(DWORD *)RVATOVA(hModule,ped->AddressOfFunctions);
	
	dwRVA=pAddrTable[nOrdinal];
	
	// Check if it's a forwarder, or a local addr
	// XXX  Should probably do this someday. Just don't define forwarders. You're
	// XXX  not loading kernel32.dll with this shit anyway.
	DWORD dwAddr;
	dwAddr=(DWORD) RVATOVA(hModule,dwRVA);

	return (FARPROC) dwAddr;
}


extern "C" int ExitWindows16()
{
 HMODULE hK32 = GetModuleHandle("KERNEL32");

 pLoadLibrary16    = (PLOADLIBRARY16)    GetDLLProcAddress(hK32,(char*)0x23);
 pGetProcAddress16 = (PGETPROCADDRESS16) GetDLLProcAddress(hK32,(char*)0x25);
 pFreeLibrary16    = (PFREELIBRARY16)    GetDLLProcAddress(hK32,(char*)0x24);

 HMODULE hUser = pLoadLibrary16("USER");
 FARPROC fpExitWindows = pGetProcAddress16(hUser,"EXITWINDOWS");

/**********************************************************************
 * 		QT_Thunk			(KERNEL32)
 *
 * The target address is in EDX.
 * The 16 bit arguments start at ESP.
 * The number of 16bit argument bytes is EBP-ESP-0x40 (64 Byte thunksetup).
 * [ok]
 */
WORD Result=0;
__asm {
// Prepara i 64 byte di "Thunk Setup" nella Stack
	mov  eax, 16
L1:
	push 0
	dec  eax
	jnz  L1

	push 0x00420000 // argomenti: 0x42, 0x00 == EXITWINDOWS(EW_RESTART,0);
	mov  edx, fpExitWindows
	call QT_Thunk

	add  esp, 64    // ripristina lo stack pointer
	mov  Result, ax
}
 pFreeLibrary16(hUser);
 return Result;
}
