#include <stdint.h>

typedef enum MEMORY_MODE { MM_ROM_ONLY, MM_MBC1, MM_MBC2, MM_MBC3, MM_MBC5 } MEMORY_MODE;


// Memory mode

extern enum MEMORY_MODE memory_mode;

extern uint8_t max_memory_mode;

// Memory bank selector (16KB banks)
extern uint8_t memory_bank;

// RAM bank selector (8KB banks)
extern uint8_t ram_bank;

// MEMORY

extern uint8_t memory[0xFFFF];