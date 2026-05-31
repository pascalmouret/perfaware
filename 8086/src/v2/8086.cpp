#include "8086.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "memory.h"
#include "decode.h"
#include "text.h"

const size_t MEMORY_SIZE = 1024 * 1024;

int main(int argc, char* argv[]) {
    CPU cpu = {};
    cpu.memory.size = MEMORY_SIZE;
    cpu.memory.bytes = (u8*)malloc(MEMORY_SIZE);

    MemAccess currentMemory = {&cpu.memory, 0, 0};

    size_t bytes_loaded = load_from_file(&currentMemory, (u8*)argv[1]);

    if (strcmp(argv[2], "--decode") == 0) {
        print_instructions(&currentMemory, bytes_loaded);
        free(cpu.memory.bytes);
        return 0;
    }

    while (absolute_address(currentMemory.segment, currentMemory.offset) < bytes_loaded) {
        Instruction instruction = decode(&currentMemory);

        if (instruction.size == 0) {
            printf("ERROR: Failed to decode instruction at %u\n", absolute_address(currentMemory.segment, currentMemory.offset));
            return 1;
        }

        currentMemory.offset += instruction.size;
    }

    free(cpu.memory.bytes);

    return 0;
}
