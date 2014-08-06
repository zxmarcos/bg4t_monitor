// ============================================================================
// ttx_monitor: Opensource TaitoTypeX loader
// by Romhack
// ============================================================================

#include <windows.h>
#include <string.h>
#include "stdafx.h"
#include <list>

#define DEBUG_API	0
#define LOG_IO_STREAM	0
#define DEBUG_WHEEL	0

static const HANDLE hConnection = (HANDLE) 0x5EED0000;
static const HANDLE hConnectionSteer = (HANDLE) 0x5EED0001;
using namespace std;

std::queue<BYTE> replyBuffer[2];

static int isInitialized = 0;
#define MONITOR_JVS	0
#define MONITOR_SW	1
#define MONITOR_HANDLER(h)	(((DWORD)hFile)&1)

// BattleGear4 Tuned, chama a função nativa para exibir uma mensagem na tela.
typedef void (*LPDrawMessage)(void *font, int x, int y, unsigned color, const char *fmt, ...);
static LPDrawMessage __DrawMessage = (LPDrawMessage) 0x5FC900;
// C99...
#define DrawMessage(x, y,...) do {\
	DWORD object1 = *((DWORD *) 0x82D4A0);\
	if (!object1)	\
		break;\
	DWORD *object1_ptr = (DWORD *) object1;\
	DWORD *object2 = (DWORD *) object1_ptr[0x2C >> 2];\
	if (!object2)\
		break;\
	void *font = (void *) object2[0x1C >> 2];\
	if (!font)\
		break;\
	__DrawMessage(font, x, y, 0xFF00FF00, __VA_ARGS__);\
} while (0)


// ============================================================================
static LPGetCommModemStatus __GetCommModemStatus = NULL;
static LPEscapeCommFunction __EscapeCommFunction = NULL;
static LPClearCommError __ClearCommError = NULL;
static LPWriteFile __WriteFile = NULL;
static LPReadFile __ReadFile = NULL;
static LPCloseHandle __CloseHandle = NULL;
static LPCreateFile  __CreateFile = NULL;
static LPSetupComm  __SetupComm = NULL;
static LPGetCommState  __GetCommState = NULL;
static LPSetCommState  __SetCommState = NULL;
static LPSetCommMask  __SetCommMask = NULL;
static LPSetCommTimeouts  __SetCommTimeouts = NULL;
static LPGetCommTimeouts  __GetCommTimeouts = NULL;
static LPGetWindowTextA __GetWindowTextA = NULL;
static LPGetWindowTextW __GetWindowTextW = NULL;
static LPGetAsyncKeyState __GetAsyncKeyState = NULL;
static LPCreateWindowExA __CreateWindowExA = NULL;
static LPCreateWindowExW __CreateWindowExW = NULL;
static LPCreateFileA  __CreateFileA = NULL;
static LPCreateFileW  __CreateFileW = NULL;
static LPGetFileAttributesA __GetFileAttributesA = NULL;

//////////////////////////////////////////////////////////////////////////////////////
// Unicode/ANSI functions (W/A)
//////////////////////////////////////////////////////////////////////////////////////



HWND __stdcall Hook_CreateWindowExA(DWORD dwExStyle,
									LPCSTR lpClassName,
									LPCSTR lpWindowName,
									DWORD dwStyle,
									int x,
									int y,
									int nWidth,
									int nHeight,
									HWND hWndParent,
									HMENU hMenu,
									HINSTANCE hInstance,
									LPVOID lpParam)
{

	//if (!(dwStyle & WS_CHILD))
	//{
	//	dwExStyle =0;
	//	dwStyle = WS_OVERLAPPEDWINDOW;
	//}
	logmsg("%s(%x,%s,%s,%x,%d,%d,%d,%d,%x,%x,%x,%p)\n", __FUNCTION__,
		dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent,
		hMenu, hInstance, lpParam);

	return __CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent,
		hMenu, hInstance, lpParam);
}


