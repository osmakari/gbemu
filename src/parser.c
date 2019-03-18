#include "parser.h"
#include <unistd.h>
#include <stdio.h>
#include "disp.h"
#include <time.h>

void OP_LD8 ();
void OP_LD16 ();

void OP_ALU8 ();
void OP_ALU16 ();

uint8_t _half_carry16 (uint16_t a, uint16_t b);
uint8_t _half_carry8 (uint8_t a, uint8_t b);
uint8_t _half_carry_sub8 (uint8_t a, uint8_t b);
uint8_t _carry16 (uint16_t a, uint16_t b);

uint8_t rotate_right8 (uint8_t n, uint8_t d);
uint8_t rotate_left8 (uint8_t n, uint8_t d);

void OP_BITOP8 ();

struct timeval start;

uint16_t last_sp = 0;

uint8_t parse_op () {
    uint8_t m = memory_read8(PC);

    //printf("Reading operation: 0x%2x\n", m);
    //print_registers();
    //if(last_sp != SP) {
        //printf("PC: 0x%2x, M: 0x%2x\n", PC, m);
    //    printf("SP CHANGED FROM 0x%x to 0x%x\n", last_sp, SP);
    //    last_sp = SP;
    //}
    
    //printf("VERT: 0x%2x\n", memory[0xFF44]);
    
    if(PC >= 0x27 && PC <= 0x30) {
        //printf("DE: 0x%x\n", get_register16(REG_D));
    }

    // NO OPERATION
    if(m == 0x00) 
        return 1;

    // Enable stop mode
    if(m == 0x10) {
        MODE_STOP = 1;
        PC++;
        return 1;
    }

    if(m == 0x76) {
        printf("HALT HALT HALT!!!! int status %i\n", interrupt_enable);
        MODE_STOP = 2;
        return 1;
    }

    // 8bit LOAD operations
    if(((m >= 0x40 && m <= 0x7F) && m != 0x76) || 
        (((m & 0xF) == 0x2 || (m & 0xF) == 0x6 || (m & 0xF) == 0xA || (m & 0xF) == 0xE) && ((m >> 4) <= 3)) || 
        m == 0xE0 || m == 0xF0 || m == 0xE2 || m == 0xF2 || m == 0xEA || m == 0xFA) {
            //printf("Going to 8bit Loads\n");
            OP_LD8();
            return 1;
    }
    // 16bit LOAD operations
    if(((m & 0x0F) == 0x1 && ((m >> 4) <= 0x3 || (m >> 4) >= 0xC)) || ((m & 0x0F) == 0x5 && (m >> 4) >= 0xC) ||
        m == 0x08 || m == 0xF8 || m == 0xF9) {
        //printf("Going to 16bit Loads\n");
        OP_LD16();
        return 1;
    }

    // 8bit ALU
    if((m >= 0x80 && m <= 0xBF) || (((m >> 4) <= 0x3) && ((m & 0xF) == 0x4 || (m & 0xF) == 0x5 || (m & 0xF) == 0xC || (m & 0xF) == 0xD))
        || (((m >> 4) >= 0xC) && ((m & 0xF) == 0x6 || (m & 0xF) == 0xE)) || m == 0x27 || m == 0x37 || m == 0x2F || m == 0x3F) {

        OP_ALU8();
        return 1;
    }
    // 16it ALU
    if((((m & 0xF) == 0x3 || (m & 0xF) == 0x9 || (m & 0xF) == 0xB) && ((m >> 4) <= 0x3)) || m == 0xE8) {
        OP_ALU16();
        return 1;
    }

    // RRCA
    if(m == 0x0F) {

        set_flag(FLAG_Z, get_register8(REG_A) == 0);
        set_flag(FLAG_C, get_register8(REG_A) & 0x01);
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, 0);
        set_register8(REG_A, rotate_right8(get_register8(REG_A), 1));
        return 1;
    }
    // RRA
    if(m == 0x1F) {

        
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, 0);
        uint8_t c = get_flag(FLAG_C);
        set_flag(FLAG_C, get_register8(REG_A) & 0x01);
        set_register8(REG_A, (c << 7) | (get_register8(REG_A) >> 1));
        set_flag(FLAG_Z, get_register8(REG_A) == 0);
        return 1;
    }

    // RLCA
    if(m == 0x07) {

        set_flag(FLAG_Z, get_register8(REG_A) == 0);
        set_flag(FLAG_C, (get_register8(REG_A) << 7) & 0x01);
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, 0);
        set_register8(REG_A, rotate_left8(get_register8(REG_A), 1));
        return 1;
    }
    // RLA
    if(m == 0x17) {

        
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, 0);
        uint8_t c = get_flag(FLAG_C);
        set_flag(FLAG_C, (get_register8(REG_A) << 7) & 0x01);
        set_register8(REG_A, (c & 0x1) | (get_register8(REG_A) << 1));
        set_flag(FLAG_Z, get_register8(REG_A) == 0);
        return 1;
    }

    // JUMPS
    // Jump to address
    if(m == 0xC3) {
        PC += 2;
        uint16_t addr = memory_read16(PC - 1);
        PC = addr - 1;
        return 1;
    }
    // Jump to address nn if following condition is true
    if(m == 0xC2) {
        PC += 2;
        uint16_t addr = memory_read16(PC - 1);
        if(!get_flag(FLAG_Z)) {
            PC = addr - 1;
        }
        return 1;
    }
    if(m == 0xCA) {
        PC += 2;
        uint16_t addr = memory_read16(PC - 1);
        if(get_flag(FLAG_Z)) {
            PC = addr - 1;
        }
        return 1;
    }
    if(m == 0xD2) {
        PC += 2;
        uint16_t addr = memory_read16(PC - 1);
        if(!get_flag(FLAG_C)) {
            PC = addr - 1;
        }
        return 1;
    }
    if(m == 0xDA) {
        PC += 2;
        uint16_t addr = memory_read16(PC - 1);
        if(get_flag(FLAG_C)) {
            PC = addr - 1;
        }
        return 1;
    }
    // JUMP HL
    if(m == 0xE9) {
        uint16_t addr = get_register16(REG_H);
        PC = addr - 1;
        return 1;
    }
    // Add n to current address and jump to it
    if(m == 0x18) {
        PC++;
        uint16_t addr = PC + (int8_t)memory_read8(PC) + 1;
        PC = addr - 1;
        return 1;
    }

    // If following condition is true then add n and jump
    if(m == 0x20) {
        PC++;
        if(!get_flag(FLAG_Z)) {
            uint16_t addr = PC + (int8_t)memory_read8(PC) + 1;
            PC = addr - 1;
        }
        return 1;
    }
    if(m == 0x28) {
        PC++;
        if(get_flag(FLAG_Z)) {
            uint16_t addr = PC + (int8_t)memory_read8(PC) + 1;
            PC = addr - 1;
        }
        return 1;
    }
    if(m == 0x30) {
        PC++;
        if(!get_flag(FLAG_C)) {
            uint16_t addr = PC + (int8_t)memory_read8(PC) + 1;
            PC = addr - 1;
        }
        return 1;
    }
    if(m == 0x38) {
        PC++;
        if(get_flag(FLAG_C)) {
            uint16_t addr = PC + (int8_t)memory_read8(PC) + 1;
            PC = addr - 1;
        }
        return 1;
    }
    // CALLS

    // Push address of next instruction onto stack, jump to address nn
    if(m == 0xCD) {
        // Write PC to stack
        memory_write16(SP - 2, PC + 3);
        SP -= 2;

        PC += 2;
        uint16_t addr = memory_read16(PC - 1);
        PC = addr - 1;
        return 1;
    }
    // jump if flag
    if(m == 0xC4) {
        PC += 2;
        uint16_t addr = memory_read16(PC - 1);
        if(!get_flag(FLAG_Z)) {
            // Write PC to stack
            memory_write16(SP - 2, PC + 1);
            SP -= 2;
            PC = addr - 1;
        }
        return 1;
    }
    if(m == 0xCC) {
        PC += 2;
        uint16_t addr = memory_read16(PC - 1);
        if(get_flag(FLAG_Z)) {
            // Write PC to stack
            memory_write16(SP - 2, PC + 1);
            SP -= 2;
            PC = addr - 1;
        }
        return 1;
    }
    if(m == 0xD4) {
        PC += 2;
        uint16_t addr = memory_read16(PC - 1);
        if(!get_flag(FLAG_C)) {
            // Write PC to stack
            memory_write16(SP - 2, PC + 1);
            SP -= 2;
            PC = addr - 1;
        }
        return 1;
    }
    if(m == 0xDC) {
        PC += 2;
        uint16_t addr = memory_read16(PC - 1);
        if(get_flag(FLAG_C)) {
            // Write PC to stack
            memory_write16(SP - 2, PC + 1);
            SP -= 2;
            PC = addr - 1;
        }
        return 1;
    }

    // RESTARTS
    if((m >> 4) >= 0xC && ((m & 0xF) == 0x7 || (m & 0xF) == 0xF)) {
        switch(m) {
            case 0xC7:
            {   
                // NOTE: PC or PC + 1 ????
                memory_write16(SP - 2, PC + 1);
                SP -= 2;
                PC = 0 - 1;
                break;
            }
            case 0xCF:
            {   
                // NOTE: PC or PC + 1 ????
                memory_write16(SP - 2, PC + 1);
                SP -= 2;
                PC = 0x8 - 1;
                break;
            }
            case 0xD7:
            {   
                // NOTE: PC or PC + 1 ????
                memory_write16(SP - 2, PC + 1);
                SP -= 2;
                PC = 0x10 - 1;
                break;
            }
            case 0xDF:
            {   
                // NOTE: PC or PC + 1 ????
                memory_write16(SP - 2, PC + 1);
                SP -= 2;
                PC = 0x18 - 1;
                break;
            }
            case 0xE7:
            {   
                // NOTE: PC or PC + 1 ????
                memory_write16(SP - 2, PC + 1);
                SP -= 2;
                PC = 0x20 - 1;
                break;
            }
            case 0xEF:
            {   
                // NOTE: PC or PC + 1 ????
                memory_write16(SP - 2, PC + 1);
                SP -= 2;
                PC = 0x28 - 1;
                break;
            }
            case 0xF7:
            {   
                // NOTE: PC or PC + 1 ????
                memory_write16(SP - 2, PC + 1);
                SP -= 2;
                PC = 0x30 - 1;
                break;
            }
            case 0xFF:
            {   
                // NOTE: PC or PC + 1 ????
                memory_write16(SP - 2, PC + 1);
                SP -= 2;
                PC = 0x38 - 1;
                break;
            }
        }
        return 1;
    }

    // RETURNS

    // pop 2 from stack & jump
    if(m == 0xC9) {
        PC = memory_read16(SP) - 1;
        memory_write16(SP, 0);
        SP += 2;
        return 1;
    }

    // pop 2 from stack & jump if flag
    if(m == 0xC0) {
        if(!get_flag(FLAG_Z)) {
            PC = memory_read16(SP) - 1;
            memory_write16(SP, 0);
            SP += 2;
        }
        return 1;
    }
    if(m == 0xC8) {
        if(get_flag(FLAG_Z)) {
            PC = memory_read16(SP) - 1;
            memory_write16(SP, 0);
            SP += 2;
        }
        return 1;
    }
    if(m == 0xD0) {
        if(!get_flag(FLAG_C)) {
            PC = memory_read16(SP) - 1;
            memory_write16(SP, 0);
            SP += 2;
        }
        return 1;
    }
    if(m == 0xD8) {
        if(get_flag(FLAG_C)) {
            PC = memory_read16(SP) - 1;
            memory_write16(SP, 0);
            SP += 2;
        }
        return 1;
    }

    // RETurn and enable INT
    if(m == 0xD9) {
        PC = memory_read16(SP) - 1;
        memory_write16(SP, 0);
        SP += 2;
        interrupt_enable = 1;
        return 1;
    }

    // Disable Interrupt
    if(m == 0xF3) {
        interrupt_enable = 0;
        return 1;
    }
    // Enable interrupt
    if(m == 0xFB) {
        interrupt_enable = 1;
        return 1;
    }

    // 8 bit operations
    if(m == 0xCB) {
        OP_BITOP8();
        return 1;
    }

    printf("THIS OPCODE IS UNKNOWN!!!!\n");
    return 0;
}


