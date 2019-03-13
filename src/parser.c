#include "parser.h"

void OP_LD8 ();
uint8_t _half_carry (uint16_t a, uint16_t b);
uint8_t _carry16 (uint16_t a, uint16_t b);

uint8_t parse_op () {
    uint8_t m = memory_read8(PC);

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
        ((m & 0xF == 0x2 || m & 0xF == 0x6 || m & 0xF == 0xA || m & 0xF == 0xE) && m >> 4 <= 3) || 
        m == 0xE0 || m == 0xF0 || m == 0xE2 || m == 0xF2 || m == 0xEA || m == 0xFA) {
            OP_LD8();
            return 1;
    }
    // 16bit LOAD operations
    if((m & 0x0F == 0x1 && (m >> 4 <= 0x3 || m >> 4 >= 0xC)) || (m & 0x0F == 0x5 && m >= 0xC) ||
        m == 0x08 || m == 0xF8 || m == 0xF9) {

        OP_LD16();
        return 1;
    }

    return 0;
}

void OP_LD16 () {
    uint8_t m = memory_read8(PC);
    if(m >> 4 <= 0x3 && (m & 0xF == 0x1)) {
        // Put value nn into n
        PC += 2;
        switch(m) {
            case 0x01:
            {
                set_register16(REG_B, memory_read16(PC - 1));
                break;
            }
            case 0x11:
            {
                set_register16(REG_D, memory_read16(PC - 1));
                break;
            }
            case 0x21:
            {
                set_register16(REG_H, memory_read16(PC - 1));
                break;
            }
            case 0x31:
            {
                SP = memory_read16(PC - 1);
                break;
            }
        }
        return;
    }
    if(m == 0xF9) {
        // HL to SP
        SP = get_register16(REG_H);
        return;
    }
    if(m == 0xF8) {
        // LD HL, SP + n
        PC++;
        set_flag(FLAG_H, (((SP & 0xf) + ((int8_t)memory_read8(PC) & 0xf)) & 0x10) == 0x10);
        set_flag(FLAG_C, ((int)SP + (int8_t)memory_read8(PC) > 0xFFFF));
        set_register16(REG_H, SP + memory_read8(PC));
        set_flag(FLAG_Z, 0);
        set_flag(FLAG_N, 0);

        return;
    }

    if(m == 0x08) {
        // Put stack pointer at address n
        PC += 2;
        memory_write16(PC - 1, SP);
        return;
    }

    if(m >> 4 >= 0xC && m & 0xF == 0x5) {
        // PUSH nn
        switch(m) {
            case 0xF5:
            {
                memory_write16(SP - 2, get_register16(REG_A));
                SP -= 2;
                break;
            }
            case 0xC5:
            {
                memory_write16(SP - 2, get_register16(REG_B));
                SP -= 2;
                break;
            }
            case 0xD5:
            {
                memory_write16(SP - 2, get_register16(REG_D));
                SP -= 2;
                break;
            }
            case 0xE5:
            {
                memory_write16(SP - 2, get_register16(REG_H));
                SP -= 2;
                break;
            }
        }

        return;
    }
    if(m >> 4 >= 0xC && m & 0xF == 0x1) {
        // POP nn
        switch(m) {
            case 0xF1:
            {
                set_register16(REG_A, memory_read16(SP));
                memory_write16(SP, 0);
                SP += 2;
                break;
            }
            case 0xC1:
            {
                set_register16(REG_B, memory_read16(SP));
                memory_write16(SP, 0);
                SP += 2;
                break;
            }
            case 0xD1:
            {
                set_register16(REG_D, memory_read16(SP));
                memory_write16(SP, 0);
                SP += 2;
                break;
            }
            case 0xE1:
            {
                set_register16(REG_H, memory_read16(SP));
                memory_write16(SP, 0);
                SP += 2;
                break;
            }
        }

        return;
    }

}

