#include "disp.h"
#include "mbc.h"
#include <stdio.h>

uint8_t disp_init () {

    w = window_init(160, 144, "GBemu");
    memory_write8(0xFF40, memory_read8(0x91));

    return 1;
}

uint8_t screen_register_write (uint16_t address, uint8_t value) {
    switch(address) {
        case 0xFF40:
        {
            // LCDC
            break;
        }
        case 0xFF42:
        {
            // Screen Scroll Y
            break;
        }
        case 0xFF43:
        {
            // Screen scroll X
            break;
        }
        case 0xFF48:
        {
            // Object palette 0 Data
            break;
        }
        case 0xFF49:
        {
            // Object palette 1 Data
            break;
        }
    }

    //printf("SCREEN WRITE************\n");
    return 1;
}

uint8_t disp_vblank () {
    int i = 0xFE00;
    while(i < 0xFE9F) {
        uint8_t y_pos = memory_read8(i);
        uint8_t x_pos = memory_read8(i + 1);
        uint8_t sn = memory_read8(i + 2);
        uint8_t sa = memory_read8(i + 3);
        
        for(int y = 0; y < 8; y++) {
            uint16_t sprite_line = memory_read16(0x8000 + (sn * 16) + y * 2);
            for(int x = 0; x < 8; x++) {
                if(((sprite_line >> (7 - x)) & 0x01) || ((sprite_line >> (17 - x)) & 0x01))
                    draw_rectangle(w, x_pos + x, y_pos + y, 1, 1);
            }
            
        }

        i += 4;
    }
    
    return 1;
}