HWND __stdcall Hook_CreateWindowExW(DWORD dwExStyle,
									LPCWSTR lpClassName,
									LPCWSTR lpWindowName,
									DWORD dwStyle,
									int x,
									int y,
									int nWidth,
									int nHeight,
									HWND hWndParent,
									HMENU hMenu,
									HINSTANCE hInstance,
									LPVOID lpParam)
{
	//if (!(dwStyle & WS_CHILD))
	//{
	//	dwExStyle =0;
	//	dwStyle = WS_OVERLAPPEDWINDOW;
	//}

	logmsgw(L"%s(%x,%s,%s,%x,%d,%d,%d,%d,%x,%x,%x,%p)\n", __FUNCTION__,
		dwExStyle, NULL, NULL, dwStyle, x, y, nWidth, nHeight, hWndParent,
		hMenu, hInstance, lpParam);

	return __CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent,
		hMenu, hInstance, lpParam);
}


int __stdcall Hook_GetWindowTextA(HWND hWnd, LPSTR lpString, int nMaxCount)
{
	LPSTR text = "ttx_monitor_by_romhack";
	LPSTR s = text, p = lpString;

	for (int i=0; i<nMaxCount; i++) {
		if (*s)
			*p++ = *s++;
	}
	return TRUE;
}

int __stdcall Hook_GetWindowTextW(HWND hWnd, LPWSTR lpString, int nMaxCount)
{
	LPWSTR text = L"ttx_monitor_by_romhack";
	LPWSTR s = text, p = lpString;

	for (int i=0; i<nMaxCount; i++) {
		if (*s)
			*p++ = *s++;
	}
	return TRUE;
}


static inline bool isMonitored(HANDLE handle)
{
	if ((handle == hConnection) || (handle == hConnectionSteer))
		return true;
	return false;
}

// ============================================================================


LPVOID __stdcall SetHookFunction(LPVOID dst, LPVOID lpHookFunction, LPCTSTR name)
{

	DWORD oldProt = 0, newProt = PAGE_EXECUTE_READWRITE;
	VirtualProtect(dst, 4, newProt, &oldProt);

#if 1
	LPVOID ret = (LPVOID) *((DWORD*) dst); 
#else
	LPVOID ret = (LPVOID) GetProcAddress(NULL, name);
#endif
	logmsg("Hooking %s em %08p (%08X) para %08p\n", name, dst, *((DWORD*)dst), lpHookFunction);

	*((DWORD *) dst) = (DWORD) lpHookFunction; 
	VirtualProtect(dst, 4, oldProt, &newProt);

	return ret;
}

#define __HOOK(addr, name)	\
	__ ##name = (LP ##name) SetHookFunction((LPVOID *) addr, &Hook_ ##name, #name)

#define __XHOOKA(mod, name)	\
	__ ##name = (LP ##name) HookIt(mod, #name "A", (LPVOID) &Hook_ ##name)

#define __XHOOKW(mod, name)	\
	__ ##name = (LP ##name) HookIt(mod, #name "W", (LPVOID) &Hook_ ##name)
#define __XHOOKn(mod, name)	\
	__ ##name = (LP ##name) HookIt(mod, #name, (LPVOID) &Hook_ ##name)

#define __XHOOKX(mod, ori, name)	\
	__ ##name = (LP ##name) HookIt(mod, #ori, (LPVOID) &Hook_ ##name)

BOOL __stdcall TTX_HookFunctions()
{
	logmsg("Fazendo o hooking das funções...\n");
	//__XHOOKA("kernel32.dll", CreateFile);

	__XHOOKn("kernel32.dll", CreateFileA);
	__XHOOKn("kernel32.dll", CreateFileW);

	// O samurai trava nessa função, logo...
	__XHOOKn("user32.dll", GetWindowTextA);
	__XHOOKn("user32.dll", GetWindowTextW);

	//__XHOOKn("user32.dll", CreateWindowExA);
	//__XHOOKn("user32.dll", CreateWindowExW);
	__XHOOKn("kernel32.dll", GetFileAttributesA);
	__XHOOKn("kernel32.dll", WriteFile);
	__XHOOKn("kernel32.dll", ReadFile);
	__XHOOKn("kernel32.dll", CloseHandle);
	__XHOOKn("kernel32.dll", GetCommModemStatus);
	__XHOOKn("kernel32.dll", EscapeCommFunction);
	__XHOOKn("kernel32.dll", ClearCommError);
	__XHOOKn("kernel32.dll", SetCommMask);
	__XHOOKn("kernel32.dll", SetupComm);
	__XHOOKn("kernel32.dll", GetCommState);
	__XHOOKn("kernel32.dll", SetCommState);
	__XHOOKn("kernel32.dll", SetCommTimeouts);
	__XHOOKn("kernel32.dll", GetCommTimeouts);
	__XHOOKn("DINPUT8.dll", DirectInput8Create);

	// As duas versões são retrocompatíveis.
	//HookIt("D3D8.dll", "Direct3DCreate8", (LPVOID) &Direct3DCreate9);
	//__XHOOKn("D3D9.dll", Direct3DCreate9);

	__XHOOKn("user32.dll", GetAsyncKeyState);

	return TRUE;
}
#undef __HOOK
#undef __XHOOKA
#undef __XHOOKn