void OP_ALU16 () {
    uint8_t m = memory_read8(PC);
    uint16_t hl = get_register16(REG_H);
    switch(m) {
        // ADD hl, n
        case 0x09:
        {
            uint32_t res = hl + get_register16(REG_B);
            set_flag(FLAG_N, 0);
            set_flag(FLAG_H, _half_carry16(hl, get_register16(REG_B)));
            set_flag(FLAG_C, res > 0xFFFF);
            set_register16(REG_H, res);
            return;
        }
        case 0x19:
        {
            uint32_t res = hl + get_register16(REG_D);
            set_flag(FLAG_N, 0);
            set_flag(FLAG_H, _half_carry16(hl, get_register16(REG_D)));
            set_flag(FLAG_C, res > 0xFFFF);
            set_register16(REG_H, res);
            return;
        }
        case 0x29:
        {
            uint32_t res = hl + get_register16(REG_H);
            set_flag(FLAG_N, 0);
            set_flag(FLAG_H, _half_carry16(hl, get_register16(REG_H)));
            set_flag(FLAG_C, res > 0xFFFF);
            set_register16(REG_H, res);
            return;
        }
        case 0x39:
        {
            uint32_t res = hl + SP;
            set_flag(FLAG_N, 0);
            set_flag(FLAG_H, _half_carry16(hl, SP));
            set_flag(FLAG_C, res > 0xFFFF);
            set_register16(REG_H, res);
            return;
        }
        // ADD sp,n
        case 0xE8:
        {
            PC++;
            uint32_t res = (int8_t)memory_read8(PC) + SP;
            set_flag(FLAG_N, 0);
            set_flag(FLAG_Z, 0);
            set_flag(FLAG_H, _half_carry16(memory_read8(PC), SP));
            set_flag(FLAG_C, res > 0xFFFF || res < 0);
            SP = res;
            //PC += (int8_t)memory_read8(PC) - 1;
            return;
        }
        // INC nn
        case 0x03:
        {
            uint32_t res = get_register16(REG_B) + 1;
            set_register16(REG_B, res);
            return;
        }
        case 0x13:
        {
            uint32_t res = get_register16(REG_D) + 1;
            set_register16(REG_D, res);
            return;
        }
        case 0x23:
        {
            uint32_t res = get_register16(REG_H) + 1;
            set_register16(REG_H, res);
            return;
        }
        case 0x33:
        {
            uint32_t res = SP + 1;
            set_register16(REG_H, res);
            return;
        }
        // DEC nn
        case 0x0B:
        {
            uint32_t res = get_register16(REG_B) - 1;
            set_register16(REG_B, res);
            return;
        }
        case 0x1B:
        {
            uint32_t res = get_register16(REG_D) - 1;
            set_register16(REG_D, res);
            return;
        }
        case 0x2B:
        {
            uint32_t res = get_register16(REG_H) - 1;
            set_register16(REG_H, res);
            return;
        }
        case 0x3B:
        {
            SP--;
            return;
        }
    }

    printf("ALU16 MISSED\n");
}

