#include<bits/stdc++.h>
#include "CPU.h"
#include "Memory.h"
#include "ALU.h"
#include "Instructions.h"

SM83::SM83(MEMORY& mem) : A(0), F(0), B(0), C(0),
                            D(0), E(0), H(0), L(0), SP(0),
                            PC(0), cycles(0), memory(mem) {};

void SM83::stepPC(const uint8_t length, const uint8_t baseCycles, const uint8_t altCycles, const bool condition) {
    PC += length;
    cycles += condition ? altCycles+baseCycles : baseCycles;
}


inline uint8_t mask8(uint16_t value) {
    return value & 0xFF;
}

inline void setFlags(uint8_t &F, int z, int n, int h, int c) {
    F =  (z ? FLAG_Z : 0) |
         (n ? FLAG_N : 0) |
         (h ? FLAG_H : 0) |
         (c ? FLAG_C : 0);
}

uint8_t& SM83::getR(const int r) {
    switch (r) {
        case 0: return B;
        case 1: return C;
        case 2: return D;
        case 3: return E;
        case 4: return H;
        case 5: return L;
        case 7: return A;
        default:
            throw std::runtime_error("Invalid Register R");
    }
}

uint16_t SM83::getRR(const int rr) {
    switch (rr) {
        case 0: return ((B << 8) | C);
        case 1: return ((D << 8) | E);
        case 2: return ((H << 8) | L);
        case 3: return SP;
        default:
            throw std::runtime_error("Invalid Register RR");
    }
}

void SM83::setR(int r, uint8_t val) {
    switch (r) {
        case 0: B = val; break;
        case 1: C = val; break;
        case 2: D = val; break;
        case 3: E = val; break;
        case 4: H = val; break;
        case 5: L = val; break;
        case 7: A = val; break;
        default: throw std::runtime_error("Invalid Register R");
    }
}

void SM83::setRR(int rr, uint16_t val) {
    switch (rr) {
        case 0: B = val >> 8; C = val & 0xFF; break;
        case 1: D = val >> 8; E = val & 0xFF; break;
        case 2: H = val >> 8; L = val & 0xFF; break;
        case 3: SP = val; break;
        default: throw std::runtime_error("Invalid RR code");
    }
}

uint16_t SM83::getHL() {
    return (H << 8) | L;
}

uint16_t SM83::getBC() {
    return (B << 8) | C;
}

uint16_t SM83::getDE() {
    return (D << 8) | E;
}

void SM83::ld_r_r(uint8_t opcode) {
    uint8_t dst = (opcode >> 3) & 0x07;
    uint8_t src = (opcode & 7);
    if (dst == 6 && src == 6) {
        //HALT

    }

    uint8_t value;
    if (src == 6) {
        value = memory.read8(getHL());
    } else {
        value = getR(src);
    }

    if (dst == 6) {
        memory.write8(getHL(), value);
    }
    else {
        setR(dst, value);
    }
}

void SM83::ld_r_d8(uint8_t opcode) {
    uint8_t r = (opcode >> 3) & 0x07; //dst
    uint8_t d8 = memory.read8(PC + 1); //src

    if (r == 6) {
        memory.write8(getHL(), d8);

    } else {
        setR(r, d8);
    }
}

void SM83::ld_rr_d16(uint8_t opcode) {
    const uint16_t d16 = memory.read16(PC + 1);
    setRR((opcode >> 4) & 0x07, d16);

}

void SM83::ld_a_rr(uint8_t opcode) { //here RR stores addr not value
    uint16_t addr = (opcode & 0x10) ? getDE() : getBC();
    setR(7, memory.read8(addr));
}

void SM83::ld_rr_a(uint8_t opcode) {
    uint16_t addr = (opcode & 0x10) ? getDE() : getBC();
    memory.write8(addr, A);
}

void SM83::ld_a_nn(uint8_t opcode) {
    uint16_t addr = memory.read16(PC+1);
    A = memory.read8(addr);

}

