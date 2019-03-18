#include "mbc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disp.h"
#include "registers.h"
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

uint8_t memory_bank = 1;

uint8_t ram_bank = 1;

// Memory buffer for cartridge
uint8_t *cartridge;

uint32_t cartridge_size = 0;

uint8_t rom_size = 0;
uint8_t ram_size = 0;

uint8_t *ext_ram;

uint8_t memory[0xFFFF] = { 0 };

uint8_t booting = 1;

uint8_t boot_program[0x100] = { 
	0x31, 0xfe, 0xff, 0xaf, 0x21, 0xff, 0x9f, 0x32, 
	0xcb, 0x7c, 0x20, 0xfb, 0x21, 0x26, 0xff, 0x0e, 
	0x11, 0x3e, 0x80, 0x32, 0xe2, 0x0c, 0x3e, 0xf3, 
	0xe2, 0x32, 0x3e, 0x77, 0x77, 0x3e, 0xfc, 0xe0, 
	0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1a, 
	0xcd, 0x95, 0x00, 0xcd, 0x96, 0x00, 0x13, 0x7b, 
	0xfe, 0x34, 0x20, 0xf3, 0x11, 0xd8, 0x00, 0x06, 
	0x08, 0x1a, 0x13, 0x22, 0x23, 0x05, 0x20, 0xf9, 
	0x3e, 0x19, 0xea, 0x10, 0x99, 0x21, 0x2f, 0x99, 
	0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20, 
	0xf9, 0x2e, 0x0f, 0x18, 0xf3, 0x67, 0x3e, 0x64, 
	0x57, 0xe0, 0x42, 0x3e, 0x91, 0xe0, 0x40, 0x04, 
	0x1e, 0x02, 0x0e, 0x0c, 0xf0, 0x44, 0xfe, 0x90, 
	0x20, 0xfa, 0x0d, 0x20, 0xf7, 0x1d, 0x20, 0xf2, 
	0x0e, 0x13, 0x24, 0x7c, 0x1e, 0x83, 0xfe, 0x62, 
	0x28, 0x06, 0x1e, 0xc1, 0xfe, 0x64, 0x20, 0x06, 
	0x7b, 0xe2, 0x0c, 0x3e, 0x87, 0xe2, 0xf0, 0x42, 
	0x90, 0xe0, 0x42, 0x15, 0x20, 0xd2, 0x05, 0x20, 
	0x4f, 0x16, 0x20, 0x18, 0xcb, 0x4f, 0x06, 0x04, 
	0xc5, 0xcb, 0x11, 0x17, 0xc1, 0xcb, 0x11, 0x17, 
	0x05, 0x20, 0xf5, 0x22, 0x23, 0x22, 0x23, 0xc9, 
	0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b, 
	0x03, 0x73, 0x00, 0x83, 0x00, 0x0c, 0x00, 0x0d, 
	0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e, 
	0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99, 
	0xbb, 0xbb, 0x67, 0x63, 0x6e, 0x0e, 0xec, 0xcc, 
	0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e, 
	0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5, 0x42, 0x3c, 
	0x21, 0x04, 0x01, 0x11, 0xa8, 0x00, 0x1a, 0x13, 
	0xbe, 0x20, 0xfe, 0x23, 0x7d, 0xfe, 0x34, 0x20, 
	0xf5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 
	0xfb, 0x86, 0x00, 0x00, 0x3e, 0x01, 0xe0, 0x50
};


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
        printf("MEMORY MODE: 0x%x\n", memory_mode);
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
        uint8_t t = 0x19;
        for(int x = 0x134; x < 0x14D; x++) {
            t += memory[x];
        }
        printf("CHECK: %i\n", t);
        //exit(0);
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
            printf("Changing MAX MEMORY MODE to: %i\n", value & 0x01);
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

            printf("Setting memory bank to: 0x%2x AT PC: 0x%x\n", memory_bank, PC - 1);
            
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
        
        
        
    }
    if(address >= 0xFF40 && address <= 0xFF4B) {
        screen_register_write(address, value);
    }
    if(address == 0xFF50) {
        if(value)
            booting = 0;
        else
            booting = 1;
    }

    if(address >= 0xE000 && address <= 0xFDFF) {
        memory[(address - 0xE000) + 0xFF80] = value;
        return 1;
    }

    // Actual memory write...
    memory[address] = value;
    if(address >= 0xFE00 && address <= 0xFE9F) {
        printf("Writing to OAM at 0x%x value 0x%x\n", address, value);
    }
    if(address >= 0x9800 && address <= 0x9BFF) {
        // Writing to Tile map #1
        printf("Writing to tilemap:  %x, %x\n", (address), value);
        tilemap[(address - 0x9800)/32][(address - 0x9800) % 32] = value;
    }
    if(address >= 0x8000 && address <= 0x9FFF) {
        //printf("Writing to VRAM at 0x%x value 0x%x\n", address, value);
    }

    return 1;
}

uint8_t memory_write16 (uint16_t address, uint16_t value) {
    memory_write8(address, value & 0xFF);
    memory_write8(address + 1, value >> 8);
    return 1;
}

uint8_t memory_read8 (uint16_t address) {
    if(address < 0x100 && booting) {
        return boot_program[address];
    }

    if(address >= 0x4000 && address <= 0x7FFF) {
        return cartridge[(0x4000 * memory_bank) + (address - 0x4000)];
    }
    else if(address >= 0xA000 && address <= 0xBFFF) {
        return ext_ram[0xA000 + (0x2000 * ram_bank) + (address - 0xA000)];
    }

    if(address >= 0xE000 && address <= 0xFDFF) {
        return memory[(address - 0xE000) + 0xFF80];
    }

    if(address >= 0xFF40 && address <= 0xFF4B) {
        //printf("Reading LCD registers\n");
    }
    if(address >= 0xFE00 && address <= 0xFE9F) {
        //printf("Reading OAM\n");
    }
    if(address >= 0x8000 && address <= 0x9FFF) {
        //printf("Reading VRAM\n");
    }
    
    return memory[address];
}

uint16_t memory_read16 (uint16_t address) {
    return memory_read8(address) | (memory_read8(address + 1) << 8);
}