SHORT __stdcall Hook_GetAsyncKeyState(int vKey)
{
	return 0;
}

BOOL __stdcall Hook_GetCommModemStatus(HANDLE hFile, LPDWORD lpModemStat)
{
	if (!isMonitored(hFile)) {
		return __GetCommModemStatus(hFile, lpModemStat);
	}
#if DEBUG_API
	logmsg("GetCommModemStatus(%x, %p)\n",
		hFile, lpModemStat);
#endif

	// Só devemos retornar esse valor quando a JVS receber o comando de Address
	if (is_addressed())
		*lpModemStat = 0x10;
	else
		*lpModemStat = 0;

	return TRUE;
}

BOOL __stdcall Hook_EscapeCommFunction(HANDLE hFile, DWORD dwFunc)
{

	if (!isMonitored(hFile)) {
		return __EscapeCommFunction(hFile, dwFunc);
	}
#if DEBUG_API
	logmsg("EscapeCommFunction(%x, %x)\n",
		hFile, dwFunc);
#endif
	return TRUE;
}

BOOL __stdcall Hook_ClearCommError(HANDLE hFile, LPDWORD lpErrors, LPCOMSTAT lpStat)
{
	if (!isMonitored(hFile)) {
		return __ClearCommError(hFile, lpErrors, lpStat);
	}
#if DEBUG_API
	logmsg("ClearCommError(%x, %p, %p)\n",
		hFile, lpErrors, lpStat);
#endif
	DWORD chan = MONITOR_HANDLER(hFile);
	if (lpStat) {
		if (!replyBuffer[chan].empty())
			lpStat->cbInQue = replyBuffer[chan].size();
		else
			lpStat->cbInQue = 0;
	}

	return TRUE;
}

BOOL __stdcall Hook_SetupComm(HANDLE hFile, DWORD dwInQueue, DWORD dwOutQueue)
{

	if (!isMonitored(hFile)) {
		return __SetupComm(hFile, dwInQueue, dwOutQueue);
	}
#if DEBUG_API
	logmsg("SetupComm(%x, %d, %d)\n",
		hFile, dwInQueue, dwOutQueue);
#endif
	return TRUE;
}

BOOL __stdcall Hook_GetCommState(HANDLE hFile, LPDCB lpDCB)
{
	if (!isMonitored(hFile)) {
		return __GetCommState(hFile, lpDCB);
	}
#if DEBUG_API
	logmsg("GetCommState(%x, %p)\n",
		hFile, lpDCB);
#endif
	return TRUE;
}

BOOL __stdcall Hook_SetCommState(HANDLE hFile, LPDCB lpDCB)
{
	if (!isMonitored(hFile)) {
		return __SetCommState(hFile, lpDCB);
	}
#if DEBUG_API
	logmsg("SetCommState(%x, %p)\n",
		hFile, lpDCB);
#endif
	return TRUE;
}

BOOL __stdcall Hook_SetCommMask(HANDLE hFile, DWORD dwEvtMask)
{

	if (!isMonitored(hFile)) {
		return __SetCommMask(hFile, dwEvtMask);
	}
#if DEBUG_API
	logmsg("SetCommMask(%x, %x)\n",
		hFile, dwEvtMask);
#endif
	return TRUE;
}