void SM83::ld_nn_a(uint8_t opcode) {
    uint16_t addr = memory.read16(PC+1);
    memory.write8(addr, A);

}


//TODO : More LD functions here

void SM83::inc_r(uint8_t opcode) {
    uint8_t r = (opcode >> 3) & 0x07;

    if (r == 6) {
        //for HL
        uint8_t oldHL = memory.read8(getHL());
        memory.write8(getHL(), oldHL+1);

        setFlags(
            F,
            ((oldHL + 1)) == 0,                  // Z
            0,                       // N = 0
            HALF_CARRY_ADD8(oldHL, 1),   // H
            (F & FLAG_C) != 0              // C unchanged
        );

    }
    else {
        uint8_t oldReg = getR(r);
        setR(r, getR(r)+1);

        setFlags(
            F,
            getR(r) == 0,                  // Z
            0,                       // N = 0
            HALF_CARRY_ADD8(oldReg, 1),   // H
            (F & FLAG_C) != 0              // C unchanged
        );
    }
}

void SM83::inc_rr(uint8_t opcode) {
    uint8_t rr = (opcode >> 4) & 0x03;

    if (rr == 3) {
        //SP
        SP++;

    } else {
        setRR(rr, getRR(rr)+1);
    }
}

void SM83::dec_r(uint8_t opcode) {
    uint8_t r = (opcode >> 3) & 0x07;

    if (r == 6) {
        //for HL
        uint8_t oldHL = memory.read8(getHL());
        memory.write8(getHL(), oldHL - 1);

        setFlags(
            F,
            ((oldHL - 1)) == 0,                 // Z
            1,                      // N = 1
            HALF_CARRY_SUB8(oldHL, 1),  // H (borrow from bit 4)
            (F & FLAG_C) != 0             // C unchanged
        );

    }
    else {
        uint8_t oldReg = getR(r);
        setR(r, getR(r) - 1);

        setFlags(
            F,
            oldReg - 1 == 0,                 // Z
            1,                      // N = 1
            HALF_CARRY_SUB8(oldReg, 1),  // H (borrow from bit 4)
            (F & FLAG_C) != 0             // C unchanged
        );
    }
}

void SM83::dec_rr(uint8_t opcode) {
    uint8_t rr = (opcode >> 4) & 3;

    if (rr == 3) {
        //SP
        SP--;

    } else {
        setRR(rr, getRR(rr)-1);
    }
}

void SM83::add_a_r(uint8_t opcode) {
    const uint8_t r = opcode & 7; //last 3 bits

    if (r == 6) {
        //(HL)
        const uint8_t HL = memory.read8(getHL());
        uint8_t oldA = A;
        A += HL;
        setFlags(
            F,
            (A == 0),
            0,
            HALF_CARRY_ADD8(oldA, HL),
            CARRY_ADD8(oldA, HL)
        );

    } else {
        const uint8_t value = getR(r);
        uint8_t oldA = A;
        A += value;
        setFlags(
            F,
            (A == 0),
            0,
            HALF_CARRY_ADD8(oldA, value),
            CARRY_ADD8(oldA, value)
        );
    }
}

void SM83::add_a_n(uint8_t opcode) {
    uint8_t value = memory.read8(PC + 1);
    uint8_t oldA = A;
    A += value;

    setFlags(
        F,
        (A == 0),
        0,
        HALF_CARRY_ADD8(oldA, value),
        CARRY_ADD8(oldA, value)
    );
}

void SM83::add_hl_rr(uint8_t opcode) {
    const uint8_t rr = (opcode >> 4) & 3;
    const uint16_t oldHL = memory.read8(getHL());
    memory.write16(getHL(), oldHL + getRR(rr));
    setFlags(
        F,
        0,
        0,
        HALF_CARRY_ADD16(oldHL, getRR(rr)),
        CARRY_ADD16(oldHL, getRR(rr))
    );
}

