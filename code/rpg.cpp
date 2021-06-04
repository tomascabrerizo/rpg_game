#include "rpg.h"
#include "math.h"

internal void
DrawBackBufferPatron(BackBuffer *backBuffer)
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

inline u32
roundR32ToU32(r32 value)
{
    return (u32)roundf(value);
}

inline i32
roundR32ToI32(r32 value)
{
    return (i32)roundf(value);
}

internal void
DrawRect(BackBuffer *backBuffer, 
        r32 minX, r32 minY, r32 maxX, r32 maxY, 
        r32 r, r32 g, r32 b)
{
    i32 uMinX = roundR32ToI32(minX); 
    i32 uMinY = roundR32ToI32(minY);
    i32 uMaxX = roundR32ToI32(maxX);
    i32 uMaxY = roundR32ToI32(maxY);

    if(uMinX < 0)
    {
        uMinX = 0;
    }
    if(uMaxX > backBuffer->width)
    {
        uMaxX = backBuffer->width;
    }
    if(uMinY < 0)
    {
        uMinY = 0;
    }
    if(uMaxY > backBuffer->height)
    {
        uMaxY = backBuffer->height;
    }
    
    u32 color =  ((u32)(255.0f)   << 24) | 
                 ((u32)(r*255.0f) << 16) |
                 ((u32)(g*255.0f) <<  8) |
                 ((u32)(b*255.0f) <<  0); 

    u8 *row = (u8 *)backBuffer->memory + (uMinY * backBuffer->pitch);
    for(int dy = uMinY; dy < uMaxY; ++dy)
    {
        u32 *pixel = (u32 *)row + uMinX;
        for(int dx = uMinX; dx <uMaxX; ++dx)
        {
            *pixel++ = color; 
        }
        row += backBuffer->pitch;
    }
}

internal void
InitMemoryArena(MemoryArena *arena, memory_index size, u8 *base)
{
    Assert(base);
    if(base)
    {
        arena->size = size;
        arena->base = base;
        arena->used = 0;
    }
}

#define PushSize(arena, type) (type *)PushSize_(arena, sizeof(type))
#define PushArray(arena, count, type) (type *)PushSize_(arena, (count)*sizeof(type))
internal void *
PushSize_(MemoryArena *arena, memory_index size)
{
    Assert((arena->used + size) <= arena->size);
    void *result = arena->base + arena->used;
    arena->used += size;
    return result; 
}

internal void
GameUpdateAndRender(BackBuffer *buffer, SoundBuffer *soundBuffer, 
                    Input *input, GameMemory *memory, r32 elapseTime)
{
    GameState *gameState = (GameState *)memory->permanet;
    if(!gameState->isInitialize)
    {
        
        gameState->isInitialize = true;
    }

    DrawBackBufferPatron(buffer);

    DrawRect(buffer, 200, 200, 320, 320, 1.0f, 0.0f, 0.6f);
}
