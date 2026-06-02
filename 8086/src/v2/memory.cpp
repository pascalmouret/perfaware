#include "../sanity.h"
#include "./memory.h"

#include <cstdio>
#include <cstdlib>

u32 absolute_address(u16 segment, u16 offset) {
    return ((u32)segment << 4) + (u32)offset;
}

u8* access_memory(MemAccess access) {
    return &access.memory->bytes[absolute_address(access.segment, access.offset)];
}

u8* access_memory(Memory* memory, u32 absoluteAddress) {
    return &memory->bytes[absoluteAddress];
}

size_t load_from_file(MemAccess access, u8* path) {
    FILE *file = fopen((char*)path, "rb");

    if (!file) {
        printf("ERROR: Could not open file %p\n", path);
        exit(1);
    }

    u32 base = absolute_address(access.segment, access.offset);
    u32 maxBytes = access.memory->size - base;

    size_t result = fread(access_memory(access), 1, maxBytes, file);
    fclose(file);

    return result;
}