void SM83::add_sp_n(uint8_t opcode) {
    const uint8_t n = memory.read8(PC + 1);
    const uint16_t oldSP = SP;
    SP += static_cast<int8_t>(n);

    uint8_t low = static_cast<int8_t>(oldSP & 0xFF);
    uint8_t resultLow = low + static_cast<int8_t>(n);

    setFlags(
        F,
        0,
        0,
        HALF_CARRY_ADD16(oldSP, n),
        CARRY_ADD16(oldSP, n)
    );
}

void SM83::adc_r(uint8_t opcode) {
    uint8_t r = opcode  & 0x07;
    uint8_t oldA = A;

    uint8_t operand = r == 6 ? memory.read8(getHL()) : getR(r);
    uint8_t result = A + operand + (F & FLAG_C ? 1 : 0);

    A = result;

    setFlags(
        F,
        result == 0,
        0,
        ( ((oldA & 0xF) + (operand & 0xF) + ((F & FLAG_C ? 1 : 0))) > 0xF ),
        ( static_cast<uint16_t>(oldA) + static_cast<uint16_t>(operand) + static_cast<uint16_t>(F & FLAG_C ? 1 : 0) ) > 0xFF  // Carry flag
    );
}

void SM83::adc_n(uint8_t opcode) {
    uint8_t oldA = A;

    uint8_t operand = memory.read8(PC + 1);
    uint8_t result = A + operand + (F & FLAG_C ? 1 : 0);

    A = result;

    setFlags(
        F,
        result == 0,
        0,
        ( ((oldA & 0xF) + (operand & 0xF) + ((F & FLAG_C ? 1 : 0))) > 0xF ),
        ( static_cast<uint16_t>(oldA) + static_cast<uint16_t>(operand) + static_cast<uint16_t>(F & FLAG_C ? 1 : 0) ) > 0xFF  // Carry flag
    );
}


void SM83::sub_a_r(uint8_t opcode) {
    const uint8_t r = opcode & 3; //last 3 bits

    if (r == 6) {
        //(HL)
        const uint8_t HL = memory.read8(getHL());
        uint8_t oldA = A;
        A -= HL;
        setFlags(
            F,
            (A == 0),
            0,
            HALF_CARRY_ADD8(oldA, HL),
            CARRY_ADD8(oldA, HL)
        );

    } else {
        const uint8_t value = getR(r);
        uint8_t oldA = A;
        A -= value;
        setFlags(
            F,
            (A == 0),
            0,
            HALF_CARRY_ADD8(oldA, value),
            CARRY_ADD8(oldA, value)
        );
    }
}

void SM83::sub_a_n(uint8_t opcode) {
    uint8_t value = memory.read8(PC + 1);
    uint8_t oldA = A;
    A -= value;

    setFlags(
        F,
        (A == 0),
        0,
        HALF_CARRY_ADD8(oldA, value),
        CARRY_ADD8(oldA, value)
    );
}

void SM83::sub_hl_rr(uint8_t opcode) {
    const uint8_t rr = (opcode >> 4) & 3;
    const uint16_t oldHL = memory.read8(getHL());
    memory.write16(getHL(), oldHL + getRR(rr));
    setFlags(
        F,
        0,
        0,
        HALF_CARRY_ADD16(oldHL, getRR(rr)),
        CARRY_ADD16(oldHL, getRR(rr))
    );
}

void SM83::sub_sp_n(uint8_t opcode) {
    const uint8_t n = memory.read8(PC + 1);
    const uint16_t oldSP = SP;
    SP += static_cast<int8_t>(n);

    uint8_t low = static_cast<int8_t>(oldSP & 0xFF);
    uint8_t resultLow = low + static_cast<int8_t>(n);

    setFlags(
        F,
        0,
        0,
        HALF_CARRY_ADD16(oldSP, n),
        CARRY_ADD16(oldSP, n)
    );
}

