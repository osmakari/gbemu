#include <stdint.h>

typedef enum MEMORY_MODE { MM_ROM_ONLY, MM_MBC1, MM_MBC2, MM_MBC3, MM_MBC5 } MEMORY_MODE;


// Memory mode

extern enum MEMORY_MODE memory_mode;

extern uint8_t max_memory_mode;

extern uint8_t *cartridge;

extern uint32_t cartridge_size;

// Memory bank selector (16KB banks)
extern uint8_t memory_bank;

// RAM bank selector (8KB banks)
extern uint8_t ram_bank;

// ROM size
extern uint8_t rom_size;

// RAM size
extern uint8_t ram_size;

// External Ram in cartridge
extern uint8_t *ext_ram;

extern uint8_t booting;

// MEMORY

extern uint8_t memory[0xFFFF];


uint8_t rom_load (const char *file);

uint8_t memory_write8 (uint16_t address, uint8_t value);

uint8_t memory_write16 (uint16_t address, uint16_t value);

uint8_t memory_read8 (uint16_t address);

uint16_t memory_read16 (uint16_t address);