// 8 bit Load operations
void OP_LD8 () {
    uint8_t m = memory_read8(PC);

    // put value nn into n
    if((m >= 0x06 && m <= 0x2E) && (m & 0xF == 0x6 || m & 0xF == 0xE)) {
        PC++;
        switch(m) {
            case 0x6:
            {
                set_register8(REG_B, memory_read8(PC));
                break;
            }
            case 0xE:
            {
                set_register8(REG_C, memory_read8(PC));
                break;
            }
            case 0x16:
            {
                set_register8(REG_D, memory_read8(PC));
                break;
            }
            case 0x1E:
            {
                set_register8(REG_E, memory_read8(PC));
                break;
            }
            case 0x26:
            {
                set_register8(REG_H, memory_read8(PC));
                break;
            }
            case 0x2E:
            {
                set_register8(REG_L, memory_read8(PC));
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
        set_register8(REG_A, memory_read8(get_register16(REG_H)));
        return;
    }

    // put r2 into B
    if(m >= 0x40 && m <= 0x45) {
        set_register8(REG_B, get_register8(m - 0x40 + REG_B));
        return;
    }
    // put address of HL into B
    if(m == 0x46) {
        set_register8(REG_B, memory_read8(get_register16(REG_H)));
        return;
    }

    // put r2 into C
    if(m >= 0x48 && m <= 0x4D) {
        set_register8(REG_C, get_register8(m - 0x48 + REG_B));
        return;
    }
    // put address of HL into C
    if(m == 0x4E) {
        set_register8(REG_C, memory_read8(get_register16(REG_H)));
        return;
    }

    // put r2 into D
    if(m >= 0x50 && m <= 0x55) {
        set_register8(REG_D, get_register8(m - 0x50 + REG_B));
        return;
    }
    // put address of HL into D
    if(m == 0x56) {
        set_register8(REG_D, memory_read8(get_register16(REG_H)));
        return;
    }

    // put r2 into E
    if(m >= 0x58 && m <= 0x5D) {
        set_register8(REG_E, get_register8(m - 0x58 + REG_B));
        return;
    }
    // put address of HL into E
    if(m == 0x5E) {
        set_register8(REG_E, memory_read8(get_register16(REG_H)));
        return;
    }

    // put r2 into H
    if(m >= 0x60 && m <= 0x65) {
        set_register8(REG_H, get_register8(m - 0x60 + REG_B));
        return;
    }
    // put address of HL into H
    if(m == 0x66) {
        set_register8(REG_H, memory_read8(get_register16(REG_H)));
        return;
    }

    // put r2 into L
    if(m >= 0x68 && m <= 0x6D) {
        set_register8(REG_L, get_register8(m - 0x68 + REG_B));
        return;
    }
    // put address of HL into L
    if(m == 0x6E) {
        set_register8(REG_L, memory_read8(get_register16(REG_H)));
        return;
    }
    #pragma endregion

    // Register to (HL)
    if(m >= 0x70 && m <= 0x75) {
        memory_write8(get_register16(REG_H), get_register8(m - 0x70 + REG_B));
        return;
    }
    // Put value nn into (HL)
    if(m == 0x36) {
        PC++;
        memory_write8(get_register16(REG_H), memory_read8(PC));
        return;
    }

    // put value nn into A
    if((m >= 0x78 && m <= 0x7F) || m == 0x0A || m == 0x1A || m == 0xFA || m == 0x3E) {
        if(m >= 0x78 && m <= 0x7D) {
            set_register8(REG_A, get_register8(m - 0x78 + REG_B));
            return;
        }
        switch(m) {
            case 0x0A:
            {
                // (BC) to A
                set_register8(REG_A, memory_read8(get_register16(REG_B)));
                break;
            }
            case 0x1A:
            {
                // (DE) to A
                set_register8(REG_A, memory_read8(get_register16(REG_D)));
                break;
            }
            case 0x7E:
            {
                // (HL) to A
                set_register8(REG_A, memory_read8(get_register16(REG_H)));
                break;
            }
            case 0xFA:
            {
                // (nn) to A(16b) LSB first
                PC++;
                uint16_t addr = memory_read16(PC);
                set_register8(REG_A, memory_read8(addr));
                PC++;
                break;
            }
            case 0x3E:
            {
                // # to A(8b)
                PC++;
                set_register8(REG_A, memory_read8(PC));
                break;
            }
        }
        return;
    }

    // put value A into nn
    if(((m >= 0x47 && m <= 0x7F) && (m & 0xF == 0xF || m & 0xF == 0x7)) || m == 0x02 || m == 0x12 || m == 0xEA) {
        switch(m) {
            case 0x7F:
            {
                // A to A
                break;
            }
            case 0x47:
            {
                // A to B
                set_register8(REG_B, get_register8(REG_A));
                break;
            }
            case 0x4F:
            {
                // A to C
                set_register8(REG_C, get_register8(REG_A));
                break;
            }
            case 0x57:
            {
                // A to D
                set_register8(REG_D, get_register8(REG_A));
                break;
            }
            case 0x5F:
            {
                // A to E
                set_register8(REG_E, get_register8(REG_A));
                break;
            }
            case 0x67:
            {
                // A to H
                set_register8(REG_H, get_register8(REG_A));
                break;
            }
            case 0x6F:
            {
                // A to L
                set_register8(REG_L, get_register8(REG_A));
                break;
            }
            case 0x02:
            {
                // A to (BC)
                memory_write8(get_register16(REG_B), get_register8(REG_A));
                break;
            }
            case 0x12:
            {
                // A to (BC)
                memory_write8(get_register16(REG_D), get_register8(REG_A));
                break;
            }
            case 0x77:
            {
                // A to (BC)
                memory_write8(get_register16(REG_H), get_register8(REG_A));
                break;
            }
            case 0xEA:
            {
                // A to (nn)(16b) LSB first
                PC++;
                uint16_t addr = memory_read16(PC);
                memory_write8(addr, get_register8(REG_A));
                PC++;
                break;
            }
        }
        return;
    }

    // Put value $FF00 + C into A
    if(m == 0xF2) {
        set_register8(REG_A, memory_read8(0xFF00 + get_register8(REG_C)));
        return;
    }
    // Put value A into $FF00 + C
    if(m == 0xE2) {
        memory_write8(0xFF00 + get_register8(REG_C), get_register8(REG_A));
        return;
    }

    // Put value at address HL into A, Dec HL
    if(m == 0x3A) {
        set_register(REG_A, memory_read8(get_register16(REG_H)));
        set_register16(REG_H, get_register16(REG_H) - 1);
        return;
    }

    // Put value at address HL into A, inc HL
    if(m == 0x2A) {
        set_register(REG_A, memory_read8(get_register16(REG_H)));
        set_register16(REG_H, get_register16(REG_H) + 1);
        return;
    }

    // Put value A at address HL, Dec HL
    if(m == 0x32) {
        memory_write8(get_register16(REG_H), get_register8(REG_A));
        set_register16(REG_H, get_register16(REG_H) - 1);
        return;
    }

    // Put value A at address HL, inc HL
    if(m == 0x32) {
        memory_write8(get_register16(REG_H), get_register8(REG_A));
        set_register16(REG_H, get_register16(REG_H) + 1);
        return;
    }

    // Put value A to $FF00 + n
    if(m == 0x32) {
        PC++;
        memory_write8(0xFF00 + memory_read8(PC), get_register8(REG_A));
        return;
    }

    // Put value $FF00 + n to A
    if(m == 0x32) {
        PC++;
        set_register8(REG_A, memory_read8(0xFF00 + memory_read8(PC)));
        return;
    }

}

uint8_t _half_carry (uint16_t a, uint16_t b) {
    return (((a & 0xf) + (b & 0xf)) & 0x10) == 0x10;
}

uint8_t _carry16 (uint16_t a, uint16_t b) {
    return ((uint32_t)a + (uint32_t)b > 0xFFFF);
}

uint8_t run () {
    while(1) {

        parse_op();
        PC++;
    }
}