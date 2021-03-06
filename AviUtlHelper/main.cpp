#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "resource.h"
#include <chrono>
#include "../GlobalHook/GlobalHook.h"


void GlobalHookKeyDown(WPARAM wParam, LPARAM lParam)
{
	if (0 == (0x80000000 & lParam) && (VK_OEM_PERIOD == wParam || VK_OEM_COMMA == wParam))
	{
		//	',' or '.'キーが押されたら反応する.

		//	現在のフォーカスが、カスタムオブジェクトの場合のみ動作するようにする.
		HWND hWndDialog = ::GetForegroundWindow();

		//	ウインドウのタイトルバーテキストで判断する.
		{
			WCHAR text[128];
			::GetWindowTextW(hWndDialog, text, sizeof(text) / sizeof(WCHAR));
			if (0 != ::wcscmp(L"カスタムオブジェクト[標準描画]", text) && 0 != ::wcscmp(L"カスタムオブジェクト[拡張描画]", text))
			{
				return;
			}
		}

		{
			//	ウインドウ情報が書き代わるまで待機する.
			//	開始フレーム番号が書き代わるのを監視する.
			WCHAR before[128];
			WCHAR after[128];
			HWND hWndStatic = ::FindWindowExW(hWndDialog, NULL, L"Static", NULL);
			::GetWindowTextW(hWndStatic, before, sizeof(before) / sizeof(WCHAR));

			auto begin = std::chrono::steady_clock::now();
			while(true)
			{
				::GetWindowTextW(hWndStatic, after, sizeof(after) / sizeof(WCHAR));
				if (0 != wcscmp(before, after))
				{
					break;
				}
				if (1000 < std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count())
				{
					return;
				}
				::Sleep(1);
			}
		}

		//	"多目的スライダー@PSDToolKit"が選ばれているかで判断する.
		{
			HWND hWndComboBox = ::FindWindowExW(hWndDialog, NULL, L"ComboBox", NULL);
			auto index = ::SendMessageW(hWndComboBox, CB_GETCURSEL, 0, 0);
			WCHAR text[128];
			::SendMessageW(hWndComboBox, CB_GETLBTEXT, index, (LPARAM)text);
			if (0 != ::wcscmp(L"多目的スライダー@PSDToolKit", text))
			{
				return;
			}
		}

		//	多目的スライダーの1つ目のEditBoxをアクティブかつ、全選択状態にする.
		{
			HWND hWndEdit = ::FindWindowExW(hWndDialog, NULL, L"Edit", NULL);
			::SetForegroundWindow(hWndEdit);
			::SendMessageW(hWndEdit, EM_SETSEL, 0, -1);
		}
	}
}








//	ウインドウ生成周りは、デフォルト生成されるテンプレートそのまま.

const static int MAX_LOADSTRING = 100;
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_AVIUTLHELPER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_AVIUTLHELPER));

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AVIUTLHELPER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(NULL);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // グローバル変数にインスタンス処理を格納します。

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 256, 64, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE:
	{
		SetHook(hWnd);
		break;
	}
    case WM_GLOBAL_HOOK_KEYDOWN:
    {
		GlobalHookKeyDown(wParam, lParam);
		break;
    }
    case WM_DESTROY:
	{
		ResetHook();
		PostQuitMessage(0);
		break;
	}
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
