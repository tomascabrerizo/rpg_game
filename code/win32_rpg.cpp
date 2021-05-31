#include "win32_rpg.h"

global b32 globalRunning;
global Win32BackBuffer globalBackBuffer;
global Win32SoundBuffer globalSecondaryBuffer;
global i64 globalPerformanceFrequency;

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter )
typedef DIRECT_SOUND_CREATE(DSoundCreate);

internal void
Win32WriteSineWave(Win32SoundBuffer *soundBuffer)
{
    r32 tSine = 0;
    int toneHz = 512;
    int wavePeriod = soundBuffer->samplesPerSecond / toneHz;
    int volume = 2000;

    DWORD playCursor;
    DWORD writeCursor;
    if(soundBuffer->dsoundBuffer->GetCurrentPosition(&playCursor, &writeCursor) == DS_OK)
    {
        DWORD bytesToWrite = soundBuffer->size; 
        DWORD byteToLock = 0;
        VOID *audioArea1;
        DWORD audioArea1Bytes;
        VOID *audioArea2;
        DWORD audioArea2Bytes;
        if(soundBuffer->dsoundBuffer->Lock(byteToLock, bytesToWrite, 
            &audioArea1, &audioArea1Bytes, 
            &audioArea2, &audioArea2Bytes, 
            DSBLOCK_ENTIREBUFFER) == DS_OK)
        {
            i16 *byteSample = (i16 *)audioArea1; 
            DWORD audioArea1Samples = (audioArea1Bytes / soundBuffer->bytesPerSample);
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
            DWORD audioArea2Samples = (audioArea2Bytes / soundBuffer->bytesPerSample);
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
            soundBuffer->dsoundBuffer->Unlock(audioArea1, audioArea1Bytes, audioArea2, audioArea2Bytes);
        }
    }
}

internal void
Win32LoadDirectSound(HWND window, Win32SoundBuffer *soundBuffer, DWORD samplesPerSecond)
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
                waveFormat.nBlockAlign = (waveFormat.nChannels * (waveFormat.wBitsPerSample/8));
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
                
                int bufferSize = samplesPerSecond * sizeof(i16)*2;

                DSBUFFERDESC secondaryBufferDescription = {};
                secondaryBufferDescription.dwSize = sizeof(secondaryBufferDescription);
                secondaryBufferDescription.dwFlags = 0;
                secondaryBufferDescription.dwBufferBytes = bufferSize;
                secondaryBufferDescription.lpwfxFormat = &waveFormat;
                if(DirectSound->CreateSoundBuffer(&secondaryBufferDescription, &soundBuffer->dsoundBuffer, 0) == DS_OK)
                {
                    OutputDebugString("DSOUND: Secondary Buffer Created\n");
                    soundBuffer->samplesPerSecond = samplesPerSecond;
                    soundBuffer->bytesPerSample = sizeof(i16)*2;
                    soundBuffer->size = bufferSize;
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
Win32DrawBackBufferPatron(Win32BackBuffer *backBuffer)
{
    u8 *row = (u8 *)backBuffer->memory; 
    for(int y = 0; y < backBuffer->height; ++y)
    {
        u32 *pixel = (u32 *)row;
        for(int x = 0; x < backBuffer->width; ++x)
        {
            u8 color0 = (u8)x;
            u8 color1 = (u8)y;
            *pixel++ =  (u32)(color0 | (color1 << 0)); 
        }
        row += backBuffer->pitch;
    }
}

internal void
Win32CreateBackBuffer(Win32BackBuffer *backBuffer, int width, int height)
{
    backBuffer->width = width;
    backBuffer->height = height;
    backBuffer->bytesPerPixel = 4;
    backBuffer->pitch = (backBuffer->width*backBuffer->bytesPerPixel);
    u32 backBufferSize = (backBuffer->width*backBuffer->height*backBuffer->bytesPerPixel);
    
    backBuffer->memory = VirtualAlloc(0, backBufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    
    backBuffer->bitmapInfo.bmiHeader.biSize = sizeof(backBuffer->bitmapInfo.bmiHeader);
    backBuffer->bitmapInfo.bmiHeader.biWidth = width;
    backBuffer->bitmapInfo.bmiHeader.biHeight = -height;
    backBuffer->bitmapInfo.bmiHeader.biPlanes = 1;
    backBuffer->bitmapInfo.bmiHeader.biBitCount = 32;
    backBuffer->bitmapInfo.bmiHeader.biCompression = BI_RGB;
}

internal void
Win32UpdateBackBuffer(HDC deviceContext, Win32BackBuffer *backBuffer)
{
    StretchDIBits(
        deviceContext,
        0, 0,
        backBuffer->width, backBuffer->height,
        0, 0,
        backBuffer->width, backBuffer->height,
        backBuffer->memory,
        &backBuffer->bitmapInfo,
        DIB_RGB_COLORS,
        SRCCOPY
    );
}

internal LARGE_INTEGER
Win32GetWallClock()
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result;
}

internal r32
Win32GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
    r32 result = ((r32)(end.QuadPart - start.QuadPart) / (r32)globalPerformanceFrequency);
    return result;
}

