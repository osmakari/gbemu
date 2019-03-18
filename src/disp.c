#include "disp.h"
#include "mbc.h"
#include "registers.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

uint8_t tilemap[32][32] = { 0 };

uint16_t window_tile_address = 0x8000;

// 144 rows of 8 pixels * 20 (2bits for color)
uint16_t screen_buffer[144][20] = { 0 };

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
                // 4 bytes per Object
                // y, x, pattern, flags
                for(int i = 0xFE00; i < 0xFE9F; i += 4) {
                    uint8_t yp = memory[i];
                    uint8_t xp = memory[i + 1];
                    uint8_t sprite_pointer = memory[i + 2];

                    for(int y = 0; y < (((memory[0xFF40] >> 2) & 0x1) ? 16 : 8); y++) {
                        uint16_t sprite = memory_read16(0x8000 + (sprite_pointer * (((memory[0xFF40] >> 2) & 0x1) ? 16 : 8) * 8));
                        for(int x = 0; x < 8; x++) {
                            if(((sprite >> (7 - x)) & 0x01) || ((sprite >> (15 - x)) & 0x01)) {
                                uint8_t yoff = memory[0xFF42];
                                uint8_t xoff = memory[0xFF43];
                                fill_rectangle(w, (xp + x) * 2, (yp + y) * 2, 2, 2);
                                //printf("DRAW x %i y %i\n", (x * 8) + sx, (y * 8) + sy + yoff);
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

unsigned long last_clock = 0;

const uint16_t _VBLANK_CLOCKS = 4560;
const uint16_t _HBLANK_CLOCKS = 201;
const uint16_t _SEARCH_OAM_CLOCKS = 77;
const uint16_t _LCD_TRANSFER = 169;

uint8_t oam_addr_index = 0;
// OAM line buffer
uint16_t oam_addr[10] = { 0 };

uint8_t display_update () {
    uint8_t state = (memory[0xFF41] & 0b11);
    //printf("STATE: %x; CLOCKS: %i;\n", state, clocks);
    // VBLANK waiting...
    if(state == 1) {
        if(last_clock + _VBLANK_CLOCKS <= clocks) {
            // Clear the screen
            window_clear(w, 0, 0, 320, 288);
            //printf("RENDERING\n");
            last_clock = clocks;

            // DRAW here
            /*
            for(int y = 0; y < 32; y++) {
                for(int x = 0; x < 32; x++) {
                    for(int sy = 0; sy < 8; sy++) {
                        uint16_t tile = memory_read16(window_tile_address + ((tilemap[y][x] * 16) + (sy * 2)));
                        if(tile == 0)
                            continue;

                        printf("TILE: %x\n", tilemap[y][x]);
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
            */
            
            
            for(int y = 0; y < 144; y++) {
                for(int x = 0; x < 160; x++) {
                    uint8_t color = ((screen_buffer[y][x/8] >> (7 - (x % 8))) & 0x01) + ((screen_buffer[y][x/8] >> (15 - (x % 8))) & 0x01);
                    if(color != 0) {
                        fill_rectangle(w, x * 2, y * 2, 2, 2);
                    }
                }
            }
            
            
            
            window_update(w);
            for(int x = 0; x < 144; x++) {
                memset(screen_buffer[x], 0, 20 * 2);
            }
            // move to OAM read phase
            // Do OAM stuff
            for(int i = 0xFE00; i < 0xFE9F; i += 4) {
                if(memory[i] >= memory[0xFF44] && memory[i] <= (memory[0xFF44] + (((memory[0xFF40] >> 2) & 0x1) ? 16 : 8))) {
                    if(oam_addr_index >= 10) {
                        // Hardware limitiations...
                        break;
                    }
                    oam_addr[oam_addr_index] = i;
                    oam_addr_index++;
                    
                }
            }

            set_bit(&memory[0xFF41], 1, 1);
            set_bit(&memory[0xFF41], 0, 0);

            // VBLANK over, set LY
            memory[0xFF44] = 0;

            // INTERRUPT
            if((memory[0xFFFF] & 0x01) == 1) {
                // reset IF
                set_bit(&memory[0xFF0F], 0, 0);
                // Add current PC to stack
                memory_write16(SP - 2, PC);
                SP -= 2;
                PC = 0x40;
            }

            // return 1: VBLANK
            return 1;
        }
        // VBLANK lasts for 4560 clocks...
        memory[0xFF44] = 144 + (9 * (float)((float)(clocks - last_clock) / (float)_VBLANK_CLOCKS));
        // Return 0: WAIT
        return 0;
    }
    // HBLANK waiting..
    if(state == 0) {
        if(last_clock + _HBLANK_CLOCKS <= clocks) {
            
            last_clock = clocks;
            if(memory[0xFF44] < 143) {
                // Read OAM
                for(int i = 0xFE00; i < 0xFE9F; i += 4) {
                    if(memory[i] >= memory[0xFF44] && memory[i] <= (memory[0xFF44] + (((memory[0xFF40] >> 2) & 0x1) ? 16 : 8))) {
                        if(oam_addr_index >= 10) {
                            // Hardware limitiations...
                            break;
                        }
                        if(i + 2 == 0)
                            continue;
                            
                        oam_addr[oam_addr_index] = i;
                        oam_addr_index++;
                        
                    }
                }
                // Go to wait OAM
                set_bit(&memory[0xFF41], 1, 1);
                set_bit(&memory[0xFF41], 0, 0);
                return 3;
            }
            else {
                // GO TO VBLANK:
                set_bit(&memory[0xFF41], 1, 0);
                set_bit(&memory[0xFF41], 0, 1);

                // Set interrupt for VBLANK
                set_bit(&memory[0xFF0F], 0, 1);
                return 4;
            }
        }

        return 0;
    }
    // Read OAM waiting...
    if(state == 2) {
        if(last_clock + _SEARCH_OAM_CLOCKS <= clocks) {
            last_clock = clocks;
            // Do transfers

            uint8_t line = memory[0xFF44];
            uint8_t yoff = memory[0xFF42];
            uint8_t xoff = memory[0xFF43];

            if((line + yoff)/8 < 32) {
                for(int x = 0; x < 32; x++) {
                    if(xoff + (x * 8) > 153 || (yoff + line) > 144) 
                        break;

                    if(tilemap[(line + yoff)/8][((x * 8 + xoff)/8)] == 0)
                        continue;
                    //uint16_t tile = memory_read16(window_tile_address + (tilemap[line/8 - yoff/8][x] * 16) + ((line % 8) * 2));
                    uint16_t tile = memory_read16(window_tile_address + tilemap[(line + yoff)/8][((x * 8 + xoff)/8)] * 16 + (((line + yoff) % 8) * 2));
                    
                    if(tile == 0)
                        continue;
                    //printf("TILE %x\n", tilemap[(line + yoff)/8][x + (xoff/8)]);
                    
                    screen_buffer[line][x] = tile;
                    /*
                    if(((x * 8) % 8) != 0 && x < 19) {
                        // continue to next bitset
                        screen_buffer[line-yoff][x + 1] |= (((tile & 0xFF00) << ((7 - ((x * 8) % 8))) & 0xFF00) | (((tile & 0xFF) << (7 - ((x * 8) % 8))) & 0x00FF));
                    }
                    */
                }
            }
            
            for(int i = 0; i < oam_addr_index; i++) {
                uint8_t sprite_index = memory[oam_addr[oam_addr_index] + 2];
                uint8_t xp = memory[oam_addr[oam_addr_index] + 1];
                uint8_t yp = memory[oam_addr[oam_addr_index]];
                uint8_t attr = memory[oam_addr[oam_addr_index] + 3];
                uint16_t sprite = memory_read16(0x8000 + (sprite_index * (((memory[0xFF40] >> 2) & 0x1) ? 16 : 8) * 8));
                screen_buffer[memory[0xFF44]][xp/8] |= (((sprite & 0xFF00) >> (xp % 8)) & 0xFF00) | (((sprite & 0xFF) >> (xp % 8)) & 0x00FF);
                if((xp % 8) != 0 && xp/8 < 19) {
                    // continue to next bitset
                    screen_buffer[memory[0xFF44]][xp/8 + 1] |= ((sprite & 0xFF00) << ((7 - (xp % 8))) & 0xFF00) | (((sprite & 0xFF) >> (7 - (xp % 8))) & 0x00FF);
                }
            }
            
            set_bit(&memory[0xFF41], 1, 1);
            set_bit(&memory[0xFF41], 0, 1);
            memory[0xFF44]++;
            return 4;
        }
        // Waiting....
        return 0;
    }

    // Transfer waiting..
    if(state == 3) {
        if(last_clock + _LCD_TRANSFER <= clocks) {
            last_clock = clocks;
            // Transfer over:
            memset(oam_addr, oam_addr_index * 2, 0);
            oam_addr_index = 0;
            // Go to HBLANK

            set_bit(&memory[0xFF41], 1, 0);
            set_bit(&memory[0xFF41], 0, 0);
            return 5;

        }
        return 0;
    }

    // Return 0xFF: UNKNOWN
    return 0xFF;
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
