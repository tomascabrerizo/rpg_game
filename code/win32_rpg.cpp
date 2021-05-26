#include <Windows.h>
#include <Dsound.h>
#include <stdint.h>
#include <math.h>

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

#define PI32 3.14159265359f

global b32 globalRunning;
global BITMAPINFO globalBitmapInfo;
global void *globalBackBuffer;
global int backBufferWidth = 1280;
global int backBufferHeihgt = 720;
global int bytesPerPixel = 4;
global int backBufferPitch = (backBufferWidth*bytesPerPixel); 

global LPDIRECTSOUNDBUFFER globalSecondaryBuffer;
global DWORD globalSamplesPerSecond = 44100;
global int globalBytesPerSample = sizeof(i16)*2;
global int globalSecondaryBufferSize = globalSamplesPerSecond * globalBytesPerSample;

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter )
typedef DIRECT_SOUND_CREATE(DSoundCreate);

internal void
Win32WriteSineWave()
{
    r32 tSine = 0;
    int toneHz = 512;
    int wavePeriod = globalSamplesPerSecond / toneHz;
    int volume = 500;

    DWORD playCursor;
    DWORD writeCursor;
    if(globalSecondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor) == DS_OK)
    {
        DWORD bytesToWrite = globalSecondaryBufferSize; 
        DWORD byteToLock = 0;
        VOID *audioArea1;
        DWORD audioArea1Bytes;
        VOID *audioArea2;
        DWORD audioArea2Bytes;
        if(globalSecondaryBuffer->Lock(byteToLock, bytesToWrite, 
            &audioArea1, &audioArea1Bytes, 
            &audioArea2, &audioArea2Bytes, 
            DSBLOCK_ENTIREBUFFER) == DS_OK)
        {
            i16 *byteSample = (i16 *)audioArea1; 
            DWORD audioArea1Samples = (audioArea1Bytes / globalBytesPerSample);
            for(DWORD areaIndex = 0; areaIndex < audioArea1Samples; ++areaIndex)
            {
                *byteSample++ = (i16)(sinf(tSine) * volume);
                *byteSample++ = (i16)(sinf(tSine) * volume);

                tSine += 2.0f*PI32*(1.0f/(r32)wavePeriod);
                if(tSine > 2.0f*PI32)
                {
                    tSine -= (2.0f*PI32);
                }
            }
            
            byteSample = (i16 *)audioArea2;
            DWORD audioArea2Samples = (audioArea2Bytes / globalBytesPerSample);
            for(DWORD areaIndex = 0; areaIndex < audioArea2Samples; ++areaIndex)
            {
                *byteSample++ = (i16)(sinf(tSine) * volume);
                *byteSample++ = (i16)(sinf(tSine) * volume);

                tSine += 2.0f*PI32*(1.0f/(r32)wavePeriod);
                if(tSine > 2.0f*PI32)
                {
                    tSine -= (2.0f*PI32);
                }
            }

            if(globalSecondaryBuffer->Unlock(audioArea1, audioArea1Bytes, audioArea2, audioArea2Bytes) == DS_OK)
            {
                int breakHere = 0;
            }
        }
    }
}

internal void
Win32LoadDirectSound(HWND window, DWORD samplesPerSecond, DWORD bufferSize)
{
    HMODULE directSoundLibrary = LoadLibraryA("dsound.dll");
    if(directSoundLibrary)
    {
        DSoundCreate *DirectSoundCreate = 
            (DSoundCreate *)GetProcAddress(directSoundLibrary, "DirectSoundCreate");
        
        LPDIRECTSOUND DirectSound;
        if(DirectSoundCreate && (DirectSoundCreate(0, &DirectSound, 0) == DS_OK))
        {
            if(DirectSound->SetCooperativeLevel(window, DSSCL_PRIORITY) == DS_OK)
            {
                WAVEFORMATEX waveFormat = {};
                waveFormat.wFormatTag = WAVE_FORMAT_PCM;
                waveFormat.nChannels = 2;
                waveFormat.nSamplesPerSec = samplesPerSecond;
                waveFormat.wBitsPerSample = 16;
                waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample/8);
                waveFormat.nAvgBytesPerSec = (waveFormat.nSamplesPerSec * waveFormat.nBlockAlign);

                DSBUFFERDESC primaryBufferDescription = {};
                primaryBufferDescription.dwSize = sizeof(primaryBufferDescription);
                primaryBufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

                LPDIRECTSOUNDBUFFER primaryBuffer;
                if(DirectSound->CreateSoundBuffer(&primaryBufferDescription, &primaryBuffer, 0) == DS_OK)
                {
                    primaryBuffer->SetFormat(&waveFormat);
                    OutputDebugString("DSOUND: Primary Buffer Created\n");
                }
                
                DSBUFFERDESC secondaryBufferDescription = {};
                secondaryBufferDescription.dwSize = sizeof(secondaryBufferDescription);
                secondaryBufferDescription.dwFlags = 0;
                secondaryBufferDescription.dwBufferBytes = bufferSize;
                secondaryBufferDescription.lpwfxFormat = &waveFormat;
                if(DirectSound->CreateSoundBuffer(&secondaryBufferDescription, &globalSecondaryBuffer, 0) == DS_OK)
                {
                    OutputDebugString("DSOUND: Secondary Buffer Created\n");
                }
            }
            else
            {
                // TODO: Log error message!
            }
        }
        else
        {
            // TODO: Log error message!
        }

    }
    else
    {
        // TODO: Log error message!
    }
}

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
        
        Win32LoadDirectSound(window, globalSamplesPerSecond, globalSecondaryBufferSize);
        
        Win32WriteSineWave();
        globalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

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