internal LRESULT CALLBACK 
Win32WindowsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

internal void
Win32DebugDrawRect(Win32BackBuffer *backBuffer, int x, int y, int width, int height, u32 color)
{
    int minX = x;
    int minY = y;
    int maxX = x + width;
    int maxY = y + height;

    if(minX < 0)
    {
        minX = 0;
    }
    if(maxX > backBuffer->width)
    {
        maxX = backBuffer->width;
    }
    if(minY < 0)
    {
        minY = 0;
    }
    if(maxY > backBuffer->height)
    {
        maxY = backBuffer->height;
    }
    
    u8 *row = (u8 *)backBuffer->memory + (minY * backBuffer->pitch);
    for(int dy = minY; dy < maxY; ++dy)
    {
        u32 *pixel = (u32 *)row + minX;
        for(int dx = minX; dx <maxX; ++dx)
        {
            *pixel++ = color; 
        }
        row += backBuffer->pitch;
    }
}

internal void
Win32DrawSoundDebufInfo(Win32SoundDebugInfo *info, DWORD byteToLock, DWORD bytesToWrite, DWORD playCursor, DWORD writeCursor)
{
    // TODO: Define a DEBUG flag for this kind of code
    int secondaryBufferHeight = 200;
    // NOTE: This ratio maps the secondary buffer size in bytes to the screen width
    r32 ratio = (r32)(globalBackBuffer.width - 2*80) / (r32)globalSecondaryBuffer.size;
    int secondaryBufferWidth = (int)(ratio * (r32)globalSecondaryBuffer.size);
    info->byteToLockPosition[info->debugIndex] =  (int)(ratio * (r32)byteToLock);
    info->bytesToWriteWidth[info->debugIndex] = (int)(ratio * (r32)bytesToWrite);
    
    Win32DebugDrawRect(&globalBackBuffer, 
        80, 200, secondaryBufferWidth, secondaryBufferHeight, 0xFF999999);
    
    for(int index = 0; index < DEBUG_FRAMES; ++index)
    {
        Win32DebugDrawRect(&globalBackBuffer, 
            80 + info->byteToLockPosition[index], 220, info->bytesToWriteWidth[index]  , 160, 0xFFFFFFFF);
    }
    Win32DebugDrawRect(&globalBackBuffer, 80 +(int)(ratio * (r32)playCursor), 220, 10, 160, 0xFFFF0000);
    Win32DebugDrawRect(&globalBackBuffer, 80 +(int)(ratio * (r32)writeCursor), 220, 10, 160, 0xFF00FF00);
    Win32DebugDrawRect(&globalBackBuffer, 80 +(int)(ratio * (r32)byteToLock), 220, 10, 160, 0xFFFFFF00);
    if(++info->debugIndex == DEBUG_FRAMES)
    {
        info->debugIndex = 0;

        for(int index = 0; index < DEBUG_FRAMES; ++index)
        {
            info->byteToLockPosition[index] = {};
            info->bytesToWriteWidth[index] = {};
        }
    }
}

