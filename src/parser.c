#include "parser.h"

void OP_LD ();

uint8_t parse_op () {
    uint8_t m = memory[PC];

    // NO OPERATION
    if(m == 0x00) 
        return 1;

    // Enable stop mode
    if(m == 0x10) {
        MODE_STOP = 1;
        PC++;
        return 1;
    }

    // 8bit LOAD operations
    if(((m >= 0x40 && m <= 0x7F) && m != 0x76) || 
        ((m & 0x0F == 0x02 || m & 0x0F == 0x06 || m & 0x0F == 0x0A || m & 0x0F == 0x0E ) && (m >> 4) < 0x3)) {
            OP_LD();
            return 1;
    }

    return 0;
}

// Load operations
void OP_LD () {
    uint8_t m = memory[PC];

    // put value nn into n
    if(m % 0x8 == 0x6) {
        PC++;
        switch(m) {
            case 0x6:
            {
                set_register8(REG_B, memory[PC]);
                break;
            }
            case 0xE:
            {
                set_register8(REG_C, memory[PC]);
                break;
            }
            case 0x16:
            {
                set_register8(REG_D, memory[PC]);
                break;
            }
            case 0x1E:
            {
                set_register8(REG_E, memory[PC]);
                break;
            }
            case 0x26:
            {
                set_register8(REG_H, memory[PC]);
                break;
            }
            case 0x2E:
            {
                set_register8(REG_L, memory[PC]);
                break;
            }
        }
        return;
    }

    // put R2 into R1
    #pragma region OP_LD_R2_INTO_R1
    // put r2 into A
    if(m >= 0x78 && m <= 0x7D) {
        set_register8(REG_A, get_register8(m - 0x78 + REG_B));
        return;
    }
    // put address of HL into A
    if(m == 0x7E) {
        set_register8(REG_A, memory[get_register16(REG_H)]);
        return;
    }

    // put r2 into B
    if(m >= 0x40 && m <= 0x45) {
        set_register8(REG_B, get_register8(m - 0x40 + REG_B));
        return;
    }
    // put address of HL into B
    if(m == 0x46) {
        set_register8(REG_B, memory[get_register16(REG_H)]);
        return;
    }

    // put r2 into C
    if(m >= 0x48 && m <= 0x4D) {
        set_register8(REG_C, get_register8(m - 0x48 + REG_B));
        return;
    }
    // put address of HL into C
    if(m == 0x4E) {
        set_register8(REG_C, memory[get_register16(REG_H)]);
        return;
    }

    // put r2 into D
    if(m >= 0x50 && m <= 0x55) {
        set_register8(REG_D, get_register8(m - 0x50 + REG_B));
        return;
    }
    // put address of HL into D
    if(m == 0x56) {
        set_register8(REG_D, memory[get_register16(REG_H)]);
        return;
    }

    // put r2 into E
    if(m >= 0x58 && m <= 0x5D) {
        set_register8(REG_E, get_register8(m - 0x58 + REG_B));
        return;
    }
    // put address of HL into E
    if(m == 0x5E) {
        set_register8(REG_E, memory[get_register16(REG_H)]);
        return;
    }

    // put r2 into H
    if(m >= 0x60 && m <= 0x65) {
        set_register8(REG_H, get_register8(m - 0x60 + REG_B));
        return;
    }
    // put address of HL into H
    if(m == 0x66) {
        set_register8(REG_H, memory[get_register16(REG_H)]);
        return;
    }

    // put r2 into L
    if(m >= 0x68 && m <= 0x6D) {
        set_register8(REG_L, get_register8(m - 0x68 + REG_B));
        return;
    }
    // put address of HL into L
    if(m == 0x6E) {
        set_register8(REG_L, memory[get_register16(REG_H)]);
        return;
    }
    #pragma endregion

    // Register to (HL)
    if(m >= 0x70 && m <= 0x75) {
        memory[get_register16(REG_H)] = get_register8(m - 0x70 + REG_B);
        return;
    }
    // Put value nn into (HL)
    if(m == 0x36) {
        PC++;
        memory[get_register16(REG_H)] = memory[PC];
        return;
    }

}


uint8_t run () {
    while(1) {

        parse_op();
        PC++;
    }
}