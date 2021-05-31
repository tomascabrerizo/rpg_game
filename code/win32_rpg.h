#ifndef WIN32_RPG_H
#define WIN32_RPG_H

#include <Windows.h>
#include <Dsound.h>
#include <stdio.h>
#include <math.h>
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

#define PI32 3.14159265359f

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

struct Key
{
    b32 wasPress;
    b32 wasRelease;
    b32 isDown;
};

struct Input
{
    Key keyW;
    Key keyS;
    Key keyA;
    Key keyD;
};

#endif //WIN32_RPG_H
