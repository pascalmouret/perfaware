#pragma once

#include "../sanity.h"

#include <cstdlib>

// 8086 can access 1MB of memory
typedef struct {
    u32 size;
    u8* bytes;
} Memory;

typedef struct {
    Memory* memory;
    u16 segment;
    u16 offset;
} MemAccess;

u32 absolute_address(u16 segment, u16 offset);
u32 maxAddress(Memory* memory);
u8* access_memory(MemAccess mem);
u8* access_memory(Memory* memory, u32 absoluteAddress);
size_t load_from_file(MemAccess mem, u8* path);