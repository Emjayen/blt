/*
 * blt.cpp
 *
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>





HDC hdcBackBuffer;
HBITMAP hbmBackBuffer;
DWORD* pPixels;
LONG BackBufferPitch;
LONG BackBufferWidth;
LONG BackBufferHeight;



void ResizeBackBuffer(LONG cx, WORD cy)
{
    BITMAPINFO bmi = {};

    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = cx;
    bmi.bmiHeader.biHeight = ~cy + 1;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    hbmBackBuffer = CreateDIBSection(hdcBackBuffer, &bmi, DIB_RGB_COLORS, (void**) &pPixels, NULL, 0);

    DeleteObject(SelectObject(hdcBackBuffer, hbmBackBuffer));

    BackBufferWidth = cx;
    BackBufferHeight = cy;
    BackBufferPitch = (BackBufferWidth+3) & ~3;
}


void Present(HDC hdc, RECT* rcDst)
{
    BitBlt(hdc, rcDst->left, rcDst->top, rcDst->right - rcDst->left, rcDst->bottom - rcDst->top, hdcBackBuffer, 0, 0, SRCCOPY);
}


void Render(HWND hwnd)
{
    static POINT pt;

    if(++pt.x >= BackBufferWidth || ++pt.y >= BackBufferHeight)
    {
        pt.x = 0;
        pt.y = 0;
    }

    pPixels[pt.y*BackBufferPitch+pt.x] = RGB(0, 0xFF, 0);

    RECT rcDirty = { pt.x, pt.y, 1, 1 };
    InvalidateRect(hwnd, &rcDirty, FALSE);
}


LRESULT CALLBACK OnMessage(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch(Msg)
    {
        case WM_CREATE:
        {
            hdcBackBuffer = CreateCompatibleDC(GetDC(hwnd));
            return 0;
        }

        case WM_ERASEBKGND:
        {
            return TRUE;
        } break;

        case WM_SIZE:
        {
            ResizeBackBuffer(LOWORD(lParam), HIWORD(lParam));
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            Present(ps.hdc, &ps.rcPaint);
            EndPaint(hwnd, &ps);

            return 0;
        } break;

        case WM_MOUSEMOVE:
        {
            // 
            Render(hwnd);
        } break;
    }

    return DefWindowProc(hwnd, Msg, wParam, lParam);
}



int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, int Show)
{
    WNDCLASSEX wcx = {};
    wcx.cbSize = sizeof(wcx);
    wcx.hInstance = hInstance;
    wcx.lpfnWndProc = &OnMessage;
    wcx.lpszClassName = "Test";

    RegisterClassEx(&wcx);
    CreateWindowEx(WS_EX_APPWINDOW, wcx.lpszClassName, "Test", WS_VISIBLE | WS_OVERLAPPED, 0, 0, 800, 600, NULL, NULL, hInstance, NULL);


    for(MSG msg;;)
    {
        GetMessage(&msg, NULL, 0, 0);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 1;
}