BOOL __stdcall Hook_GetCommTimeouts(HANDLE hFile, LPCOMMTIMEOUTS lpCommTimeouts)
{

	if (!isMonitored(hFile)) {
		return __GetCommTimeouts(hFile, lpCommTimeouts);
	}
#if DEBUG_API
	logmsg("GetCommTimeouts(%x, %p)\n",
		hFile, lpCommTimeouts);
#endif
	return TRUE;
}

BOOL __stdcall Hook_SetCommTimeouts(HANDLE hFile, LPCOMMTIMEOUTS lpCommTimeouts)
{

	if (!isMonitored(hFile)) {
		return __SetCommTimeouts(hFile, lpCommTimeouts);
	}
#if DEBUG_API
	logmsg("SetCommTimeouts(%x, %p)\n",
		hFile, lpCommTimeouts);
#endif
	return TRUE;
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

unsigned long MyMap(unsigned long x, unsigned long in_min, unsigned long in_max, unsigned long out_min, unsigned long out_max) {

      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static inline int clampDeadZone(long value)
{
	if (value > 100 || value < -100)
		return value;
	return 0;
}

BOOL __stdcall Hook_WriteFile(HANDLE hFile,
							  LPVOID lpBuffer,
							  DWORD nNumberOfBytesToWrite,
							  LPDWORD lpNumberOfBytesWritten,
							  LPOVERLAPPED lpOverlapped)
{

	if (!isMonitored(hFile)) {
		return __WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
	}
	DWORD chan = MONITOR_HANDLER(hFile);

#if DEBUG_API
	logmsg("WriteFile(%x, %p, %d, %p, %p)\n",
		hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
#endif

	BYTE *ptr = (BYTE *) lpBuffer;

	//logmsg("Recebendo %d bytes...\n", nNumberOfBytesToWrite);
#if LOG_IO_STREAM
	if (chan == MONITOR_SW) {
		logmsg("RD:  ");
		for (DWORD i = 0; i < nNumberOfBytesToWrite; i++) {
			logmsg("%02X ", (DWORD) *ptr);
			++ptr;
		}
		logmsg("\n");
	}
#endif

	*lpNumberOfBytesWritten = nNumberOfBytesToWrite;

	BYTE rbuffer[1024];
	// processa a saída da JVS
	if (chan == MONITOR_JVS) {
		DWORD sz = process_stream((LPBYTE)lpBuffer, nNumberOfBytesToWrite, rbuffer, 1024);
		if (sz != 1) {
			for (DWORD i=0; i < sz; i++)
				replyBuffer[chan].push(rbuffer[i]);
		}
	} else {
		// processa a saída do volante
		DWORD pos = 0;
		BYTE *stream = (BYTE *) lpBuffer;

		while (pos < nNumberOfBytesToWrite) {
			switch (stream[pos]) {

			case 0x20: //????
				replyBuffer[chan].push(1);
				pos += 2;
				break;

			case 0x1F:
			case 0x00: // steering wheel (in-game)
				{

					int analog = inputMgr.GetState(ANALOG_3);
					// 1025 - 2045
					unsigned counter = MyMap(analog, -1000, 1000, 0x400, 0x7FF);
					if (!clampDeadZone(analog))
						counter = 1535;
	
					// O valor enviado é convertido através desta função
#if 0
					char low = LOBYTE(counter);
					unsigned result = 0;
					char high = HIBYTE(counter);
					if ( low & 0xC )
						result = high | ((low & 3) << 8);
					else
						result = low | ((high & 3) << 8);

					logmsg("%d = %x\n", counter, result);
#endif
#if 0
					DrawMessage(000, 500, "HANDLE: %4x, ACCEL: %4x, BRAKE: %4", counter,
						inputMgr.GetState(ANALOG_1), inputMgr.GetState(ANALOG_2));
#endif
					replyBuffer[chan].push(HIBYTE(counter));
					replyBuffer[chan].push(LOBYTE(counter));
					pos += 2;
					break;
				}


			default:
				{
					// FORCE FEEDBACK?
					pos += 2;
					break;
				}
			}
		}
	}
	return TRUE;
}



BOOL __stdcall Hook_ReadFile(HANDLE hFile,
							 LPVOID lpBuffer,
							 DWORD nNumberOfBytesToRead,
							 LPDWORD lpNumberOfBytesRead,
							 LPOVERLAPPED lpOverlapped)
{


	if (!isMonitored(hFile)) {
		return __ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	}
#if DEBUG_API
	logmsg("ReadFile(%x, %p, %d, %p, %p)\n",
		hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
#endif

	DWORD chan = MONITOR_HANDLER(hFile);

	if (replyBuffer[chan].size())
	{
		if (nNumberOfBytesToRead >= replyBuffer[chan].size())
			nNumberOfBytesToRead = replyBuffer[chan].size();
		*lpNumberOfBytesRead = nNumberOfBytesToRead;
		BYTE *ptr = (BYTE*) lpBuffer;
		for (DWORD i=0; i < nNumberOfBytesToRead; i++) {
			if (!replyBuffer[chan].empty()) {
				*ptr++ = replyBuffer[chan].front();
				replyBuffer[chan].pop();
			} else
				*ptr++ = 0;
		}
#if LOG_IO_STREAM
		if (chan == MONITOR_SW) {
			//logmsg("Lidos %d\n", nNumberOfBytesToRead);
			ptr = (BYTE*) lpBuffer;
			logmsg("SD:  ");
			for (DWORD i=0; i < nNumberOfBytesToRead; i++) {
				logmsg("%02X ", (DWORD) *ptr++);
			}
			logmsg("\n");
		}
#endif
	} else {
		*lpNumberOfBytesRead = 0;
		return TRUE;
	}


	return TRUE;
}


BOOL __stdcall Hook_CloseHandle(HANDLE hObject)
{

	if (hObject != hConnection) {
		return __CloseHandle(hObject);
	} else
		reset_addressed();
#if DEBUG_API
	logmsg("CloseHandle(%x)\n", hObject);
#endif
	return TRUE;
}


static int __createfile_nested = 0;
//
//
//
HANDLE __stdcall Hook_CreateFile(LPCTSTR lpFileName,
								 DWORD dwDesiredAccess,
								 DWORD dwShareMode,
								 LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								 DWORD dwCreationDisposition,
								 DWORD dwFlagsAndAttributes,
								 HANDLE hTemplateFile)
{
#if 0//DEBUG_API
	if (!__createfile_nested) {
		__createfile_nested = 1;
		logmsg("CreateFile(%s, %x, %x, %p, %x, %x, %x)\n",
			lpFileName,
			dwDesiredAccess,
			dwShareMode,
			lpSecurityAttributes,
			dwCreationDisposition,
			dwFlagsAndAttributes,
			hTemplateFile);
		__createfile_nested = 0;
	}
#endif
	if (!strcmp(lpFileName, "COM2")) {
		if (!isInitialized) {
			inputMgr.Init();
			isInitialized = 1;

		}
		return hConnection;

	} else {
		return __CreateFile(lpFileName,
			dwDesiredAccess,
			dwShareMode,
			lpSecurityAttributes,
			dwCreationDisposition,
			dwFlagsAndAttributes,
			hTemplateFile);
	}
}




int _mbsnbcmp(const char *a, const char *b, int l)
{
	int ret = 0;
	for (int i=0;i<l;i++) {
		ret += ((int)*a - (int)*b);
		++a;
		++b;
	}
	return ret;
}

HANDLE __stdcall Hook_CreateFileA(LPCSTR lpFileName,
								  DWORD dwDesiredAccess,
								  DWORD dwShareMode,
								  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								  DWORD dwCreationDisposition,
								  DWORD dwFlagsAndAttributes,
								  HANDLE hTemplateFile)
{
#if DEBUG_API
	if (!__createfile_nested) {
		__createfile_nested = 1;
		logmsg("CreateFile(%s, %x, %x, %p, %x, %x, %x)\n",
			lpFileName,
			dwDesiredAccess,
			dwShareMode,
			lpSecurityAttributes,
			dwCreationDisposition,
			dwFlagsAndAttributes,
			hTemplateFile);
		__createfile_nested = 0;
	}
#endif
	if (!strcmp(lpFileName, "COM2")) {
		if (!isInitialized) {
			inputMgr.Init();
			isInitialized = 1;

		}
		return hConnection;
	} else if (!strcmp(lpFileName, "COM1")) {
		if (!isInitialized) {
			inputMgr.Init();
			isInitialized = 1;
		}
#if DEBUG_WHEEL
		mapHandleValues();
#endif
		return hConnectionSteer;
	}
#if 1
	else
	{

		if (!(_mbsnbcmp(lpFileName, "D:\\", 3)) || !(_mbsnbcmp(lpFileName, "d:\\", 3)) ||
			!(_mbsnbcmp(lpFileName, "E:\\", 3)) || !(_mbsnbcmp(lpFileName, "e:\\", 3)))
		{
			// skip D:\\ or E:\\

			lpFileName += 3;
			//logmsg("load at: %s\n", lpFileName);
#if 0
			HANDLE hTemp = __CreateFileA(lpFileName+3, GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
			if (hTemp == INVALID_HANDLE_VALUE)
			{

				if ((_mbsnbcmp(lpFileName, "D:\\WINDOWS", 10)))
				{
					static char pathbuf[MAX_PATH];
					char *pstr, *str = strdup(lpFileName);
					pstr = str;
					while (*pstr != '\\')
						pstr++;
					*pstr = '/';

					sprintf(pathbuf, "%s%s", "save", pstr);
					logmsg("PATH: %s\n", pathbuf);

					char *_path = strdup(pathbuf);
					pstr = _path;
					char *lastSlash = NULL;
					while (*pstr) {
						if (*pstr == '\\')
							*pstr = '/';
						if (*pstr == '/')
							lastSlash = pstr;
						++pstr;
					}
					if (lastSlash) {
						*lastSlash = '\0';
						logmsg("Criando diretório %s\n", _path);
						CreateDirectory(_path, NULL);
						free(_path);
					}

					HANDLE ret = __CreateFileA(pathbuf,
						dwDesiredAccess,
						dwShareMode,
						lpSecurityAttributes,
						dwCreationDisposition,
						dwFlagsAndAttributes,
						hTemplateFile);

					free(str);

					return ret;
				}
			}
			else
				CloseHandle(hTemp);
#endif
		}
	}
#endif
	return __CreateFileA(lpFileName,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile);
}

DWORD __stdcall Hook_GetFileAttributesA(LPSTR lpFileName)
{
	if (!(_mbsnbcmp(lpFileName, "D:\\", 3)) || !(_mbsnbcmp(lpFileName, "d:\\", 3)) ||
		!(_mbsnbcmp(lpFileName, "E:\\", 3)) || !(_mbsnbcmp(lpFileName, "e:\\", 3)))
	{
		// skip D:\\ or E:\\

		lpFileName += 3;
	}
	return __GetFileAttributesA(lpFileName);
}

HANDLE __stdcall Hook_CreateFileW(LPCWSTR lpFileName,
								  DWORD dwDesiredAccess,
								  DWORD dwShareMode,
								  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								  DWORD dwCreationDisposition,
								  DWORD dwFlagsAndAttributes,
								  HANDLE hTemplateFile)
{
#if 0//DEBUG_API
	if (!__createfile_nested) {
		__createfile_nested = 1;
		logmsg("CreateFile(%s, %x, %x, %p, %x, %x, %x)\n",
			lpFileName,
			dwDesiredAccess,
			dwShareMode,
			lpSecurityAttributes,
			dwCreationDisposition,
			dwFlagsAndAttributes,
			hTemplateFile);
		__createfile_nested = 0;
	}
#endif
	if (!wcscmp(lpFileName, L"COM2")) {
		if (!isInitialized) {
			inputMgr.Init();
			isInitialized = 1;

		}
		return hConnection;

	} else {
		return __CreateFileW(lpFileName,
			dwDesiredAccess,
			dwShareMode,
			lpSecurityAttributes,
			dwCreationDisposition,
			dwFlagsAndAttributes,
			hTemplateFile);
	}
}