#include "mbc.h"

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

uint8_t memory[0xFFFF] = { 0 };

uint8_t memory_write8 (uint16_t address, uint8_t value) {

    if(memory_mode == MM_MBC1) {
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
            ram_bank = value & 0b11;
        }

    }
    return 0;
}