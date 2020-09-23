#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define	WM_GLOBAL_HOOK_KEYDOWN		(WM_USER + 777)
#define	WM_GLOBAL_HOOK_SYSKEYDOWN	(WM_USER + 778)

LRESULT CALLBACK KeyHookProc(int nCode, WPARAM wp, LPARAM lp);
int SetHook(HWND hWnd);
int ResetHook();