int CALLBACK 
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
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
        
        Win32LoadDirectSound(window, &globalSecondaryBuffer,44100);
        globalSecondaryBuffer.dsoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
        Win32CreateBackBuffer(&globalBackBuffer, 1280, 720);
       
        // NOTE(tomi): Set Window Sleep granularity to 1 millisecond
        UINT DesiredSchedulerMS = 1;
        b32 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);


        LARGE_INTEGER performanceFrequency;
        QueryPerformanceFrequency(&performanceFrequency);
        globalPerformanceFrequency = performanceFrequency.QuadPart;

        int monitorHz = 30;
        r32 targetSecPerFrame = 1.0f / (r32)monitorHz;
        LARGE_INTEGER lastTime;
        QueryPerformanceCounter(&lastTime);
        
        // TODO: Maybe some of this should be in the soundBuffer struct
        DWORD runningSampleIndex = 0;
        b32 firstSoundWrite = true;

        r32 tSine = 0;
        int toneHz = 255;
        int wavePeriod = globalSecondaryBuffer.samplesPerSecond / toneHz;
        int volume = 500;
        Win32SoundDebugInfo DEBUGSoundInfo = {};
        
        globalRunning = true;
        while(globalRunning)
        {
            MSG message;
            while(PeekMessage(&message, window, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&message); 
                DispatchMessage(&message); 
            }
             
            DWORD playCursor;
            DWORD writeCursor;
            if(globalSecondaryBuffer.dsoundBuffer->GetCurrentPosition(&playCursor, &writeCursor) == DS_OK)
            {
                DWORD safeBytes = 0;
                DWORD safeWriteCursor = writeCursor;
                if(safeWriteCursor < playCursor)
                {
                    safeWriteCursor += globalSecondaryBuffer.size;
                }
                if(firstSoundWrite)
                {
                    runningSampleIndex = writeCursor/globalSecondaryBuffer.bytesPerSample;
                    firstSoundWrite = false;
                }
                DWORD byteToLock = (runningSampleIndex*globalSecondaryBuffer.bytesPerSample) % globalSecondaryBuffer.size;
                DWORD targetCursor = writeCursor + 
                    (DWORD)((r32)globalSecondaryBuffer.size * (targetSecPerFrame*3)) + safeBytes;
                DWORD bytesToWrite = 0;
                if(byteToLock > targetCursor)
                {
                    bytesToWrite = (globalSecondaryBuffer.size - byteToLock);
                    bytesToWrite += playCursor;
                }
                if(byteToLock < targetCursor)
                {
                    bytesToWrite = targetCursor - byteToLock;
                }

                VOID *audioArea1;
                DWORD audioArea1Bytes;
                VOID *audioArea2;
                DWORD audioArea2Bytes;
                if(globalSecondaryBuffer.dsoundBuffer->Lock(byteToLock, bytesToWrite, 
                    &audioArea1, &audioArea1Bytes, 
                    &audioArea2, &audioArea2Bytes, 
                    0) == DS_OK)
                {
                    i16 *byteSample = (i16 *)audioArea1;
                    DWORD audioArea1Samples = (audioArea1Bytes / globalSecondaryBuffer.bytesPerSample);
                    for(DWORD areaIndex = 0; areaIndex < audioArea1Samples; ++areaIndex)
                    {
                    // TODO: Re enable this code to copy a sound buffer from the game
                    #if 0
                        *byteSample++ = (i16)(sinf(tSine) * volume);
                        *byteSample++ = (i16)(sinf(tSine) * volume);

                        tSine += 2.0f*PI32*(1.0f/(r32)wavePeriod);
                        if(tSine > 2.0f*PI32)
                        {
                            tSine -= (2.0f*PI32);
                        }
                    #endif
                        ++runningSampleIndex;
                    }

                    byteSample = (i16 *)audioArea2;
                    DWORD audioArea2Samples = (audioArea2Bytes / globalSecondaryBuffer.bytesPerSample);
                    for(DWORD areaIndex = 0; areaIndex < audioArea2Samples; ++areaIndex)
                    {
                    // TODO: Re enable this code to copy a sound buffer from the game
                    #if 0
                        *byteSample++ = (i16)(sinf(tSine) * volume);
                        *byteSample++ = (i16)(sinf(tSine) * volume);

                        tSine += 2.0f*PI32*(1.0f/(r32)wavePeriod);
                        if(tSine > 2.0f*PI32)
                        {
                            tSine -= (2.0f*PI32);
                        }
                    #endif
                        ++runningSampleIndex;
                    }
                    
                    globalSecondaryBuffer.dsoundBuffer->Unlock(audioArea1, audioArea1Bytes, audioArea2, audioArea2Bytes);
                }
                Win32DrawBackBufferPatron(&globalBackBuffer);
                Win32DrawSoundDebufInfo(&DEBUGSoundInfo, byteToLock, bytesToWrite, playCursor, writeCursor);
                
                // TODO: Move this code to a DEBUG function
            }
            
            // TODO: Probably get the target secons per frame from this calculation
            LARGE_INTEGER currentTime = Win32GetWallClock();
            r32 currentSecPerFrame = Win32GetSecondsElapsed(lastTime, currentTime); 
            
            while(currentSecPerFrame < targetSecPerFrame)
            {
                DWORD msToSleep = (DWORD)((targetSecPerFrame - currentSecPerFrame) * 1000.0f);
                Sleep(msToSleep);
                QueryPerformanceCounter(&currentTime);
                currentSecPerFrame = Win32GetSecondsElapsed(lastTime, currentTime); 
            }
            lastTime.QuadPart = currentTime.QuadPart;
            
            Win32UpdateBackBuffer(deviceContext, &globalBackBuffer);
#if 0
            char Buffer[64];
            sprintf_s(Buffer, "ms: %f\n", currentSecPerFrame*1000.0f);
            OutputDebugString(Buffer);
#endif
        }

        ReleaseDC(window, deviceContext);
    }
    else
    {
        // TODO: Log error message!
    }

    return 0;
}
