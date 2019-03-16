#include "disp.h"
#include "mbc.h"
#include "registers.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

uint8_t tilemap[32][32] = { 0 };

uint16_t window_tile_address = 0x8000;

void *disp_worker () {
    //while(1) {
        // VBLANK counter
        memory[0xFF44] = (memory[0xFF44] + 1) % 153;

        if(memory[0xFF44] < 0x90) {
            //set_bit(&memory[0xFF41], 0, 1);
            //set_bit(&memory[0xFF41], 1, 1);
            
            /*
            for(int i = 0xFE00; i < 0xFE9F; i += 4) {
                uint8_t y_pos = memory[i];
                if(y_pos >= memory[0xFF44] && y_pos <= memory[0xFF44] - 8) {
                    uint8_t x_pos = memory[i + 1];
                    uint8_t sprite = memory[i + 2];
                    uint16_t sprite_line = memory_read16(0x8000 + (sprite * 16) + (memory[0xFF44] - y_pos) * 2);
                    for(int x = 0; x < 8; x++) {
                        if(((sprite_line >> (7 - x)) & 0x01) || ((sprite_line >> (15 - x)) & 0x01)) {
                            draw_rectangle(w, x_pos + x, memory[0xFF44], 1, 1);
                            printf("DRAW\n");
                        }
                    }
                }
            }
            */
        }
        else {
            // VBLANK
            set_bit(&memory[0xFF41], 0, 0);
            set_bit(&memory[0xFF41], 1, 0);
            if(memory[0xFF44] == 152) {
                window_clear(w, 0, 0, 320, 288);
                for(int y = 0; y < 32; y++) {
                    for(int x = 0; x < 32; x++) {
                        for(int sy = 0; sy < 8; sy++) {
                            uint16_t tile = memory_read16(window_tile_address + ((tilemap[y][x] * 16) + (sy * 2)));
                            if(tile == 0)
                                continue;
                            for(int sx = 0; sx < 8; sx++) {
                                if(((tile >> (7 - sx)) & 0x01) || ((tile >> (15 - sx)) & 0x01)) {
                                    uint8_t yoff = memory[0xFF42];
                                    uint8_t xoff = memory[0xFF43];
                                    fill_rectangle(w, ((x * 8) + sx - xoff) * 2, ((y * 8) + sy - yoff) * 2, 2, 2);
                                    //printf("DRAW x %i y %i\n", (x * 8) + sx, (y * 8) + sy + yoff);
                                }
                            }
                            
                        }
                        
                    }
                }
                window_update(w);
            }
            
        }

        // sleep for ~16.7ms / 153 lines = 100us
        //usleep(50000);
        //usleep(100);
    //}
    
}

uint8_t disp_init () {

    w = window_init(320, 288, "GBemu");
    memory_write8(0xFF40, memory_read8(0x91));
    //pthread_create(&disp_thread_handle, NULL, disp_worker, NULL);
    return 1;
}

uint8_t screen_register_write (uint16_t address, uint8_t value) {
    switch(address) {
        case 0xFF40:
        {
            // LCDC
            if(((value >> 6) & 0x1) == 1) {
                window_tile_address = 0x8800;
            }
            else {
                window_tile_address = 0x8000;
            }
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