void OP_ALU8 () {
    uint8_t m = memory_read8(PC);
    uint8_t reg_a_val = get_register8(REG_A);


    // Complement A
    if(m == 0x2F) {
        set_register8(REG_A, ~reg_a_val);
        return;
    }

    // Add n to A
    if(m == 0x87) {
        uint16_t res = reg_a_val + get_register8(REG_A);
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, res > 255);
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, _half_carry8(reg_a_val, get_register8(REG_A)));
        set_register8(REG_A, res);
        return;
    }
    if(m >= 0x80 && m <= 0x85) {
        uint16_t res = reg_a_val + get_register8(m - 0x80 + REG_B);
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, res > 255);
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, _half_carry8(reg_a_val, get_register8(m - 0x80 + REG_B)));
        set_register8(REG_A, res);
        return;
    }
    if(m == 0x86) {
        uint16_t res = reg_a_val + memory_read8(get_register16(REG_H));
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, res > 255);
        set_flag(FLAG_H, _half_carry8(reg_a_val, memory_read8(get_register16(REG_H))));
        set_flag(FLAG_N, 0);
        set_register8(REG_A, res);
        return;
    }
    if(m == 0xC6) {
        PC++;
        uint16_t res = reg_a_val + memory_read8(PC);
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, res > 255);
        set_flag(FLAG_H, _half_carry8(reg_a_val, memory_read8(PC)));
        set_flag(FLAG_N, 0);
        set_register8(REG_A, res);
        return;
    }

    // Add n + Carry flag to A.
    if(m == 0x8F) {
        uint16_t res = reg_a_val + get_register8(REG_A);
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, res > 255);
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, _half_carry8(reg_a_val, get_register8(REG_A)));
        set_register8(REG_A, res + (res > 255));
        return;
    }
    if(m >= 0x88 && m <= 0x8D) {
        uint16_t res = reg_a_val + get_register8(m - 0x88 + REG_B);
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, res > 255);
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, _half_carry8(reg_a_val, get_register8(m - 0x88 + REG_B)));
        set_register8(REG_A, res + (res > 255));
        return;
    }
    if(m == 0x8E) {
        uint16_t res = reg_a_val + memory_read8(get_register16(REG_H));
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, res > 255);
        set_flag(FLAG_H, _half_carry8(reg_a_val, memory_read8(get_register16(REG_H))));
        set_flag(FLAG_N, 0);
        set_register8(REG_A, res + (res > 255));
        return;
    }
    if(m == 0xCE) {
        PC++;
        uint16_t res = reg_a_val + memory_read8(PC);
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, res > 255);
        set_flag(FLAG_H, _half_carry8(reg_a_val, memory_read8(PC)));
        set_flag(FLAG_N, 0);
        set_register8(REG_A, res + (res > 255));
        return;
    }

    // SUB n from A
    if(m == 0x97) {
        int16_t res = reg_a_val - get_register8(REG_A);
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, res < 0);
        set_flag(FLAG_N, 1);
        set_flag(FLAG_H, _half_carry_sub8(reg_a_val, get_register8(REG_A)));
        set_register8(REG_A, res);
        return;
    }
    if(m >= 0x90 && m <= 0x95) {
        int16_t res = reg_a_val - get_register8(m - 0x90 + REG_B);
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, res < 0);
        set_flag(FLAG_N, 1);
        set_flag(FLAG_H, _half_carry_sub8(reg_a_val, get_register8(m - 0x90 + REG_B)));
        set_register8(REG_A, res);
        return;
    }
    if(m == 0x96) {
        int16_t res = reg_a_val - memory_read8(get_register16(REG_H));
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, res < 0);
        set_flag(FLAG_H, _half_carry_sub8(reg_a_val, memory_read8(get_register16(REG_H))));
        set_flag(FLAG_N, 1);
        set_register8(REG_A, res);
        return;
    }
    if(m == 0xC6) {
        PC++;
        int16_t res = reg_a_val - memory_read8(PC);
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, res < 0);
        set_flag(FLAG_H, _half_carry_sub8(reg_a_val, memory_read8(PC)));
        set_flag(FLAG_N, 1);
        set_register8(REG_A, res);
        return;
    }

    // SUB n + carry flag from A
    if(m == 0x9F) {
        int16_t res = reg_a_val - get_register8(REG_A);
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_N, 1);
        set_flag(FLAG_H, _half_carry_sub8(reg_a_val, get_register8(REG_A)));
        set_register8(REG_A, res - get_flag(FLAG_C));
        set_flag(FLAG_C, res < 0);
        return;
    }
    if(m >= 0x98 && m <= 0x9D) {
        int16_t res = reg_a_val - get_register8(m - 0x98 + REG_B);
        set_flag(FLAG_Z, res == 0);
        
        set_flag(FLAG_N, 1);
        set_flag(FLAG_H, _half_carry_sub8(reg_a_val, get_register8(m - 0x98 + REG_B)));
        set_register8(REG_A, res - get_flag(FLAG_C));
        set_flag(FLAG_C, res < 0);
        return;
    }
    if(m == 0x9E) {
        int16_t res = reg_a_val - memory_read8(get_register16(REG_H));
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_H, _half_carry_sub8(reg_a_val, memory_read8(get_register16(REG_H))));
        set_flag(FLAG_N, 1);
        set_register8(REG_A, res - get_flag(FLAG_C));
        set_flag(FLAG_C, res < 0);
        return;
    }
    if(m == 0xDE) {
        PC++;
        int16_t res = reg_a_val - memory_read8(PC);
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, res < 0);
        set_flag(FLAG_H, _half_carry_sub8(reg_a_val, memory_read8(PC)));
        set_flag(FLAG_N, 1);
        set_register8(REG_A, res - get_flag(FLAG_C));
        set_flag(FLAG_C, res < 0);
        return;
    }
    
    // AND n with A, store in A
    if(m == 0xA7) {
        uint8_t res = get_register8(REG_A) & reg_a_val;
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, 0);
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, 1);
        set_register8(REG_A, res);
        return;
    }
    if(m >= 0xA0 && m <= 0xA5) {
        uint8_t res =  get_register8(m - 0xA0 + REG_B) & reg_a_val;
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, 0);
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, 1);
        set_register8(REG_A, res);
        return;
    }
    if(m == 0xA6) {
        uint8_t res =  memory_read8(get_register16(REG_H)) & reg_a_val;
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, 0);
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, 1);
        set_register8(REG_A, res);
        return;
    }
    if(m == 0xE6) {
        PC++;
        uint8_t res = memory_read8(PC) & reg_a_val;
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, 0);
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, 1);
        set_register8(REG_A, res);
        return;
    }

    // OR n with A, store in A
    if(m == 0xB7) {
        uint8_t res = get_register8(REG_A) | reg_a_val;
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, 0);
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, 0);
        set_register8(REG_A, res);
        return;
    }
    if(m >= 0xB0 && m <= 0xB5) {
        uint8_t res =  get_register8(m - 0xB0 + REG_B) | reg_a_val;
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, 0);
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, 0);
        set_register8(REG_A, res);
        return;
    }
    if(m == 0xB6) {
        uint8_t res =  memory_read8(get_register16(REG_H)) | reg_a_val;
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, 0);
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, 0);
        set_register8(REG_A, res);
        return;
    }
    if(m == 0xF6) {
        PC++;
        uint8_t res = memory_read8(PC) | reg_a_val;
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, 0);
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, 0);
        set_register8(REG_A, res);
        return;
    }

    // XOR n with A, store in A
    if(m == 0xAF) {
        uint8_t res = get_register8(REG_A) ^ reg_a_val;
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, 0);
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, 0);
        set_register8(REG_A, res);
        return;
    }
    if(m >= 0xA8 && m <= 0xAD) {
        uint8_t res =  get_register8(m - 0xA8 + REG_B) ^ reg_a_val;
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, 0);
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, 0);
        set_register8(REG_A, res);
        return;
    }
    if(m == 0xAE) {
        uint8_t res =  memory_read8(get_register16(REG_H)) ^ reg_a_val;
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, 0);
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, 0);
        set_register8(REG_A, res);
        return;
    }
    if(m == 0xEE) {
        PC++;
        uint8_t res = memory_read8(PC) ^ reg_a_val;
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, 0);
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, 0);
        set_register8(REG_A, res);
        return;
    }

    // Compare A with N
    if(m == 0xBF) {
        int16_t res = reg_a_val - get_register8(REG_A);
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, res < 0);
        set_flag(FLAG_N, 1);
        set_flag(FLAG_H, _half_carry_sub8(reg_a_val, get_register8(REG_A)));
        return;
    }
    if(m >= 0xB8 && m <= 0xBD) {
        int16_t res = reg_a_val - get_register8(m - 0xB8 + REG_B);
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, res < 0);
        set_flag(FLAG_N, 1);
        set_flag(FLAG_H, _half_carry_sub8(reg_a_val, get_register8(m - 0xB8 + REG_B)));
        return;
    }
    if(m == 0xBE) {
        int16_t res = reg_a_val - memory_read8(get_register16(REG_H));
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, res < 0);
        //printf("COMPARISON: %i < %i = %i\n", reg_a_val, memory_read8(get_register16(REG_H)), res < 0);
        set_flag(FLAG_H, _half_carry_sub8(reg_a_val, memory_read8(get_register16(REG_H))));
        set_flag(FLAG_N, 1);
        return;
    }
    if(m == 0xFE) {
        PC++;
        int16_t res = reg_a_val - memory_read8(PC);
        set_flag(FLAG_Z, res == 0);
        set_flag(FLAG_C, res < 0);
        set_flag(FLAG_H, _half_carry_sub8(reg_a_val, memory_read8(PC)));
        set_flag(FLAG_N, 1);
        return;
    }

    // INC n
    if(((m >= 0x04 && m <= 0x3C) && ((m & 0xF) == 0xC || (m & 0xF) == 0x4))) {
        switch(m) {
            case 0x3C:
            {
                uint16_t res = reg_a_val + 1;
                set_flag(FLAG_Z, res == 0);
                set_flag(FLAG_H, _half_carry8(reg_a_val, 1));
                set_flag(FLAG_N, 0);
                set_register8(REG_A, res);
                break;
            }
            case 0x04:
            {
                uint16_t res = get_register8(REG_B) + 1;
                set_flag(FLAG_Z, res == 0);
                set_flag(FLAG_H, _half_carry8(get_register8(REG_B), 1));
                set_flag(FLAG_N, 0);
                set_register8(REG_B, res);
                break;
            }
            case 0x0C:
            {
                uint16_t res = get_register8(REG_C) + 1;
                set_flag(FLAG_Z, res == 0);
                set_flag(FLAG_H, _half_carry8(get_register8(REG_C), 1));
                set_flag(FLAG_N, 0);
                set_register8(REG_C, res);
                break;
            }
            case 0x14:
            {
                uint16_t res = get_register8(REG_D) + 1;
                set_flag(FLAG_Z, res == 0);
                set_flag(FLAG_H, _half_carry8(get_register8(REG_D), 1));
                set_flag(FLAG_N, 0);
                set_register8(REG_D, res);
                break;
            }
            case 0x1C:
            {
                uint16_t res = get_register8(REG_E) + 1;
                set_flag(FLAG_Z, res == 0);
                set_flag(FLAG_H, _half_carry8(get_register8(REG_E), 1));
                set_flag(FLAG_N, 0);
                set_register8(REG_E, res);
                break;
            }
            case 0x24:
            {
                uint16_t res = get_register8(REG_H) + 1;
                set_flag(FLAG_Z, res == 0);
                set_flag(FLAG_H, _half_carry8(get_register8(REG_H), 1));
                set_flag(FLAG_N, 0);
                set_register8(REG_H, res);
                break;
            }
            case 0x2C:
            {
                uint16_t res = get_register8(REG_L) + 1;
                set_flag(FLAG_Z, res == 0);
                set_flag(FLAG_H, _half_carry8(get_register8(REG_L), 1));
                set_flag(FLAG_N, 0);
                set_register8(REG_L, res);
                break;
            }
            case 0x34:
            {
                uint16_t res = memory_read8(get_register16(REG_H)) + 1;
                set_flag(FLAG_Z, res == 0);
                set_flag(FLAG_H, _half_carry8(memory_read8(get_register16(REG_H)), 1));
                set_flag(FLAG_N, 0);
                memory_write8(get_register16(REG_H), res);
                break;
            }
        }
        return;
    }
    // DEC n
    if(((m >= 0x05 && m <= 0x3D) && ((m & 0xF) == 0xD || (m & 0xF) == 0x5))) {
        switch(m) {
            case 0x3D:
            {
                int16_t res = reg_a_val - 1;
                set_flag(FLAG_Z, res == 0);
                set_flag(FLAG_H, _half_carry_sub8(reg_a_val, 1));
                set_flag(FLAG_N, 1);
                set_register8(REG_A, res);
                break;
            }
            case 0x05:
            {
                int16_t res = get_register8(REG_B) - 1;
                set_flag(FLAG_Z, res == 0);
                set_flag(FLAG_H, _half_carry_sub8(get_register8(REG_B), 1));
                set_flag(FLAG_N, 1);
                set_register8(REG_B, res);
                break;
            }
            case 0x0D:
            {
                int16_t res = get_register8(REG_C) - 1;
                set_flag(FLAG_Z, res == 0);
                set_flag(FLAG_H, _half_carry_sub8(get_register8(REG_C), 1));
                set_flag(FLAG_N, 1);
                set_register8(REG_C, res);
                break;
            }
            case 0x15:
            {
                int16_t res = get_register8(REG_D) - 1;
                set_flag(FLAG_Z, res == 0);
                set_flag(FLAG_H, _half_carry_sub8(get_register8(REG_D), 1));
                set_flag(FLAG_N, 1);
                set_register8(REG_D, res);
                break;
            }
            case 0x1D:
            {
                int16_t res = get_register8(REG_E) - 1;
                set_flag(FLAG_Z, res == 0);
                set_flag(FLAG_H, _half_carry_sub8(get_register8(REG_E), 1));
                set_flag(FLAG_N, 1);
                set_register8(REG_E, res);
                break;
            }
            case 0x25:
            {
                int16_t res = get_register8(REG_H) - 1;
                set_flag(FLAG_Z, res == 0);
                set_flag(FLAG_H, _half_carry_sub8(get_register8(REG_H), 1));
                set_flag(FLAG_N, 1);
                set_register8(REG_H, res);
                break;
            }
            case 0x2D:
            {
                int16_t res = get_register8(REG_L) - 1;
                set_flag(FLAG_Z, res == 0);
                set_flag(FLAG_H, _half_carry_sub8(get_register8(REG_L), 1));
                set_flag(FLAG_N, 1);
                set_register8(REG_L, res);
                break;
            }
            case 0x35:
            {
                int16_t res = memory_read8(get_register16(REG_H)) - 1;
                set_flag(FLAG_Z, res == 0);
                set_flag(FLAG_H, _half_carry_sub8(memory_read8(get_register16(REG_H)), 1));
                set_flag(FLAG_N, 1);
                memory_write8(get_register16(REG_H), res);
                break;
            }
        }
        return;
    }
    printf("ALU8 MISSED\n");

}