void SM83::sbc_r(uint8_t opcode) {
    uint8_t r = opcode  & 0x07;
    uint8_t oldA = A;
    uint8_t operand = r == 6 ? memory.read8(getHL()) : getR(r);
    uint8_t result = A - operand - (F & FLAG_C ? 1 : 0);

    A = result;

    setFlags(
        F,
        result == 0,
        1,
        (oldA & 0xF) < (((operand & 0xF) + ((F & FLAG_C ? 1 : 0)))),
        (static_cast<uint16_t>(oldA) < static_cast<uint16_t>(operand) + (F & FLAG_C ? 1 : 0))
    );
}

void SM83::sbc_n(uint8_t opcode) {
    uint8_t oldA = A;
    uint8_t operand = memory.read8(PC+1);
    uint8_t result = A - operand - (F & FLAG_C ? 1 : 0);

    A = result;

    setFlags(
        F,
        result == 0,
        1,
        (oldA & 0xF) < (((operand & 0xF) + ((F & FLAG_C ? 1 : 0)))),
        (static_cast<uint16_t>(oldA) < static_cast<uint16_t>(operand) + (F & FLAG_C ? 1 : 0))
    );
}

void SM83::rlca(uint8_t opcode) {
    const uint8_t oldA = A;
    A = (A << 1) | (oldA >> 7);
    setFlags(
        F,
        0,
        0,
        0,
        (oldA & 0x80) != 0
    );
}

void SM83::rrca(uint8_t opcode) {
    const uint8_t oldA = A;
    A = (oldA << 7) | (oldA >> 1);
    setFlags(
        F,
        0,
        0,
        0,
        (oldA & 0x01) != 0 //last but of oldA
    );
}


void SM83::stop(uint8_t opcode) {
    bool running = true;
    while (running) {
        PC -= 1;
        // TODO: keypad event here

        // if () {
        //     running = false;
        // }
    }
}

void SM83::jr(uint8_t opcode) {
    uint8_t e8 = memory.read8(PC + 1);

    if (opcode == 0x18) { //unconditional jump
        PC = PC + static_cast<int8_t>(e8); //jump
        stepPC(2, 12, 8, false);

    } else { //conditional jump
        const uint8_t condition = (opcode >> 3) & 0x03;

        switch (condition) {
            case 0: { //NZ
                if ((F & FLAG_Z) == 0) {
                    PC = PC + static_cast<int8_t>(e8); //jump
                    stepPC(2, 12, 8, true);
                } else {
                   stepPC(2, 12, 8, false);
                }
                break;
            }

            case 1: { //Z
                if ((F & FLAG_Z) != 0) {
                    PC = PC + static_cast<int8_t>(e8); //jump
                    stepPC(2, 12, 8, true);
                } else {
                    stepPC(2, 12, 8, false);
                }
                break;
            }

            case 2: { //NC
                if ((F & FLAG_C) == 0) {
                    PC = PC + static_cast<int8_t>(e8); //jump
                    stepPC(2, 12, 8, true);
                } else {
                    stepPC(2, 12, 8, false);
                }
                break;
            }

            case 3: { //C
                if ((F & FLAG_C) != 0) {
                    PC = PC + static_cast<int8_t>(e8); //jump
                    stepPC(2, 12, 8, true);
                } else {
                    stepPC(2, 12, 8, false);
                }
                break;
            }
            default:
                throw std::runtime_error("Invalid opcode");
        }
    }
}

void SM83::rla(uint8_t opcode) {
    uint8_t new_carry = (A >> 7) & 0x1;
    A = (A << 1) | ((F & FLAG_C) >> 4);

    setFlags(
        F,
        0,
        0,
        0,
        new_carry
    );
}

void SM83::rra(uint8_t opcode) {
    uint8_t new_carry = A & 0x1;
    A = (A >> 1) | ((F & FLAG_C) << 3);

    setFlags(
        F,
        0,
        0,
        0,
        new_carry
    );
}

