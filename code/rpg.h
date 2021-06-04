#ifndef RPG_H
#define RPG_H

#include <stdio.h>
#include <math.h>
#include <stdint.h>

#define Assert(condition) if(!(condition)) { *(u32 *)0 = 0; } 

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabyytes(Value)*1024LL)
#define Terabyytes(Value) (terabytes(Value)*1024LL)

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef int32_t b32;
typedef size_t memory_index; 

typedef float  r32; 
typedef double r64; 

#define global static
#define internal static

#define PI32 3.14159265359f

struct BackBuffer
{
    int width;
    int height;
    int bytesPerPixel;
    int pitch; 
    void *memory;
};

struct SoundBuffer
{
    u32 samplesPerSecond;
    int bytesPerSample;
    int size;
    void *memory;
};

struct Key
{
    b32 wasPress;
    b32 wasRelease;
    b32 isDown;
};

#define NUM_OF_KEYS 5
struct Input
{
    union 
    {
        struct
        {
            Key W;
            Key S;
            Key A;
            Key D;
            Key Space; 
        };
        Key keys[NUM_OF_KEYS];
    };
};

struct MemoryArena
{
    memory_index size;
    u8 *base;
    memory_index used;
};

struct GameMemory
{
    void *permanet;
    void *transient;
    memory_index permanentSize;
    memory_index transientSize;
};

struct GameState
{
    b32 isInitialize;
    r32 playerX;
    r32 playerY;
};

internal void
GameUpdateAndRender(BackBuffer *buffer, SoundBuffer *soundBuffer, 
                    Input *input, GameMemory *memory, r32 elapseTime);

#endif //RPG_H
