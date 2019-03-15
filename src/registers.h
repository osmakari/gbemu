/*
    All registers/controls
*/
#include <stdint.h>

// register labels...
typedef enum L_REGISTER { REG_A, REG_F, REG_B, REG_C, REG_D, REG_E, REG_H, REG_L } L_REGISTER;

// Flag labels
typedef enum L_FLAG { FLAG_Z = 7, FLAG_N = 6, FLAG_H = 5, FLAG_C = 4 } L_FLAG;

/*
    15 ... 8 | 7 ... 0
    A (ACC)    F (flags)
    B          C
    D          E
    H          L
*/
extern uint16_t _registers[4];

/*
    F (flags) register bits:
    7: Z | Zero flag
    6: N | Substract flag
    5: H | Half Carry
    4: C | Carry flag

*/

extern uint16_t SP;
extern uint16_t PC;

extern uint8_t MODE_STOP;

extern uint8_t interrupt_enable;


// Register functions


// Sets 1 byte to register
uint8_t set_register8 (enum L_REGISTER reg, uint8_t value);

// Sets 2 bytes to register NOTE: registers are 1 byte, so setting A will set A and F, B will set B and C etc...
uint8_t set_register16 (enum L_REGISTER reg, uint16_t value);

// Sets flag
uint8_t set_flag (enum L_FLAG flag, uint8_t state);

uint8_t get_register8 (enum L_REGISTER reg);

uint16_t get_register16 (enum L_REGISTER reg);

uint8_t get_flag (enum L_FLAG flag);

void print_registers ();