void SM83::cpl(uint8_t opcode) {
    A = ~A;
    setFlags(
        F,
        (F & FLAG_Z) != 0,
        1,
        1,
        (F & FLAG_C) != 0
    );
}

void SM83::scf(uint8_t opcode) {
    setFlags(
        F,
        (F & FLAG_Z) != 0,
        0,
        0,
        1
    );
}

void SM83::ccf(uint8_t opcode) {
    setFlags(
        F,
        (F & FLAG_Z) != 0,
        0,
        0,
        (F & FLAG_C) == 0
    );
}

void SM83::_and_r(uint8_t opcode) {
    uint8_t r = opcode & 0x7; //last 3 bits
    uint8_t result = 0;

    if (r == 6) {
        r = memory.read8(getHL());
        result = A & r;
        A &= r;

    } else {
        result = A & getR(r);
        A &= getR(r);
    }

    setFlags(
        F,
        result == 0,
        0,
        1,
        0
    );
}

void SM83::_and_d8(uint8_t opcode) {
    const uint8_t value = memory.read8(PC + 1);

    const uint8_t result = A & value;
    A &= value;
    setFlags(
        F,
        result == 0,
        0,
        1,
        0
    );
}

void SM83::_xor_r(uint8_t opcode) {
    uint8_t r = opcode & 0x7; //last 3 bits
    uint8_t result = 0;

    if (r == 6) {
        r = memory.read8(getHL());
        result = A ^ r;
        A ^= r;

    } else {
        result = A ^ getR(r);
        A ^= getR(r);
    }

    setFlags(
        F,
        result == 0,
        0,
        0,
        0
    );
}

void SM83::_xor_d8(uint8_t opcode) {
    const uint8_t value = memory.read8(PC + 1);

    const uint8_t result = A ^ value;
    A ^= value;
    setFlags(
        F,
        result == 0,
        0,
        0,
        0
    );
}

void SM83::cp_r(uint8_t opcode) {
    uint8_t r = (opcode & 0x07);
    uint8_t result, value;
    if (r ==6) {
        value = memory.read8(getHL());
        result = A - value;
    } else {
        value = getR(r);
        result = A - value;
    }

    setFlags(
        F,
        result == 0,
        1,
        HALF_CARRY_SUB8(A, value),
        CARRY_SUB8(A, value)
    );
}

void SM83::cp_d8(uint8_t opcode) {
    uint8_t value = memory.read8(PC + 1);
    uint8_t result = A - value;

    setFlags(
        F,
        result == 0,
        1,
        HALF_CARRY_SUB8(A, value),
        CARRY_SUB8(A, value)
    );
}

void SM83::or_r(uint8_t opcode) {
    uint8_t r = opcode & 0x7; //last 3 bits
    uint8_t result = 0;

    if (r == 6) {
        r = memory.read8(getHL());
        result = A | r;
        A |= r;

    } else {
        result = A | getR(r);
        A |= getR(r);
    }

    setFlags(
        F,
        result == 0,
        0,
        0,
        0
    );
}

void SM83::or_d8(uint8_t opcode) {
    const uint8_t value = memory.read8(PC + 1);

    const uint8_t result = A | value;
    A |= value;
    setFlags(
        F,
        result == 0,
        0,
        0,
        0
    );
}

void SM83::ret_cc(uint8_t opcode) {
    if (opcode == 0xC9) {
        PC = memory.read16(SP);
        SP += 2;
        cycles += 16;
    } else {
        // Conditional RET
        bool conditionMet = false;

        switch ((opcode >> 3) & 0x03) { // top 2 bits of lower nibble
            case 0: // RET NZ
                conditionMet = (F & FLAG_Z) == 0;
                break;
            case 1: // RET Z
                conditionMet = (F & FLAG_Z) != 0;
                break;
            case 2: // RET NC
                conditionMet = (F & FLAG_C) == 0;
                break;
            case 3: // RET C
                conditionMet = (F & FLAG_C) != 0;
                break;
        }

        if (conditionMet) {
            // Pop PC from stack
            PC = memory.read16(SP);
            SP += 2;
            cycles += 20;
        } else {
            cycles += 8;
            // Move PC past instruction (1 byte)
            PC += 1;
        }
    }
}

