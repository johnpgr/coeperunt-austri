#ifndef PLATFORM_WIN32_H
#define PLATFORM_WIN32_H

#include "../core.h"

// Detect if Windows SDK headers have been loaded to prevent redefinition conflicts
#if defined(_WINDOWS_) || defined(_INC_WINDOWS) || defined(_WINDOWS_H) || defined(CS_HREDRAW)
#define PLATFORM_WIN32_SDK_INCLUDED 1
#endif

#if !defined(PLATFORM_WIN32_SDK_INCLUDED)

// ============================================================================
// Basic Win32 Type Definitions
// ============================================================================
typedef unsigned long DWORD;
typedef int BOOL;
typedef void* HANDLE;
typedef void* HINSTANCE;
typedef void* HWND;
typedef void* HBRUSH;
typedef void* HCURSOR;
typedef void* HMODULE;
typedef long long INT_PTR;
typedef unsigned long long UINT_PTR;
typedef INT_PTR LRESULT;
typedef UINT_PTR WPARAM;
typedef INT_PTR LPARAM;
typedef unsigned int UINT;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef void* HDC;
typedef void* HGLRC;
typedef float FLOAT;
typedef INT_PTR (__stdcall* PROC)(void);

struct PIXELFORMATDESCRIPTOR {
    WORD  nSize;
    WORD  nVersion;
    DWORD dwFlags;
    BYTE  iPixelType;
    BYTE  cColorBits;
    BYTE  cRedBits;
    BYTE  cRedShift;
    BYTE  cGreenBits;
    BYTE  cGreenShift;
    BYTE  cBlueBits;
    BYTE  cBlueShift;
    BYTE  cAlphaBits;
    BYTE  cAlphaShift;
    BYTE  cAccumBits;
    BYTE  cAccumRedBits;
    BYTE  cAccumGreenBits;
    BYTE  cAccumBlueBits;
    BYTE  cAccumAlphaBits;
    BYTE  cDepthBits;
    BYTE  cStencilBits;
    BYTE  cAuxBuffers;
    BYTE  iLayerType;
    BYTE  bReserved;
    DWORD dwLayerMask;
    DWORD dwVisibleMask;
    DWORD dwDamageMask;
};

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef CALLBACK
#define CALLBACK __stdcall
#endif
#ifndef WINAPI
#define WINAPI __stdcall
#endif

typedef const char* LPCSTR;
typedef char* LPSTR;

union LARGE_INTEGER {
    struct {
        DWORD LowPart;
        long HighPart;
    };
    struct {
        DWORD LowPart;
        long HighPart;
    } u;
    long long QuadPart;
};

struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

struct POINT {
    long x;
    long y;
};

struct MSG {
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
    DWORD lPrivate;
};

typedef LRESULT (__stdcall* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

struct WNDCLASSEXA {
    UINT cbSize;
    UINT style;
    WNDPROC lpfnWndProc;
    int cbClsExtra;
    int cbWndExtra;
    HINSTANCE hInstance;
    void* hIcon;
    HCURSOR hCursor;
    HBRUSH hbrBackground;
    const char* lpszMenuName;
    const char* lpszClassName;
    void* hIconSm;
};

// ============================================================================
// Win32 Constant Macros
// ============================================================================
#define INVALID_HANDLE_VALUE ((HANDLE)(INT_PTR)-1)
#define GENERIC_READ         0x80000000
#define GENERIC_WRITE        0x40000000
#define FILE_SHARE_READ      0x00000001
#define OPEN_EXISTING        3
#define CREATE_ALWAYS        2
#define FILE_ATTRIBUTE_NORMAL 0x00000080

#define MEM_COMMIT      0x00001000
#define MEM_RESERVE     0x00002000
#define MEM_RELEASE     0x00008000
#define PAGE_READWRITE  0x04

#define CS_HREDRAW      0x0002
#define CS_VREDRAW      0x0001
#define WS_OVERLAPPED   0x00000000L
#define WS_CAPTION      0x00C00000L
#define WS_SYSMENU      0x00080000L
#define WS_THICKFRAME   0x00040000L
#define WS_MINIMIZEBOX  0x00020000L
#define WS_MAXIMIZEBOX  0x00010000L
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)

#define CW_USEDEFAULT   ((int)0x80000000)
#define SW_SHOW         5
#define COLOR_WINDOW    5
#define IDC_ARROW       ((const char*)(INT_PTR)32512)
#define PM_REMOVE       0x0001

#define WM_DESTROY      0x0002
#define WM_SIZE         0x0005
#define WM_CLOSE        0x0010
#define WM_LBUTTONDOWN  0x0201
#define WM_LBUTTONUP    0x0202
#define WM_RBUTTONDOWN  0x0204
#define WM_RBUTTONUP    0x0205
#define WM_MOUSEMOVE    0x0200

#define LOWORD(l)        ((unsigned short)(((unsigned long)(l)) & 0xffff))
#define HIWORD(l)        ((unsigned short)((((unsigned long)(l)) >> 16) & 0xffff))

