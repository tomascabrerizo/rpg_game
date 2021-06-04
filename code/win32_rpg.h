#ifndef WIN32_RPG_H
#define WIN32_RPG_H

#include <Windows.h>
#include <Dsound.h>

struct Win32BackBuffer
{
    BITMAPINFO bitmapInfo;
    int width;
    int height;
    int bytesPerPixel;
    int pitch; 
    void *memory;
};

struct Win32SoundBuffer
{
    LPDIRECTSOUNDBUFFER dsoundBuffer;
    DWORD samplesPerSecond;
    int bytesPerSample;
    int size;
};


#define DEBUG_FRAMES 30
struct Win32SoundDebugInfo
{
    int byteToLockPosition[DEBUG_FRAMES];
    int bytesToWriteWidth[DEBUG_FRAMES];
    int debugIndex;
};

#endif //WIN32_RPG_H
