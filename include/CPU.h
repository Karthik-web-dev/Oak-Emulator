#ifndef GAMEBOYEMULATOR_CPU_H
#define GAMEBOYEMULATOR_CPU_H

#include "Memory.h"
#include <cstdint>

#define FLAG_Z 0x80
#define FLAG_N 0x40
#define FLAG_H 0x20
#define FLAG_C 0x10


#define HALF_CARRY_ADD8(a, b)  ((((a) & 0xF) + ((b) & 0xF)) > 0xF)
#define CARRY_ADD8(a, b)       (((uint16_t)(a) + (uint16_t)(b)) > 0xFF)

#define HALF_CARRY_SUB8(a, b)  (((a) & 0xF) < ((b) & 0xF))
#define CARRY_SUB8(a, b)       ((a) < (b))

#define HALF_CARRY_ADD16(a, b) ((((a) & 0x0FFF) + ((b) & 0x0FFF)) > 0x0FFF)
#define CARRY_ADD16(a, b)      (((uint32_t)(a) + (uint32_t)(b)) > 0xFFFF)



class SM83 {
public:
    uint8_t A, F;
    uint8_t B, C;
    uint8_t D, E;
    uint8_t H, L;

    uint16_t SP;
    uint16_t PC;

    uint64_t cycles;

    MEMORY& memory;

    SM83(MEMORY& mem);
    void execute();
    void stepPC(uint8_t length, uint8_t baseCycles, uint8_t altCycles, bool condition);

    uint8_t& getR(int r);
    uint16_t getRR(int rr);
    void setR(int r, uint8_t val);
    void setRR(int rr, uint16_t val);

    uint16_t getHL();
    uint16_t getBC();
    uint16_t getDE();

    void ld_r_r(uint8_t opcode);
    void ld_r_d8(uint8_t opcode);
    void ld_rr_d16(uint8_t opcode);
    void ld_a_rr(uint8_t opcode);
    void ld_rr_a(uint8_t opcode);
    void ld_a_nn(uint8_t opcode);
    void ld_nn_a(uint8_t opcode);

    void inc_r(uint8_t opcode);
    void inc_rr(uint8_t opcode);
    void dec_r(uint8_t opcode);
    void dec_rr(uint8_t opcode);

    void add_a_r(uint8_t opcode);
    void add_a_n(uint8_t opcode);
    void add_hl_rr(uint8_t opcode);
    void add_sp_n(uint8_t opcode);

    void adc_r(uint8_t opcode);
    void adc_n(uint8_t opcode);

    void sub_a_r(uint8_t opcode);
    void sub_a_n(uint8_t opcode);
    void sub_hl_rr(uint8_t opcode);
    void sub_sp_n(uint8_t opcode);

    void sbc_r(uint8_t opcode);
    void sbc_n(uint8_t opcode);

    void rlca(uint8_t opcode);
    void rrca(uint8_t opcode);
    void rla(uint8_t opcode);
    void stop(uint8_t opcode);

    void jr(uint8_t opcode);

    void rra(uint8_t opcode);
    void cpl(uint8_t opcode);
    void scf(uint8_t opcode);
    void ccf(uint8_t opcode);

    void _and_r(uint8_t opcode);
    void _and_d8(uint8_t opcode);

    void _xor_r(uint8_t opcode);
    void _xor_d8(uint8_t opcode);

    void or_r(uint8_t opcode);
    void or_d8(uint8_t opcode);

    void cp_r(uint8_t opcode);
    void cp_d8(uint8_t opcode);

    void ret_cc(uint8_t opcode); 
    void reti(uint8_t opcode);
    void pop_rr(uint8_t opcode);

    void jp_nn(uint8_t opcode);
    void call_nn(uint8_t opcode);
    void push_nn(uint8_t opcode);
    void rst_n(uint8_t opcode);

    void ldh_n_a(uint8_t opcode);
    void ldh_a_n(uint8_t opcode);

    //Prefixed
    void bit_r(uint8_t opcode);
    void reset_r(uint8_t opcode);
    void set_r(uint8_t opcode);
    void swap_r(uint8_t opcode);
    void srl_r(uint8_t opcode);
    void sla_r(uint8_t opcode);
    void sra_r(uint8_t opcode);
    void rr_r(uint8_t opcode);
    void rl_r(uint8_t opcode);
    void rrc_r(uint8_t opcode);
    void rlc_r(uint8_t opcode);
};



#endif //GAMEBOYEMULATOR_CPU_H