#define PFD_TYPE_RGBA                      0
#define PFD_MAIN_PLANE                     0
#define PFD_DOUBLEBUFFER                   0x00000001
#define PFD_DRAW_TO_WINDOW                 0x00000004
#define PFD_SUPPORT_OPENGL                 0x00000020

// ============================================================================
// Static Import DLL Declarations (Kernel32)
// ============================================================================
extern "C" {
    __declspec(dllimport) void* __stdcall VirtualAlloc(void* lpAddress, usize dwSize, DWORD flAllocationType, DWORD flProtect);
    __declspec(dllimport) int __stdcall VirtualFree(void* lpAddress, usize dwSize, DWORD dwFreeType);
    __declspec(dllimport) void __stdcall ExitProcess(UINT uExitCode);
    __declspec(dllimport) void __stdcall OutputDebugStringA(const char* lpOutputString);
    __declspec(dllimport) HANDLE __stdcall CreateFileA(const char* lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, void* lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
    __declspec(dllimport) BOOL __stdcall GetFileSizeEx(HANDLE hFile, LARGE_INTEGER* lpFileSize);
    __declspec(dllimport) BOOL __stdcall ReadFile(HANDLE hFile, void* lpBuffer, DWORD nNumberOfBytesToRead, DWORD* lpNumberOfBytesRead, void* lpOverlapped);
    __declspec(dllimport) BOOL __stdcall WriteFile(HANDLE hFile, const void* lpBuffer, DWORD nNumberOfBytesToWrite, DWORD* lpNumberOfBytesWritten, void* lpOverlapped);
    __declspec(dllimport) BOOL __stdcall CloseHandle(HANDLE hObject);
    __declspec(dllimport) HMODULE __stdcall GetModuleHandleA(const char* lpModuleName);
    __declspec(dllimport) HMODULE __stdcall LoadLibraryA(const char* lpLibFileName);
    __declspec(dllimport) void* __stdcall GetProcAddress(HMODULE hModule, const char* lpProcName);
    __declspec(dllimport) DWORD __stdcall GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
}

#endif // !PLATFORM_WIN32_SDK_INCLUDED

// ============================================================================
// Isolated Dynamic Function Pointer Declarations (User32)
// ============================================================================
namespace win32 {
    typedef int (__stdcall* PFN_AdjustWindowRect)(RECT* lpRect, DWORD dwStyle, BOOL bMenu);
    typedef HWND (__stdcall* PFN_CreateWindowExA)(DWORD dwExStyle, const char* lpClassName, const char* lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, void* hMenu, HINSTANCE hInstance, void* lpParam);
    typedef BOOL (__stdcall* PFN_DestroyWindow)(HWND hWnd);
    typedef LRESULT (__stdcall* PFN_DefWindowProcA)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    typedef LRESULT (__stdcall* PFN_DispatchMessageA)(const MSG* lpMsg);
    typedef HCURSOR (__stdcall* PFN_LoadCursorA)(HINSTANCE hInstance, const char* lpCursorName);
    typedef BOOL (__stdcall* PFN_PeekMessageA)(MSG* lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
    typedef BOOL (__stdcall* PFN_TranslateMessage)(const MSG* lpMsg);
    typedef void (__stdcall* PFN_PostQuitMessage)(int nExitCode);
    typedef unsigned short (__stdcall* PFN_RegisterClassExA)(const WNDCLASSEXA*);
    typedef BOOL (__stdcall* PFN_ShowWindow)(HWND hWnd, int nCmdShow);
    typedef BOOL (__stdcall* PFN_UpdateWindow)(HWND hWnd);
    typedef HWND (__stdcall* PFN_SetCapture)(HWND hWnd);
    typedef BOOL (__stdcall* PFN_ReleaseCapture)(void);

    #ifdef PLATFORM_WIN32_IMPLEMENTATION
        #define WIN32_EXT
    #else
        #define WIN32_EXT extern
    #endif

    WIN32_EXT PFN_AdjustWindowRect AdjustWindowRect;
    WIN32_EXT PFN_CreateWindowExA CreateWindowExA;
    WIN32_EXT PFN_DestroyWindow DestroyWindow;
    WIN32_EXT PFN_DefWindowProcA DefWindowProcA;
    WIN32_EXT PFN_DispatchMessageA DispatchMessageA;
    WIN32_EXT PFN_LoadCursorA LoadCursorA;
    WIN32_EXT PFN_PeekMessageA PeekMessageA;
    WIN32_EXT PFN_TranslateMessage TranslateMessage;
    WIN32_EXT PFN_PostQuitMessage PostQuitMessage;
    WIN32_EXT PFN_RegisterClassExA RegisterClassExA;
    WIN32_EXT PFN_ShowWindow ShowWindow;
    WIN32_EXT PFN_UpdateWindow UpdateWindow;
    WIN32_EXT PFN_SetCapture SetCapture;
    WIN32_EXT PFN_ReleaseCapture ReleaseCapture;

    #undef WIN32_EXT
}

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

#define PLATFORM_LOAD(dll, name) \
    win32::name = (win32::PFN_##name)GetProcAddress(dll, #name)

#endif // PLATFORM_WIN32_H