void SM83::reti(uint8_t opcode) {
    uint16_t value = memory.read16(SP);
    SP += 2;
    PC = value;
    //IME = 1;
}

void SM83::jp_nn(uint8_t opcode) {
    uint8_t nn_lsb = memory.read8(PC + 1);
    uint8_t nn_msb = memory.read8(PC + 2);
    uint16_t nn = (static_cast<uint16_t>(nn_msb) << 8) | nn_lsb;
    if (opcode == 0xC3) {
        PC = nn;
        stepPC(3, 16, 0, true);
    } else if (opcode == 0xE9) {
        PC = getHL();
        cycles += 4;
    } else {
        const uint8_t condition = (opcode >> 3) & 0x03;
        switch (condition) {
            case 0: { //NZ
                bool taken = (F & FLAG_Z) == 0;
                if (taken) PC = nn;
                stepPC(2, 12, 4, taken);
                break;
            }
            case 1: { //Z
                bool taken = (F & FLAG_Z) != 0;
                if (taken) PC = nn;
                stepPC(2, 12, 4, taken);
                break;
            }
            case 2: { //NC
                bool taken = (F & FLAG_C) == 0;
                if (taken) PC = nn;
                stepPC(2, 12, 4, taken);
                break;
            }
            case 3: { //C
                bool taken = (F & FLAG_C) != 0;
                if (taken) PC = nn;
                stepPC(2, 12, 4, taken);
                break;
            }
            default:
                throw std::runtime_error("Invalid opcode");
        }
    }
}

void SM83::call_nn(uint8_t opcode) {
    uint8_t nn_lsb = memory.read8(PC + 1);
    uint8_t nn_msb = memory.read8(PC + 2);
    uint16_t nn = (static_cast<uint16_t>(nn_msb) << 8) | nn_lsb;
    if (opcode == 0xCD) {
        SP -= 2;
        memory.write16(SP, PC + 3);
        PC = nn;
        stepPC(3, 24, 0, true);
    } else {
        const uint8_t condition = (opcode >> 3) & 0x03;
        switch (condition) {
            case 0: { //NZ
                bool taken = (F & FLAG_Z) == 0;
                if (taken) { SP -= 2; memory.write16(SP, PC + 3); PC = nn; }
                stepPC(3, 12, 12, taken);
                break;
            }
            case 1: { //Z
                bool taken = (F & FLAG_Z) != 0;
                if (taken) { SP -= 2; memory.write16(SP, PC + 3); PC = nn; }
                stepPC(3, 12, 12, taken);
                break;
            }
            case 2: { //NC
                bool taken = (F & FLAG_C) == 0;
                if (taken) { SP -= 2; memory.write16(SP, PC + 3); PC = nn; }
                stepPC(3, 12, 12, taken);
                break;
            }
            case 3: { //C
                bool taken = (F & FLAG_C) != 0;
                if (taken) { SP -= 2; memory.write16(SP, PC + 3); PC = nn; }
                stepPC(3, 12, 12, taken);
                break;
            }
            default:
                throw std::runtime_error("Invalid opcode");
        }
    }
}

void SM83::push_nn(uint8_t opcode) {
    uint8_t condition = (opcode >> 4) & 0x03;
    SP -= 2;
    if (opcode == 0xF5) {
        memory.write16(SP, (static_cast<uint16_t>(A) << 8) | F);
    } else {
        uint16_t rr = getRR(condition);
        memory.write16(SP, rr);
    }
}

