#include <Windows.h>
#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef int32_t b32;

typedef float  r32; 
typedef double r64; 

#define global static
#define internal static

global b32 globalRunning;
global BITMAPINFO globalBitmapInfo;
global void *globalBackBuffer;
global int backBufferWidth = 1280;
global int backBufferHeihgt = 720;
global int bytesPerPixel = 4;
global int backBufferPitch = (backBufferWidth*bytesPerPixel); 

internal void
Win32DrawBackBufferPatron(void *backBuffer, int width, int height)
{
    u8 *row = (u8 *)backBuffer; 
    for(int y = 0; y < height; ++y)
    {
        u32 *pixel = (u32 *)row;
        for(int x = 0; x < width; ++x)
        {
            u8 color0 = (u8)x;
            u8 color1 = (u8)y;
            *pixel++ =  (u32)(color0 | (color1 << 0)); 
        }
        row += backBufferPitch;
    }
}

internal void
Win32CreateBackBuffer(void **backBuffer, int width, int height)
{
    u32 backBufferSize = (width*height*bytesPerPixel);
    *backBuffer = VirtualAlloc(0, backBufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    
    globalBitmapInfo.bmiHeader.biSize = sizeof(globalBitmapInfo.bmiHeader);
    globalBitmapInfo.bmiHeader.biWidth = width;
    globalBitmapInfo.bmiHeader.biHeight = -height;
    globalBitmapInfo.bmiHeader.biPlanes = 1;
    globalBitmapInfo.bmiHeader.biBitCount = 32;
    globalBitmapInfo.bmiHeader.biCompression = BI_RGB;
}

internal void
Win32UpdateBackBuffer(HDC deviceContext, void *backBuffer, int width, int height)
{
    StretchDIBits(
        deviceContext,
        0, 0,
        width, height,
        0, 0,
        width, height,
        globalBackBuffer,
        &globalBitmapInfo,
        DIB_RGB_COLORS,
        SRCCOPY
    );
}

internal
LRESULT CALLBACK Win32WindowsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    LRESULT result = 0;
    
    switch(uMsg)
    {
        case WM_CREATE:
        {

        }break;
        case WM_SIZE:
        {

        }break;
        case WM_CLOSE:
        {
            globalRunning = false;
        }break;
        default: 
        {
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        }break;
    }

    return result;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    WNDCLASSA win32WindowClass = {};
    win32WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    win32WindowClass.lpfnWndProc = Win32WindowsProc;
    win32WindowClass.hInstance = hInstance;
    //win32WindowClass.hIcon = 0;
    //win32WindowClass.hCursor = 0;
    //win32WindowClass.hbrBackground = 0;
    //win32WindowClass.lpszMenuName = 0;
    win32WindowClass.lpszClassName = "win32WindowClassName";

    RegisterClassA(&win32WindowClass);

    HWND window = CreateWindowExA(
        0,
        win32WindowClass.lpszClassName,
        "rpg_game",
        WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0, 
        0, 
        hInstance,    
        0
    );

    if(window)
    {
        // TODO: Maybe make the device context global if it needed
        HDC deviceContext = GetDC(window);
        Win32CreateBackBuffer(&globalBackBuffer, backBufferWidth, backBufferHeihgt);
        
        globalRunning = true;
        while(globalRunning)
        {
            MSG message;
            while(PeekMessage(&message, window, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&message); 
                DispatchMessage(&message); 
            }
            
            Win32DrawBackBufferPatron(globalBackBuffer, backBufferWidth, backBufferHeihgt);
            Win32UpdateBackBuffer(deviceContext, globalBackBuffer, backBufferWidth, backBufferHeihgt);
        }

        ReleaseDC(window, deviceContext);
    }
    else
    {
        // TODO: Log error message!
    }

    return 0;
}
