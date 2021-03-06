#include "GlobalHook.h"

#pragma data_seg("HOOK_SEG")
HHOOK g_hKeyHook = NULL;
HWND g_hWnd = NULL;
#pragma data_seg()

HINSTANCE	g_hInstance;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		g_hInstance = hModule;
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


int SetHook(HWND hWnd)
{
	g_hKeyHook = ::SetWindowsHookEx(WH_KEYBOARD, KeyHookProc, g_hInstance, 0);
	if (NULL == g_hKeyHook)
	{
		::MessageBoxW(NULL, L"フック設定失敗", L"Error", MB_OK);
	}
	g_hWnd = hWnd;
	return 0;
}

int ResetHook()
{
	if (0 == ::UnhookWindowsHookEx(g_hKeyHook))
	{
		MessageBoxW(NULL, L"フック解除失敗", L"Error", MB_OK);
	}
	return 0;
}

LRESULT CALLBACK KeyHookProc(int code, WPARAM wparam, LPARAM lparam)
{
	if (code < 0)
	{
		return ::CallNextHookEx(g_hKeyHook, code, wparam, lparam);
	}

	if (HC_ACTION == code)
	{
		if (0 == (lparam & 0x20000000))
		{
			::PostMessage(g_hWnd, WM_GLOBAL_HOOK_KEYDOWN, wparam, lparam);
		}
		else
		{
			::PostMessage(g_hWnd, WM_GLOBAL_HOOK_SYSKEYDOWN, wparam, lparam);
		}
	}
	return ::CallNextHookEx(g_hKeyHook, code, wparam, lparam);
}