void SM83::rst_n(uint8_t opcode) {
    uint8_t n = (opcode >> 3) & 0x07;
    SP--;
    memory.write8(SP, static_cast<uint8_t>(mask8(PC))); //write lsb
    SP--;
    memory.write8(SP, static_cast<uint8_t>(PC >> 8)); //write msb
    PC = static_cast<uint16_t>(n * 0x08); //PC jumps to specififed memory location, return address in stack memory
}

void SM83::pop_rr(uint8_t opcode) {
    uint8_t rr = (opcode >> 4) & 0x03;
    uint16_t value = memory.read16(SP);
    SP += 2;

    if (rr == 3) {
        A = value >> 8;
        F = value & 0xF0;
    } else {
        setRR(rr, value);
    }
}

void SM83::ldh_a_n(uint8_t opcode) {
    if (opcode == 0xE0) {

    } else if (opcode == 0x32) {

    } else if (opcode == 0xF2) {

    }
}

void SM83::ldh_n_a(uint8_t opcode) {

}



//SECTION: Unprefixed
void SM83::bit_r(uint8_t opcode) {
    uint8_t b = opcode & 0x38; //01bb brrr
    uint8_t r = (opcode >> 3) & 0x07;
    if ( r == 6 ) {
        r = memory.read8(getHL());
    } else {
        r = getR(r);
    }

    uint8_t bit_masker = 1 << b;
    uint8_t bit_test = r & bit_masker;

    setFlags(
        F,
        bit_test == 0,
        0,
        1,
        (F & FLAG_C) != 0
    );
}

void SM83::reset_r(uint8_t opcode) {
    uint8_t b = (opcode >> 3) & 0x07; //0 bb brrr
    uint8_t r = opcode & 0x07;
    uint8_t reg;
    uint8_t bit_masker = ~(1 << b);

    if (r == 6) {
        reg = memory.read8(getHL());
        memory.write8(getHL(), reg & bit_masker);
    } else {
        reg = getR(r);
        setR(r, reg & bit_masker);
    }

}

void SM83::set_r(uint8_t opcode) {
    uint8_t b = (opcode >> 3) & 0x07; //0 bb brrr
    uint8_t r = opcode & 0x07;
    uint8_t reg;
    uint8_t bit_masker = (1 << b);

    if (r == 6) {
        reg = memory.read8(getHL());
        memory.write8(getHL(), reg | bit_masker);
    } else {
        reg = getR(r);
        setR(r, reg | bit_masker);
    }
}

void SM83::swap_r(uint8_t opcode) {
    uint8_t r = opcode & 0x07;
    uint8_t reg, result;
    if (r == 6) {
        reg = memory.read8(getHL());
        uint8_t lo = reg & 0xF;
        uint8_t hi = (reg >> 4) & 0xF;
        result = (lo << 4) | hi;
        memory.write8(getHL(), result);
    } else {
        reg = getR(r);
        uint8_t lo = reg & 0xF;
        uint8_t hi = (reg >> 4) & 0xF;
        result = (lo << 4) | hi;
        setR(r, result);
    }

    setFlags(
        F,
        (result == 0),
        0,
        0,
        0
    );
}

void SM83::srl_r(uint8_t opcode) {
    uint8_t r = opcode & 0x07;
    uint8_t reg, oldReg;
    if (r == 6) {
        oldReg = memory.read8(getHL());
        reg = oldReg >> 1;
        memory.write8(getHL(), reg);
    } else {
        oldReg = getR(r);
        reg = oldReg >> 1;
        setR(r, reg);
    }

    uint8_t carry = oldReg & 0x01;
    setFlags(
        F,
        reg == 0,
        0,
        0,
        carry
    );
}

void SM83::sla_r(uint8_t opcode) {
    uint8_t r = opcode & 0x07;
    uint8_t reg, oldReg;
    if (r == 6) {
        oldReg = memory.read8(getHL());
        reg = oldReg << 1;
        memory.write8(getHL(), reg);
    } else {
        oldReg = getR(r);
        reg = oldReg << 1;
        setR(r, reg);
    }

    uint8_t carry = (oldReg >> 7) & 0x01;
    setFlags(
        F,
        reg == 0,
        0,
        0,
        carry
    );
}

