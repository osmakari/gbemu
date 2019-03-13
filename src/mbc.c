#include "mbc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
    MEMORY CONTROLLER
*/

// Cartridge type
enum MEMORY_MODE memory_mode = MM_ROM_ONLY;

// MEMORY
/*
Interrupt Enable Register
--------------------------- FFFF
Internal RAM
--------------------------- FF80
Empty but unusable for I/O
--------------------------- FF4C
I/O ports
--------------------------- FF00
Empty but unusable for I/O
--------------------------- FEA0
Sprite Attrib Memory (OAM)
--------------------------- FE00
Echo of 8kB Internal RAM
--------------------------- E000
8kB Internal RAM
--------------------------- C000
8kB switchable RAM bank
--------------------------- A000
8kB Video RAM
--------------------------- 8000 --
16kB switchable ROM bank          |
--------------------------- 4000  |= 32kB Cartrigbe
16kB ROM bank #0                  |
--------------------------- 0000 --
*/

uint8_t max_memory_mode = 0;

uint8_t memory_bank = 0;

uint8_t ram_bank = 0;

// Memory buffer for cartridge
uint8_t *cartridge;

uint32_t cartridge_size = 0;

uint8_t rom_size = 0;
uint8_t ram_size = 0;

uint8_t *ext_ram;

uint8_t memory[0xFFFF] = { 0 };


uint8_t rom_load (const char *file) {
    FILE * pFile = fopen(file, "rb");

    if(pFile != NULL) {
        uint8_t _rsz[4] = { 0 };
        // 0: GB/SGB Indicator 
        // 1: Cartridge type
        // 2: ROM size
        // 3: RAM size
        fseek(pFile, 0x146, SEEK_SET);
        fread(_rsz, 1, 4, pFile);
        if(_rsz[1] == 0) {
            memory_mode = MM_ROM_ONLY;
        }
        else if(_rsz[1] == 1) {
            memory_mode = MM_MBC1;
        }
        rom_size = _rsz[2];
        ram_size = _rsz[3];

        switch(ram_size) {
            case 0:
            {
                ext_ram = NULL;
                break;
            }
            case 1:
            {
                // 2KB
                ext_ram = (uint8_t*)malloc(2000);
                break;
            }
            case 2:
            {
                // 8KB
                ext_ram = (uint8_t*)malloc(8000);
                break;
            }
            case 3:
            {
                // 32KB
                ext_ram = (uint8_t*)malloc(32000);
                break;
            }
            case 4:
            {
                // 128KB
                ext_ram = (uint8_t*)malloc(128000);
                break;
            }
        }
        

        fseek (pFile, 0, SEEK_END);
        cartridge_size = ftell (pFile);
        rewind (pFile);

        cartridge = (uint8_t*)malloc(cartridge_size);

        fread(cartridge, 1, cartridge_size, pFile);

        memcpy(memory, cartridge, 0x4000);

        return 1;
    }
    else {
        printf("Could not load ROM\n");
        return 0;
    }
}

uint8_t memory_write8 (uint16_t address, uint8_t value) {
    
    if(memory_mode == MM_ROM_ONLY) {
        memory[address] = value;
    }
    else if(memory_mode == MM_MBC1) {
        // Controller operations
        if(address >= 0x6000 && address <= 0x7FFF) {
            // Change max memory mode
            if(value & 0x01 == 1) {
                // 4Mbit ROM/32KB ram mode
                max_memory_mode = 1;
            }
            else {
                // 16Mbit ROM/8KB ram mode
                max_memory_mode = 0;
            }
            return 1;
        }
        if(address >= 0x2000 && address <= 0x7FFF) {
            // Change memory bank
            memory_bank = value & 0b00011111;
            if(memory_bank == 0)
                memory_bank = 1;
            return 1;
        }

        // 4Mbit ROM/32KB ram mode
        if(max_memory_mode == 1) {
            if(address >= 0x4000 && address <= 0x5FFF) {
                ram_bank = value & 0b11;
                return 1;
            }
        }
        // 16Mbit ROM/8KB ram mode
        else {
            // "Writing a value (XXXXXXBB - X = Don't care, B = bank select bits) into 4000-5FFF 
            // area will set the two most significant ROM address lines."
            if(address >= 0x4000 && address <= 0x5FFF) {
                ram_bank = value & 0b11;
                return 1;
            }
        }

        // Actual memory write...
        memory[address] = value;
        return 1;
        
    }
    return 0;
}

uint8_t memory_write16 (uint16_t address, uint16_t value) {
    memory_write8(address, value >> 8);
    memory_write8(address + 1, value & 0xFF);
    return 1;
}

uint8_t memory_read8 (uint16_t address) {
    if(address >= 0x4000 && address <= 0x7FFF) {
        return cartridge[(0x4000 * memory_bank) + (address - 0x4000)];
    }
    else if(address >= 0xA000 && address <= 0xBFFF) {
        return ext_ram[0xA000 + (0x2000 * ram_bank) + (address - 0xA000)];
    }
    
    return memory[address];
}

uint16_t memory_read16 (uint16_t address) {
    return memory_read8(address) << 8 | memory_read8(address + 1);
}