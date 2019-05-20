#if !defined(COMPILER_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jesse Calvert $
   ======================================================================== */
#include <iostream>
#include <stdio.h>
using namespace std;

#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>
    
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef size_t memory_index;
    
typedef float real32;
typedef double real64;

#define Real32Maximum FLT_MAX
#define UINT32MAX 4294967295
    
#define internal static
#define local_persist static
#define global_variable static

#define ArrayCount(Value) (sizeof(Value) / sizeof((Value)[0]))

#define Assert(Value) if(!(Value)) {*(int *)0 = 0;}
#define InvalidCodePath Assert(0)

#define Kilobytes(Value) (Value)*1024LL
#define Megabytes(Value) Kilobytes(Value)*1024LL
#define Gigabytes(Value) Megabytes(Value)*1024LL
#define Terabytes(Value) Gigabytes(Value)*1024LL

struct memory_arena
{
    memory_index Size;
    uint8* Base;
    memory_index Used;

    int32 TempCount;
};

struct temporary_memory
{
    memory_arena *Arena;
    memory_index Used;
};

inline void
InitializeArena(memory_arena *Arena, memory_index Size, void *Base)
{
    Arena->Size = Size;
    Arena->Base = (uint8 *)Base;
    Arena->Used = 0;
    Arena->TempCount = 0;
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type))
#define PushSize(Arena, Size) PushSize_(Arena, Size)
inline void *
PushSize_(memory_arena *Arena, memory_index Size)
{
    Assert((Arena->Used + Size) <= Arena->Size);
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;

    return Result;
}

inline temporary_memory
BeginTemporaryMemory(memory_arena *Arena)
{
    temporary_memory Result;

    Result.Arena = Arena;
    Result.Used = Arena->Used;

    ++Arena->TempCount;

    return Result;
}

inline void
EndTemporaryMemory(temporary_memory TempMem)
{
    memory_arena *Arena = TempMem.Arena;
    Assert(Arena->Used >= TempMem.Used);
    Arena->Used = TempMem.Used;
    Assert(Arena->TempCount > 0);
    --Arena->TempCount;
}

inline void
CheckArena(memory_arena *Arena)
{
    Assert(Arena->TempCount == 0);
}

#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance))
inline void
ZeroSize(memory_index Size, void *Ptr)
{
    uint8 *Byte = (uint8 *)Ptr;
    while(Size--)
    {
        *Byte++ = 0;
    }
}

inline void
CopyMemory(void *From, void *To, memory_index Size)
{
    uint8 *Dest = (uint8 *)To;
    uint8 *Source = (uint8 *)From;
    while(Size--)
    {
        *Dest++ = *Source++;
    }
}

inline void
SetString(char *A, char *B, uint32 CharCount)
{
    for(uint32 Index = 0;
        Index < CharCount;
        ++Index)
    {
        A[Index] = B[Index];
    }
}

#include "finite_automata.h"

struct regex
{
    char String[128];
    dfa_table Table;
};

#define COMPILER_H
#endif