void OP_LD16 () {
    uint8_t m = memory_read8(PC);
    if(m >> 4 <= 0x3 && ((m & 0xF) == 0x1)) {
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
        memory_write16(memory_read16(PC - 1), SP);
        return;
    }

    if(m >> 4 >= 0xC && (m & 0xF) == 0x5) {
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
    if(m >> 4 >= 0xC && (m & 0xF) == 0x1) {
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
    printf("O16 MISSED 0x%x\n", PC);

}

// 8 bit Load operations
void OP_LD8 () {
    uint8_t m = memory_read8(PC);

    // put value nn into n
    if((m >= 0x06 && m <= 0x2E) && ((m & 0xF) == 0x6 || (m & 0xF) == 0xE)) {
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
    if(((m >= 0x47 && m <= 0x7F) && ((m & 0xF) == 0xF || (m & 0xF) == 0x7)) || m == 0x02 || m == 0x12 || m == 0xEA) {
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
        set_register8(REG_A, memory_read8(get_register16(REG_H)));
        set_register16(REG_H, get_register16(REG_H) - 1);
        return;
    }

    // Put value at address HL into A, inc HL
    if(m == 0x2A) {
        set_register8(REG_A, memory_read8(get_register16(REG_H)));
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
    if(m == 0x22) {
        memory_write8(get_register16(REG_H), get_register8(REG_A));
        set_register16(REG_H, get_register16(REG_H) + 1);
        return;
    }

    // Put value A to $FF00 + n
    if(m == 0xE0) {
        PC++;
        memory_write8(0xFF00 + memory_read8(PC), get_register8(REG_A));
        return;
    }

    // Put value $FF00 + n to A
    if(m == 0xF0) {
        PC++;
        set_register8(REG_A, memory_read8(0xFF00 + memory_read8(PC)));
        return;
    }


    printf("*************OP8 missed!!!\n");
}

// CB COMMANDS
void OP_BITOP8 () {
    PC++;
    uint8_t m = memory_read8(PC);

    // SWAP UPPER & LOWER NIBBLES
    if(m >= 0x30 && m <= 0x37) {
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, 0);
        set_flag(FLAG_C, 0);
        if(m == 0x37) {
            uint8_t ax = get_register8(REG_A);
            uint8_t n = (ax & 0xF0) | (ax >> 4);
            set_register8(REG_A, n);
            set_flag(FLAG_Z, n == 0);
            
        }
        if(m >= 0x30 && m <= 0x35) {
            uint8_t ax = get_register8(m - 0x30 + REG_B);
            uint8_t n = (ax & 0xF0) | (ax >> 4);
            set_register8(m - 0x30 + REG_B, n);
            set_flag(FLAG_Z, n == 0);
            
        }
        if(m == 0x36) {
            uint8_t ax = memory_read8(memory_read16(REG_H));
            uint8_t n = (ax & 0xF0) | (ax >> 4);
            memory_write8(memory_read16(REG_H), n);
            set_flag(FLAG_Z, n == 0);
            
        }
        return;
    }

    if(m >= 0x00 && m <= 0x07) {
        // RLC
        if(m == 0x07) {
            set_flag(FLAG_Z, get_register8(REG_A) == 0);
            set_flag(FLAG_C, (get_register8(REG_A) >> 7) & 0x01);
            set_flag(FLAG_N, 0);
            set_flag(FLAG_H, 0);
            set_register8(REG_A, rotate_left8(get_register8(REG_A), 1));
            
        }
        if(m <= 0x05) {
            set_flag(FLAG_Z, get_register8(m + REG_B) == 0);
            set_flag(FLAG_C, (get_register8(m + REG_B) >> 7) & 0x01);
            set_flag(FLAG_N, 0);
            set_flag(FLAG_H, 0);
            set_register8(REG_A, rotate_left8(get_register8(m + REG_B), 1));
        }
        if(m == 0x06) {
            set_flag(FLAG_Z, memory_read8(get_register16(REG_H)) == 0);
            set_flag(FLAG_C, (memory_read8(get_register16(REG_H)) >> 7) & 0x01);
            set_flag(FLAG_N, 0);
            set_flag(FLAG_H, 0);
            memory_write8(get_register16(REG_H), rotate_left8(memory_read8(get_register16(REG_H)), 1));
            
        }
        return;
    }
    if(m >= 0x08 && m <= 0x0F) {
        // RRC
        if(m == 0x0F) {
            set_flag(FLAG_Z, get_register8(REG_A) == 0);
            set_flag(FLAG_C, (get_register8(REG_A)) & 0x01);
            set_flag(FLAG_N, 0);
            set_flag(FLAG_H, 0);
            set_register8(REG_A, rotate_right8(get_register8(REG_A), 1));
            
        }
        if(m >= 0x08 && m <= 0x0D) {
            set_flag(FLAG_Z, get_register8(m - 0x08 + REG_B) == 0);
            set_flag(FLAG_C, (get_register8(m - 0x08 + REG_B)) & 0x01);
            set_flag(FLAG_N, 0);
            set_flag(FLAG_H, 0);
            set_register8(REG_A, rotate_right8(get_register8(m - 0x08 + REG_B), 1));
        }
        if(m == 0x0E) {
            set_flag(FLAG_Z, memory_read8(get_register16(REG_H)) == 0);
            set_flag(FLAG_C, (memory_read8(get_register16(REG_H))) & 0x01);
            set_flag(FLAG_N, 0);
            set_flag(FLAG_H, 0);
            memory_write8(get_register16(REG_H), rotate_right8(memory_read8(get_register16(REG_H)), 1));
            
        }
        return;
    }

    if(m >= 0x10 && m <= 0x17) {
        // RL n
        if(m == 0x17) {
            
            set_flag(FLAG_N, 0);
            set_flag(FLAG_H, 0);
            uint8_t c = get_flag(FLAG_C);
            set_flag(FLAG_C, (get_register8(REG_A) >> 7) & 0x01);
            set_register8(REG_A, (get_register8(REG_A) << 1) | (c & 0x1));
            set_flag(FLAG_Z, get_register8(REG_A) == 0);
            
        }
        if(m >= 0x10 && m <= 0x15) {
            set_flag(FLAG_N, 0);
            set_flag(FLAG_H, 0);
            uint8_t c = get_flag(FLAG_C);
            set_flag(FLAG_C, (get_register8(m - 0x10 + REG_B) >> 7) & 0x01);
            set_register8(m - 0x10 + REG_B, (c & 0x1) | (get_register8(m - 0x10 + REG_B) << 1));
            set_flag(FLAG_Z, get_register8(m - 0x10 + REG_B) == 0);
        }
        if(m == 0x16) {
            set_flag(FLAG_N, 0);
            set_flag(FLAG_H, 0);
            uint8_t c = get_flag(FLAG_C);
            set_flag(FLAG_C, (memory_read8(get_register16(REG_H)) >> 7) & 0x01);
            memory_write8(get_register16(REG_H), (c & 0x1) | (memory_read8(get_register16(REG_H)) << 1));
            set_flag(FLAG_Z, memory_read8(get_register16(REG_H)) == 0);

        }
        return;
    }
    if(m >= 0x18 && m <= 0x1F) {
        // RR n
        if(m == 0x1F) {
            
            set_flag(FLAG_N, 0);
            set_flag(FLAG_H, 0);
            uint8_t c = get_flag(FLAG_C);
            set_flag(FLAG_C, (get_register8(REG_A)) & 0x01);
            set_register8(REG_A, (c << 7) | (get_register8(REG_A) >> 1));
            set_flag(FLAG_Z, get_register8(REG_A) == 0);
            
        }
        if(m >= 0x18 && m <= 0x1D) {
            set_flag(FLAG_N, 0);
            set_flag(FLAG_H, 0);
            uint8_t c = get_flag(FLAG_C);
            set_flag(FLAG_C, (get_register8(m - 0x18 + REG_B)) & 0x01);
            set_register8(m - 0x18 + REG_B, (c << 7) | (get_register8(m - 0x18 + REG_B) >> 1));
            set_flag(FLAG_Z, get_register8(m - 0x18 + REG_B) == 0);
        }
        if(m == 0x1E) {
            set_flag(FLAG_N, 0);
            set_flag(FLAG_H, 0);
            uint8_t c = get_flag(FLAG_C);
            set_flag(FLAG_C, (memory_read8(get_register16(REG_H))) & 0x01);
            memory_write8(get_register16(REG_H), (c << 7) | (memory_read8(get_register16(REG_H)) >> 1));
            set_flag(FLAG_Z, memory_read8(get_register16(REG_H)) == 0);
        }
        return;
    }

    // BIT tests
    if(m >= 0x40 && m <= 0x7F) {
        uint8_t offst = (m - 0x40)/8;
        set_flag(FLAG_H, 1);
        set_flag(FLAG_N, 0);
        uint8_t v;
        if(((m - 0x40) % 8) + REG_B > REG_L) {
            if(((m - 0x40) % 8) + REG_B == 8) {
                v = memory_read8(get_register16(REG_H));
            }
            else if(((m - 0x40) % 8) + REG_B == 9) {
                v = get_register8(REG_A);
            }
        }
        else {
            v = get_register8(((m - 0x40) % 8) + REG_B);
        }
        //printf("BIT %i OF REGISTER %i SET FLAG Z: %i\n", offst, ((m - 0x40) % 8) + REG_B, ((v >> offst) & 0x01) == 0);
        set_flag(FLAG_Z, ((v >> offst) & 0x01) == 0);

        return;
    }

    // RES
    if(m >= 0x80 && m <= 0xBF) {
        uint8_t offst = (m - 0x80)/8;
        uint8_t v;
        if(((m - 0x80) % 8) + REG_B > REG_L) {
            if(((m - 0x80) % 8) + REG_B == 8) {
                v = memory_read8(get_register16(REG_H));
                v &= ~(1UL << offst);
                memory_write8(get_register16(REG_H), v);
            }
            else if(((m - 0x80) % 8) + REG_B == 9) {
                v = get_register8(REG_A);
                v &= ~(1UL << offst);
                memory_write8(get_register16(REG_A), v);
            }
        }
        else {
            v = get_register8(((m - 0x80) % 8) + REG_B);
            v &= ~(1UL << offst);
            memory_write8(((m - 0x80) % 8) + REG_B, v);
        }

        return;
    }

    /*
    // RRA
    if(m == 0x0F) {

        set_flag(FLAG_Z, get_register8(REG_A) == 0);
        set_flag(FLAG_N, 0);
        set_flag(FLAG_H, 0);
        uint8_t c = get_flag(FLAG_C);
        set_flag(FLAG_C, get_register8(REG_A) & 0x01);
        set_register8(REG_A, (c << 7) | (get_register8(REG_A) >> 1));
        return 1;
    }
    */

    printf("UNKNOWN CB %x\n", m);
    
}

uint8_t _half_carry16 (uint16_t a, uint16_t b) {
    return ((((a >> 8) & 0xf) + ((b >> 8) & 0xf)) & 0x10) == 0x10;
}

uint8_t _half_carry8 (uint8_t a, uint8_t b) {
    return (((a & 0xf) + (b & 0xf)) & 0x10) == 0x10;
}

uint8_t _half_carry_sub8 (uint8_t a, uint8_t b) {
    return ((((int)a & 0xF) - ((int)b & 0xF)) < 0);
}

uint8_t _carry16 (uint16_t a, uint16_t b) {
    return ((uint32_t)a + (uint32_t)b > 0xFFFF);
}

uint8_t rotate_right8 (uint8_t n, uint8_t d) { 
    return (n >> d)|(n << (8 - d)); 
} 

uint8_t rotate_left8 (uint8_t n, uint8_t d) { 
    return (n << d)|(n >> (8 - d)); 
} 

int vblank_c = 0;

unsigned long last_vblank = 0;

struct timespec t;
struct timespec t2;

uint8_t debugmode = 1;

uint8_t run () {
    window_update(w);
    gettimeofday(&start, NULL);
    t.tv_sec = 0;
    t.tv_nsec = 500;
    while(1) {
            //printf("********************PC: 0x%2x, M: 0x%2x\n", PC, 0);

        //print_registers();
        if(debugmode) {
            
            //getchar();

        }
        //nanosleep(&t, &t2);
        for(int x = 0; x < 0x8FF; x++);

        if(MODE_STOP == 1)
            continue;
        printf("PC: 0x%x\n", PC);
        // No processor updates when in MODE 2 (HALT)
        if(MODE_STOP == 0) {
            parse_op();
            PC++;
            clocks += 4;
        }
        //getchar();
        //usleep(1);
        display_update();
        /*
        if(vblank_c >= 310) {
            disp_worker();
            vblank_c = 0;
        }
        vblank_c++;
        last_vblank = 0;
        */
        
    }
}