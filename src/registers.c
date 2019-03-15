#include "registers.h"
#include "math.h"
#include <stdio.h>

uint16_t _registers[4] = { 0 };

uint16_t SP = 0xFFFE;
uint16_t PC = 0x0;

uint8_t interrupt_enable = 1;


void set_bit (uint8_t *a, uint8_t index, uint8_t state) {
    if(state == 0) {
        *a &= ~(1UL << index);
    }
    else {
        *a |= 1UL << index;
    }
}

void set_bit16 (uint16_t *a, uint8_t index, uint8_t state) {
    if(state == 0) {
        *a &= ~(1UL << index);
    }
    else {
        *a |= 1UL << index;
    }
}

uint8_t set_register8 (enum L_REGISTER reg, uint8_t value) {
    uint16_t mask = (reg % 2 == 0 ? 0xFF00 : 0x00FF);
    _registers[reg/2] = (_registers[reg/2] & ~mask) | (((uint16_t)value << (reg % 2 == 0 ? 8 : 0)) & mask);
    return 1;
}

uint8_t set_register16 (enum L_REGISTER reg, uint16_t value) {
    _registers[reg/2] = value;
    return 1;
}

uint8_t set_flag (enum L_FLAG flag, uint8_t state) {
    set_bit16(&_registers[0], flag, state);
    return 1;
}

uint8_t get_register8 (enum L_REGISTER reg) {
    return (_registers[reg/2] >> ((reg % 2) == 0 ? 8 : 0)) & 0xFF;
}

uint16_t get_register16 (enum L_REGISTER reg) {
    return _registers[reg/2];
}

uint8_t get_flag (enum L_FLAG flag) {
    return (_registers[0] >> flag) & 0x01;
}

void print_registers () {
    printf("\tA: 0x%x\n\tF: 0x%x\n\tB: 0x%x\n\tC: 0x%x\n\tD: 0x%x\n\tE: 0x%x\n\tH: 0x%x\n\tL: 0x%x\n",
            get_register8(REG_A), get_register8(REG_F), get_register8(REG_B), get_register8(REG_C), 
            get_register8(REG_D), get_register8(REG_E), get_register8(REG_H), get_register8(REG_L));
}


// Stop mode
uint8_t MODE_STOP = 0;