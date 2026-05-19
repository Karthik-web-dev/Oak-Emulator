#ifndef GAMEBOYEMULATOR_CPU_H
#define GAMEBOYEMULATOR_CPU_H

#include "../core/MMU.h"
#include <cstdint>
#include "Instructions.h"

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

struct ExecState {
    uint8_t opcode; //instruction identity
    uint8_t step; //execution process or count of how many M cycles
    uint8_t tmp; // scratch or temp state to be preserved between cycles.
    uint16_t tmp16;
    Instruction* instr;
    bool condition_taken = false;
};


class SM83 {
public:
    uint8_t A, F;
    uint8_t B, C;
    uint8_t D, E;
    uint8_t H, L;

    uint16_t SP;
    uint16_t PC;

    MEMORY& memory;

    SM83(MEMORY& mem);

    enum Phase {
        FETCH,
        DECODE,
        EXECUTE,
    };

    Phase phase = FETCH;
    ExecState exec;

    void cpu_tick(void);

    bool IME;
    bool IME_next;
    void handle_interrupts(void);

    uint8_t& getR(int r);
    uint16_t getRR(int rr);
    void setR(int r, uint8_t val);
    void setRR(int rr, uint16_t val);

    uint16_t getHL();
    uint16_t getBC();
    uint16_t getDE();

    void ld_r_r(ExecState& exec);
    void ld_r_d8(ExecState& exec);
    void ld_rr_d16(ExecState& exec);
    void ld_a_rr(ExecState& exec);
    void ld_rr_a(ExecState& exec);
    void ld_a_nn(ExecState& exec);
    void ld_nn_a(ExecState& exec);
    void ld_sp_hl(ExecState& exec);
    void ld_hl_sp_n(ExecState& exec);

    void inc_r(ExecState& exec);
    void inc_rr(ExecState& exec);
    void dec_r(ExecState& exec);
    void dec_rr(ExecState& exec);

    void add_a_r(ExecState& exec);
    void add_a_n(ExecState& exec);
    void add_hl_rr(ExecState& exec);
    void add_sp_n(ExecState& exec);

    void adc_r(ExecState& exec);
    void adc_n(ExecState& exec);

    void sub_a_r(ExecState& exec);
    void sub_a_n(ExecState& exec);
    void sub_hl_rr(ExecState& exec);
    void sub_sp_n(ExecState& exec);

    void sbc_r(ExecState& exec);
    void sbc_n(ExecState& exec);

    void rlca(ExecState& exec);
    void rrca(ExecState& exec);
    void rla(ExecState& exec);
    void stop(ExecState& exec);

    void jr(ExecState& exec);

    void rra(ExecState& exec);
    void cpl(ExecState& exec);
    void scf(ExecState& exec);
    void ccf(ExecState& exec);

    void _and_r(ExecState& exec);
    void _and_d8(ExecState& exec);

    void _xor_r(ExecState& exec);
    void _xor_d8(ExecState& exec);

    void or_r(ExecState& exec);
    void or_d8(ExecState& exec);

    void cp_r(ExecState& exec);
    void cp_d8(ExecState& exec);

    void ret_cc(ExecState& exec); 
    void reti(ExecState& exec);
    void pop_rr(ExecState& exec);

    void jp_nn(ExecState& exec);
    void call_nn(ExecState& exec);
    void push_nn(ExecState& exec);
    void rst_n(ExecState& exec);

    void ldh_n_a(ExecState& exec);
    void ldh_a_n(ExecState& exec);

    void ei(ExecState& exec);

    //Prefixed
    void bit_r(ExecState& exec);
    void reset_r(ExecState& exec);
    void set_r(ExecState& exec);
    void swap_r(ExecState& exec);
    void srl_r(ExecState& exec);
    void sla_r(ExecState& exec);
    void sra_r(ExecState& exec);
    void rr_r(ExecState& exec);
    void rl_r(ExecState& exec);
    void rrc_r(ExecState& exec);
    void rlc_r(ExecState& exec);
};



#endif //GAMEBOYEMULATOR_CPU_H
