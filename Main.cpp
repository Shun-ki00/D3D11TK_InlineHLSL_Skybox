//
// Main.cpp
//

#include "pch.h"
#include "Game.h"
#include <Mouse.h>
#include <Keyboard.h>

using namespace DirectX;

#include "imgui/imgui_impl_win32.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#pragma clang diagnostic ignored "-Wswitch-enum"
#endif

#pragma warning(disable : 4061)

namespace
{
    // ゲームクラスを宣言
    std::unique_ptr<Game> g_game;
}
// アプリケーション名を定義する
LPCWSTR g_szAppName = L"Balloon";
// プロトタイプを宣言する
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ExitGame() noexcept;

// ハイブリッドグラフィックスシステムにデフォルトでディスクリートパーツを優先させるよう指示する
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

// ウィンドウエントリポイント
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    if (!XMVerifyCPUSupport())
        return 1;

    HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
    if (FAILED(hr))
        return 1;

    // フルスクリーン対応
    static bool s_fullscreen = false;
    // メッセージボックスの表示
    int result = MessageBox(NULL, L"フルスクリーンにしますか？", L"画面モード設定", MB_YESNO | MB_ICONQUESTION);

    // 「はい」または「いいえ」の選択に応じてフルスクリーン設定
    if (result == IDYES) {
        s_fullscreen = true;
    }
    else {
        s_fullscreen = false;
    }

    // ゲームクラスのインスタンスを生成する
    g_game = std::make_unique<Game>();

    // クラスを登録しウィンドウを生成する
    {
        // ウィンドウクラスを登録する
        WNDCLASSEXW wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIconW(hInstance, L"IDI_ICON");
        wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wcex.lpszClassName = L"BalloonWindowClass";
        wcex.hIconSm = LoadIconW(wcex.hInstance, L"IDI_ICON");
        if (!RegisterClassExW(&wcex))
            return 1;

        // ウィンドウの既定サイズを取得する
        int w, h;
        g_game->GetDefaultSize(w, h);

        RECT rc = { 0, 0, static_cast<LONG>(w), static_cast<LONG>(h) };
        // ウィンドウの領域を調整する
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
        // ウィンドウを生成する
        HWND hwnd = CreateWindowExW(
            0,
            L"BalloonWindowClass", // ウィンドウクラス名
            g_szAppName,                     // ウィンドウタイトル
            WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME, // 最大化とサイズ変更を無効化
            CW_USEDEFAULT, CW_USEDEFAULT,    // 初期位置
            rc.right - rc.left, rc.bottom - rc.top, // 初期サイズ
            nullptr, nullptr, hInstance, nullptr
        );


        // TODO: Change to CreateWindowExW(WS_EX_TOPMOST, L"DirectX11LessonV2WindowClass", g_szAppName, WS_POPUP,
        // to default to fullscreen.
        if (!hwnd)
            return 1;
        // ウィンドウを表示する
        ShowWindow(hwnd, nCmdShow);
        // デフォルトでフルスクリーンにするにはnCmdShowをSW_SHOWMAXIMIZEDに変更する
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(g_game.get()));
        // クライアント領域を取得する
        GetClientRect(hwnd, &rc);
        // ゲームクラスを初期化する
        g_game->Initialize(hwnd, rc.right - rc.left, rc.bottom - rc.top);
        if (s_fullscreen) g_game->SetFullscreenState(TRUE);
    }

    // メッセージループ
    MSG msg = {};
    while (WM_QUIT != msg.message)
    {
        // メッセージをピークする
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            // メッセージをトランスレートする
            TranslateMessage(&msg);
            // メッセージを送信する
            DispatchMessage(&msg);
        }
        else
        {
            // Tick関数を呼び出す
            g_game->Tick();
        }
    }

    // ゲームをリセットする
    if (s_fullscreen) g_game->SetFullscreenState(FALSE);
    g_game.reset();

    CoUninitialize();
    // メッセージのパラメータを返す
    return static_cast<int>(msg.wParam);
}

// ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    //  ImGuiのメッセージ処理
    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    static bool s_in_sizemove = false;
    static bool s_in_suspend = false;
    static bool s_minimized = false;
    static bool s_fullscreen = false;
    // デフォルトでフルスクリーンに設定されている場合、s_fullscreen を true に設定する.

    auto game = reinterpret_cast<Game*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
        case WM_PAINT:
            if (s_in_sizemove && game)
            {
                game->Tick();
            }
            else
            {
                PAINTSTRUCT ps;
                std::ignore = BeginPaint(hWnd, &ps);
                EndPaint(hWnd, &ps);
            }
            break;

        case WM_DISPLAYCHANGE:
            if (game)
            {
                game->OnDisplayChange();
            }
            break;

        case WM_MOVE:
            if (game)
            {
                game->OnWindowMoved();
            }
            break;

        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED)
            {
                if (!s_minimized)
                {
                    s_minimized = true;
                    if (!s_in_suspend && game)
                        game->OnSuspending();
                    s_in_suspend = true;
                }
            }
            else if (s_minimized)
            {
                s_minimized = false;
                if (s_in_suspend && game)
                    game->OnResuming();
                s_in_suspend = false;
            }
            //else if (!s_in_sizemove && game)
            //{
            //    // ウィンドウのサイズが変更された場合に次の関数を呼び出す
            //    game->OnWindowSizeChanged(LOWORD(lParam), HIWORD(lParam));
            //}
            break;

        case WM_ENTERSIZEMOVE:
            s_in_sizemove = true;
            break;

        case WM_EXITSIZEMOVE:
            s_in_sizemove = false;
            if (game)
            {
                RECT rc;
                // クライアント領域を取得する
                GetClientRect(hWnd, &rc);
                // ウィンドウサイズが変更された場合に次の関数を呼び出す
                game->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
            }
            break;

        case WM_GETMINMAXINFO:
            if (lParam)
            {
                auto info = reinterpret_cast<MINMAXINFO*>(lParam);
                // ウィンドウの最小横幅を 1280 ピクセルに設定
                info->ptMinTrackSize.x = 1280;
                // ウィンドウの最小高さを 720 ピクセルに設定
                info->ptMinTrackSize.y = 720;
            }
            break;

        case WM_ACTIVATEAPP:
            // アプリケーションがアクティブまたは非アクティブになった場合
            DirectX::Mouse::ProcessMessage(message, wParam, lParam);
            DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
            if (game)
            {
                if (wParam)
                {
                    game->OnActivated();
                }
                else
                {
                    game->OnDeactivated();
                }
            }
            break;

        case WM_POWERBROADCAST:
            switch (wParam)
            {
                case PBT_APMQUERYSUSPEND:
                    if (!s_in_suspend && game)
                        game->OnSuspending();
                    s_in_suspend = true;
                    return TRUE;

                case PBT_APMRESUMESUSPEND:
                    if (!s_minimized)
                    {
                        if (s_in_suspend && game)
                            game->OnResuming();
                        s_in_suspend = false;
                    }
                    return TRUE;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_SYSKEYDOWN:
            if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
            {
                // ALT+ENTER  フルスクリーン切り替えを実装する
                if (s_fullscreen)
                {
                    SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
                    SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);

                    int width = 1280;
                    int height = 720;
                    if (game)
                        game->GetDefaultSize(width, height);

                    ShowWindow(hWnd, SW_SHOWNORMAL);

                    SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
                }
                else
                {
                    SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);
                    SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

                    SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

                    ShowWindow(hWnd, SW_SHOWMAXIMIZED);
                }

                s_fullscreen = !s_fullscreen;
            }
            break;

        case WM_MENUCHAR:
            // メニューがアクティブでユーザーが任意のニーモニックまたはアクセラレータキーに対応しないキーを押した場合に
                 // エラービープを発生させないように無視する
            return MAKELRESULT(0, MNC_CLOSE);

        case WM_INPUT:
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MOUSEWHEEL:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_MOUSEHOVER:
            DirectX::Mouse::ProcessMessage(message, wParam, lParam);
            break;

        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
            break;

        case WM_MOUSEACTIVATE:
            // クリックしてウィンドウをアクティブにすると、マウスはそのイベントを無視する
            return MA_ACTIVATEANDEAT;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

// Exit helper
void ExitGame() noexcept
{
    PostQuitMessage(0);
}