void SM83::sra_r(uint8_t opcode) {
    uint8_t r = opcode & 0x07;
    uint8_t reg, oldReg;
    if (r == 6) {
        oldReg = memory.read8(getHL());
        reg = (oldReg & 0x80) | (oldReg >> 1);
        memory.write8(getHL(), reg);
    } else {
        oldReg = getR(r);
        reg = (oldReg & 0x80) | (oldReg >> 1);
        setR(r, reg);
    }

    uint8_t carry = oldReg & 0x01;
    setFlags(
        F,
        reg == 0,
        0,
        0,
        carry
    );
}

void SM83::rr_r(uint8_t opcode) {
    uint8_t r = opcode & 0x07;
    uint8_t reg, oldReg;
    if (r == 6) {
        oldReg = memory.read8(getHL());
        reg = ((F & FLAG_C) << 3) | (oldReg >> 1);
        memory.write8(getHL(), reg);
    } else {
        oldReg = getR(r);
        reg = ((F & FLAG_C) << 3) | (oldReg >> 1);
        setR(r, reg);
    }

    uint8_t carry = oldReg & 0x01;
    setFlags(
        F,
        reg == 0,
        0,
        0,
        carry
    );
}

void SM83::rl_r(uint8_t opcode) {
    uint8_t r = opcode & 0x07;
    uint8_t reg, oldReg;
    if (r == 6) {
        oldReg = memory.read8(getHL());
        reg = (oldReg << 1) | ((F & FLAG_C) >> 4);
        memory.write8(getHL(), reg);
    } else {
        oldReg = getR(r);
        reg = (oldReg << 1) | ((F & FLAG_C) >> 4);
        setR(r, reg);
    }

    uint8_t carry = oldReg & 0x80;
    setFlags(
        F,
        reg == 0,
        0,
        0,
        carry
    );
}

void SM83::rrc_r(uint8_t opcode) {
    uint8_t r = opcode & 0x07;
    uint8_t reg, oldReg, carry;
    if (r == 6) {
        oldReg = memory.read8(getHL());
        carry = oldReg & 0x01;

        reg = (carry << 7) | (oldReg >> 1);
        memory.write8(getHL(), reg);
    } else {
        oldReg = getR(r);
        carry = oldReg & 0x01;

        reg = (carry << 7) | (oldReg >> 1);
        setR(r, reg);
    }

    setFlags(
        F,
        reg == 0,
        0,
        0,
        carry
    );
}

void SM83::rlc_r(uint8_t opcode) {
    uint8_t r = opcode & 0x07;
    uint8_t reg, oldReg, carry;
    if (r == 6) {
        oldReg = memory.read8(getHL());
        carry = oldReg & 0x80;

        reg = (oldReg << 1) | (carry >> 7);
        memory.write8(getHL(), reg);
    } else {
        oldReg = getR(r);
        carry = oldReg & 0x80;

        reg = (oldReg << 1) | (carry >> 7);
        setR(r, reg);
    }

    setFlags(
        F,
        reg == 0,
        0,
        0,
        carry != 0
    );
}

void SM83::execute() {
    //FETCH
    uint8_t opcode = memory.read8(PC);

    //DECODE
    uint8_t prefix = memory.read8(PC - 1);
    const Instruction& instr = (prefix == 0xCB) ? prefixed_instruction_table[opcode] :
                                                  unprefixed_instruction_table[opcode];

    //EXECUTE
    if (instr.execute) {
        (this->*instr.execute)(opcode);
    }

    if (!instr.wrapper_overrides_pc) {
        stepPC(instr.bytes, instr.cycles, instr.alt_cycles, false);
    }
}
