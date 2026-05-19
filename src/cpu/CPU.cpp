#include<bits/stdc++.h>
#include "cpu/CPU.h"
#include "core/MMU.h"
#include "cpu/ALU.h"
#include "cpu/Instructions.h"

SM83::SM83(MEMORY& mem) : A(0x01), F(0xB0), B(0x00), C(0x13),
                            D(0x00), E(0xD8), H(0x01), L(0x4D), SP(0xFFFE),
                            PC(0x100), cycles(0), memory(mem) {};

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
        case 0: return (static_cast<uint16_t>(B) << 8) | C;
        case 1: return (static_cast<uint16_t>(D) << 8) | E;
        case 2: return (static_cast<uint16_t>(H) << 8) | L;
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

void SM83::ld_r_r(ExecState& exec) {
    uint8_t dst = (exec.opcode >> 3) & 0x07;
    uint8_t src = exec.opcode & 0x07;

    if (src == 6 && dst == 6) return; //HALT
    if (src == 6 || dst == 6) {
        // 2 M-cycle path
        switch(exec.step) {
            case 0:
                exec.tmp = (src == 6) ? memory.read8(getHL()) : getR(src);
                break;
            case 1:
                if (dst == 6) memory.write8(getHL(), exec.tmp);
                else setR(dst, exec.tmp);
                break;
        }
    } else {
        // 1 M-cycle path
        setR(dst, getR(src));
    }
}

void SM83::ld_r_d8(ExecState& exec) {
    uint8_t r = (exec.opcode >> 3) & 0x07; //dst
    // 2 M-cycles 
        switch (exec.step) {
            case 0:
                exec.tmp = memory.read8(PC); //src
                PC++;
                break;
            case 1:
                // HL path
                if (r == 6) memory.write8(getHL(), exec.tmp);
                // Non-HL path
                else setR(r, exec.tmp);
                break;
        }
}

void SM83::ld_rr_d16(ExecState& exec) {
    switch (exec.step) {
        case 0:
            exec.tmp = memory.read8(PC);
            PC++;
            break;
        case 1:
            exec.tmp16 = exec.tmp | (memory.read8(PC) << 8);
            PC++;
            break;
        case 2:
            setRR((exec.opcode >> 4) & 0x03, exec.tmp16);
            break;
    }
}

void SM83::ld_a_rr(ExecState& exec) {
    switch (exec.step) {
        case 0:
            if (exec.opcode == 0x0A) {
                exec.tmp = memory.read8(getBC());
            } else if (exec.opcode == 0x1A) {
                exec.tmp = memory.read8(getDE());
            } else if (exec.opcode == 0x2A) {
                exec.tmp = memory.read8(getHL());
                setRR(2, getHL() + 1);
            } else if (exec.opcode == 0x3A) {
                exec.tmp = memory.read8(getHL());
                setRR(2, getHL() - 1);
            }
            break;
        case 1:
            A = exec.tmp;
            break;
    }
}

void SM83::ld_rr_a(ExecState& exec) {
    switch(exec.step) {
    case 0:
        if (exec.opcode == 0x02) {
            memory.write8(getBC(), A);
        } else if (exec.opcode == 0x12) {
            memory.write8(getDE(), A);
        } else if (exec.opcode == 0x22) {
            memory.write8(getHL(), A);
            setRR(2, getHL() + 1);
        } else if (exec.opcode == 0x32) {
            memory.write8(getHL(), A);
            setRR(2, getHL() - 1);
        }
        break;
    case 1:
        break;
    }
}

void SM83::ld_a_nn(ExecState& exec) {
    switch (exec.step) {
        case 0:
            exec.tmp = memory.read8(PC);
            PC++;
            break;
        case 1:
            exec.tmp16 = exec.tmp | (memory.read8(PC) << 8);
            PC++;
            break;
        case 2:
            A = memory.read8(exec.tmp16);
            break;
    }
}

void SM83::ld_nn_a(ExecState& exec) {
    switch (exec.step) {
        case 0:
            exec.tmp = memory.read8(PC);
            PC++;
            break;
        case 1:
            exec.tmp16 = exec.tmp | (memory.read8(PC) << 8);
            PC++;
            break;
        case 2:
            memory.write8(exec.tmp16, A);
            break;
    }
}

void SM83::ld_hl_sp_n(ExecState& exec) {
    switch (exec.step) {
        case 0:
            exec.tmp = memory.read8(PC);  // read operand n
            PC++;
            break;
        case 1: {
            uint16_t oldSP = SP;
            int8_t n = static_cast<int8_t>(exec.tmp);
            uint16_t result = SP + n;
            setRR(2, result);
            setFlags(F, 0, 0,
                HALF_CARRY_ADD8(oldSP & 0xFF, static_cast<uint8_t>(n)),
                CARRY_ADD8(oldSP & 0xFF, static_cast<uint8_t>(n))
            );
            break;
        }
    }
}

void SM83::ld_sp_hl(ExecState& exec) {
    SP = getHL();
    // no flags affected
}

//WARNING:
void SM83::inc_r(ExecState& exec) {
    uint8_t r = (exec.opcode >> 3) & 0x07;

    if (r == 6) {
        //for HL
        switch (exec.step) {
            case 0:
                exec.tmp = memory.read8(getHL());
                break;

            case 1:
                memory.write8(getHL(), exec.tmp + 1);
                setFlags(
                        F,
                        ((exec.tmp + 1)) == 0,                  // Z
                        0,                       // N = 0
                        HALF_CARRY_ADD8(exec.tmp, 1),   // H
                        (F & FLAG_C) != 0              // C unchanged
                );
                break;
        }
    } else {
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

void SM83::inc_rr(ExecState& exec) {
    uint8_t rr = (exec.opcode >> 4) & 0x03;

    if (rr == 3) {
        //SP
        SP++;
    } else {
        setRR(rr, getRR(rr)+1);
    }
}

void SM83::dec_r(ExecState& exec) {
    uint8_t r = (exec.opcode >> 3) & 0x07;

    if (r == 6) {
        //for HL
        switch (exec.step) {
            case 0:
                exec.tmp = memory.read8(getHL());
                break;
            case 1:
                memory.write8(getHL(), exec.tmp - 1);
                setFlags(
                        F,
                        ((exec.tmp - 1)) == 0,                 // Z
                        1,                      // N = 1
                        HALF_CARRY_SUB8(exec.tmp, 1),  // H (borrow from bit 4)
                        (F & FLAG_C) != 0             // C unchanged
                );
                break;
        }
    } else {
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

void SM83::dec_rr(ExecState& exec) {
    uint8_t rr = (exec.opcode >> 4) & 3;

    if (rr == 3) {
        //SP
        SP--;

    } else {
        setRR(rr, getRR(rr)-1);
    }
}

void SM83::add_a_r(ExecState& exec) {
    const uint8_t r = exec.opcode & 7; //last 3 bits

    if (r == 6) {
        //(HL)
        switch (exec.step) {
            case 0:
                exec.tmp = memory.read8(getHL());
                break;
            case 1: {
                uint8_t oldA = A;
                A += exec.tmp;
                setFlags(
                        F,
                        (A == 0),
                        0,
                        HALF_CARRY_ADD8(oldA, exec.tmp),
                        CARRY_ADD8(oldA, exec.tmp)
                );
                break;
            }
        }
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

void SM83::add_a_n(ExecState& exec) {
    switch (exec.step) {
        case 0:
            exec.tmp = memory.read8(PC);
            PC++;
            break;
        case 1: {
            uint8_t oldA = A;
            A += exec.tmp;
            setFlags(
                    F,
                    (A == 0),
                    0,
                    HALF_CARRY_ADD8(oldA, exec.tmp),
                    CARRY_ADD8(oldA, exec.tmp)
            );
            break;
        }
    }
}

void SM83::add_hl_rr(ExecState& exec) {
    const uint8_t rr = (exec.opcode >> 4) & 3;
    const uint16_t oldHL = (getHL());
    setRR(2, oldHL + getRR(rr));
    setFlags(
        F,
        0,
        0,
        HALF_CARRY_ADD16(oldHL, getRR(rr)),
        CARRY_ADD16(oldHL, getRR(rr))
    );
}

void SM83::add_sp_n(ExecState& exec) {
    switch (exec.step) {
        case 0:
            exec.tmp = memory.read8(PC);
            PC++;
            break;
        case 1:
            setFlags(
                    F,
                    0,
                    0,
                    HALF_CARRY_ADD8(SP & 0xFF, static_cast<int8_t>(exec.tmp)),
                    CARRY_ADD8(SP & 0xFF, static_cast<int8_t>(exec.tmp))
            );
            break;
        case 2:
            SP += static_cast<int8_t>(exec.tmp);
            break;
        case 3:
            break;
    }
}

void SM83::adc_r(ExecState& exec) {
    uint8_t r = exec.opcode  & 0x07;

    if (r == 6) {
        switch (exec.step) {
            case 0: {
                uint8_t oldA = A;
                exec.tmp = memory.read8(getHL());
                exec.tmp16 = A;
                break;
            }
            case 1: {
            uint8_t result = A + exec.tmp + (F & FLAG_C ? 1 : 0);
            A = result;
            uint8_t oldA = exec.tmp16 & 0xFF;
            setFlags(
                        F,
                        result == 0,
                        0,
                        ( ((oldA & 0xF) + (exec.tmp & 0xF) + ((F & FLAG_C ? 1 : 0))) > 0xF ),
                        (static_cast<uint16_t>(oldA) + static_cast<uint16_t>(exec.tmp) + static_cast<uint16_t>(F & FLAG_C ? 1 : 0) ) > 0xFF  // Carry flag
            );
             break;
            }

        }
    } else {
        uint8_t oldA = A;
        uint8_t result = A + getR(r) + (F & FLAG_C ? 1 : 0);
        A = result;
        setFlags(
                F,
                result == 0,
                0,
                ( ((oldA & 0xF) + (getR(r) & 0xF) + ((F & FLAG_C ? 1 : 0))) > 0xF ),
                ( static_cast<uint16_t>(oldA) + static_cast<uint16_t>(getR(r)) + static_cast<uint16_t>(F & FLAG_C ? 1 : 0) ) > 0xFF  // Carry flag
                );
    }
}

void SM83::adc_n(ExecState& exec) {
    switch (exec.step) {
        case 0:
            exec.tmp = memory.read8(PC);
            exec.tmp16 = A;
            PC++;
            break;
        case 1:
            uint8_t result = A + exec.tmp + (F & FLAG_C ? 1 : 0);
            A = result;
            uint8_t oldA = exec.tmp16 & 0xFF;
            setFlags(
                    F,
                    result == 0,
                    0,
                    ( ((oldA & 0xF) + (exec.tmp & 0xF) + ((F & FLAG_C ? 1 : 0))) > 0xF ),
                    ( static_cast<uint16_t>(oldA) + static_cast<uint16_t>(exec.tmp) + static_cast<uint16_t>(F & FLAG_C ? 1 : 0) ) > 0xFF  // Carry flag
            );
    }
}


void SM83::sub_a_r(ExecState& exec) {
    const uint8_t r = exec.opcode & 7; //last 3 bits

    if (r == 6) {
        //(HL)
        switch (exec.step) {
            case 0: {
                exec.tmp = memory.read8(getHL());
                break;
            }
            case 1:{
                uint8_t oldA = A;
                A -= exec.tmp;
                setFlags(
                        F,
                        (A == 0),
                        1,
                        HALF_CARRY_SUB8(oldA, exec.tmp),
                        CARRY_SUB8(oldA, exec.tmp)
                );
                break;
            }
        }
    } else {
        const uint8_t value = getR(r);
        uint8_t oldA = A;
        A -= value;
        setFlags(
            F,
            (A == 0),
            1,
            HALF_CARRY_SUB8(oldA, value),
            CARRY_SUB8(oldA, value)
        );
    }
}

void SM83::sub_a_n(ExecState& exec) {
    switch (exec.step) {
        case 0:
            exec.tmp = memory.read8(PC);
            PC++;
            break;
        case 1:
            uint8_t oldA = A;
            A -= exec.tmp;
            setFlags(
                    F,
                    (A == 0),
                    0,
                    HALF_CARRY_SUB8(oldA, exec.tmp),
                    CARRY_SUB8(oldA, exec.tmp)
            );
    }
}
/*
void SM83::sub_hl_rr(ExecState& exec) {
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

void SM83::sub_sp_n(ExecState& exec) {
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
*/
void SM83::sbc_r(ExecState& exec) {
    uint8_t r = exec.opcode  & 0x07;

    if (r == 6) {
        switch (exec.step) {
            case 0:
                exec.tmp = memory.read8(getHL());
                break;
            case 1: {
                uint8_t oldA = A;
                uint8_t result = A - exec.tmp - (F & FLAG_C ? 1 : 0);
                A = result;
                setFlags(
                        F,
                        result == 0,
                        1,
                        (oldA & 0xF) < (((exec.tmp & 0xF) + ((F & FLAG_C ? 1 : 0)))),
                        (static_cast<uint16_t>(oldA) < static_cast<uint16_t>(exec.tmp) + (F & FLAG_C ? 1 : 0))
                );
                break;
            }
        }
    } else {
        uint8_t oldA = A;
        uint8_t result = A - getR(r) - (F & FLAG_C ? 1 : 0);
        A = result;
        setFlags(
                F,
                result == 0,
                1,
                (oldA & 0xF) < (((getR(r) & 0xF) + ((F & FLAG_C ? 1 : 0)))),
                (static_cast<uint16_t>(oldA) < static_cast<uint16_t>(getR(r)) + (F & FLAG_C ? 1 : 0))
        );
    }
}

void SM83::sbc_n(ExecState& exec) {
    switch (exec.step) {
        case 0:
            exec.tmp = memory.read8(PC);
            PC++;
            exec.tmp16 = A;
            break;
        case 1:
            uint8_t oldA = exec.tmp16 & 0xFF;
            uint8_t result = A - exec.tmp - (F & FLAG_C ? 1 : 0);
            A = result;
            setFlags(
                    F,
                    result == 0,
                    1,
                    (oldA & 0xF) < (((exec.tmp & 0xF) + ((F & FLAG_C ? 1 : 0)))),
                    (static_cast<uint16_t>(oldA) < static_cast<uint16_t>(exec.tmp) + (F & FLAG_C ? 1 : 0))
            );
    }
}

void SM83::rlca(ExecState& exec) {
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

void SM83::rrca(ExecState& exec) {
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

//TODO:
void SM83::stop(ExecState& exec) {
    bool running = true;
    while (running) {
        PC -= 1;
        // TODO: keypad event here

        // if () {
        //     running = false;
        // }
    }
}

void SM83::jr(ExecState& exec) {
    if (exec.opcode == 0x18) { //unconditional jump
        switch (exec.step) {
            case 0:
                exec.tmp = memory.read8(PC);
                PC++;
                break;
            case 1:
                PC = PC + static_cast<int8_t>(exec.tmp); //jump
                break;
        }
    } else { //unconditional jump
        switch (exec.step) {
        case 0:
            exec.tmp = memory.read8(PC);
            PC++;
            switch ((exec.opcode >> 3) & 0x03) {
                case 0: exec.condition_taken = (F & FLAG_Z) == 0; break; // NZ
                case 1: exec.condition_taken = (F & FLAG_Z) != 0; break; // Z
                case 2: exec.condition_taken = (F & FLAG_C) == 0; break; // NC
                case 3: exec.condition_taken = (F & FLAG_C) != 0; break; // C
            }
            break;
        case 1:
            if (exec.condition_taken)
                PC += static_cast<int8_t>(exec.tmp);
            break;
    }
    }
}

void SM83::rla(ExecState& exec) {
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

void SM83::rra(ExecState& exec) {
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

void SM83::cpl(ExecState& exec) {
    A = ~A;
    setFlags(
        F,
        (F & FLAG_Z) != 0,
        1,
        1,
        (F & FLAG_C) != 0
    );
}

void SM83::scf(ExecState& exec) {
    setFlags(
        F,
        (F & FLAG_Z) != 0,
        0,
        0,
        1
    );
}

void SM83::ccf(ExecState& exec) {
    setFlags(
        F,
        (F & FLAG_Z) != 0,
        0,
        0,
        (F & FLAG_C) == 0
    );
}

void SM83::_and_r(ExecState& exec) {
    uint8_t r = exec.opcode & 0x7; //last 3 bits
    uint8_t result = 0;

    if (r == 6) {
        switch (exec.step) {
            case 0:
                exec.tmp = memory.read8(getHL());
                break;
            case 1:
                result = A & exec.tmp;
                A = result;

                setFlags(
                        F,
                        result == 0,
                        0,
                        1,
                        0
                );
                break;
        }

    } else {
        result = A & getR(r);
        A = result;
        setFlags(
                    F,
                    result == 0,
                    0,
                    1,
                    0
        );
    }

}

void SM83::_and_d8(ExecState& exec) {
    switch (exec.step) {
        case 0:
            exec.tmp = memory.read8(PC);
            PC++;
            break;
        case 1:
            const uint8_t result = A & exec.tmp;
            A = result;
            setFlags(
                    F,
                    result == 0,
                    0,
                    1,
                    0
            );
            break;
    }
}

void SM83::_xor_r(ExecState& exec) {
    uint8_t r = exec.opcode & 0x7; //last 3 bits
    uint8_t result = 0;

    if (r == 6) {
        switch (exec.step) {
            case 0:
                exec.tmp = memory.read8(getHL());
                break;
            case 1:
                result = A ^ exec.tmp;
                A = result;
                setFlags(
                        F,
                        result == 0,
                        0,
                        0,
                        0
                );
                break;
        }
    } else {
        result = A ^ getR(r);
        A = result;
        setFlags(
                F,
                result == 0,
                0,
                0,
                0
        );
    }
}

void SM83::_xor_d8(ExecState& exec) {
    switch (exec.step) {
        case 0:
            exec.tmp = memory.read8(PC);
            PC++;
            break;
        case 1:
            const uint8_t result = A ^ exec.tmp;
            A = result;
            setFlags(
                    F,
                    result == 0,
                    0,
                    0,
                    0
            );
            break;
    }
}

void SM83::cp_r(ExecState& exec) {
    uint8_t r = (exec.opcode & 0x07);
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

void SM83::cp_d8(ExecState& exec) {
    uint8_t value = memory.read8(PC);
    PC++;
    uint8_t result = A - value;

    setFlags(
        F,
        result == 0,
        1,
        HALF_CARRY_SUB8(A, value),
        CARRY_SUB8(A, value)
    );
}

void SM83::or_r(ExecState& exec) {
    uint8_t r = exec.opcode & 0x7; //last 3 bits
    uint8_t result = 0;

    if (r == 6) {
        switch (exec.step) {
            case 0:
                exec.tmp = memory.read8(getHL());
                break;
            case 1:
                result = A | r;
                A = result;
                setFlags(
                        F,
                        result == 0,
                        0,
                        0,
                        0
                );
                break;
        }
    } else {
        result = A | getR(r);
        A = result;
        setFlags(
                F,
                result == 0,
                0,
                0,
                0
        );
    }
}

void SM83::or_d8(ExecState& exec) {
    switch (exec.step) {
        case 0:
            exec.tmp = memory.read8(PC);
            PC++;
            break;
        case 1:
            const uint8_t result = A | exec.tmp;
            A = result;
            setFlags(
                    F,
                    result == 0,
                    0,
                    0,
                    0
            );
            break;
    }
}

void SM83::ret_cc(ExecState& exec) {
    if (exec.opcode == 0xC9) {
        switch (exec.step) {
            case 0:
                exec.tmp = memory.read8(SP);
                SP++;
                break;
            case 1:
                exec.tmp16 = (memory.read8(SP) << 8) | (exec.tmp);
                SP++;
                break;
            case 2:
                PC = exec.tmp16;
                break;
        }
    } else {
        // Conditional RET
        switch (exec.step) {
            case 0:
                switch ((exec.opcode >> 3) & 0x03) { // top 2 bits of lower nibble
                    case 0: exec.condition_taken = (F & FLAG_Z) == 0; break; //RET NZ
                    case 1: exec.condition_taken = (F & FLAG_Z) != 0; break; //RET Z
                    case 2: exec.condition_taken = (F & FLAG_C) == 0; break; //RET NC
                    case 3: exec.condition_taken = (F & FLAG_C) != 0; break; //RET C;
                }
                break;
            case 1:
                if (!exec.condition_taken) break;
                exec.tmp = memory.read8(SP);
                SP++;
                break;
            case 2:
                if (!exec.condition_taken) break;
                exec.tmp16 = (memory.read8(SP) << 8) | exec.tmp;
                SP++;
                break;
            case 3:
                if (!exec.condition_taken) break;
                PC = exec.tmp16;
                break;
        }
    }
}

void SM83::reti(ExecState& exec) {
   switch (exec.step) {
       case 0:
           exec.tmp = memory.read8(SP);
           SP++;
           break;
        case 1:
           exec.tmp16 = (memory.read8(SP) << 8) | exec.tmp;
           SP++;
           break;
        case 2:
           PC = exec.tmp16;
           IME = 1;
   } 
}

void SM83::jp_nn(ExecState& exec) {
    if (exec.opcode == 0xC3) {
        switch (exec.step) {
            case 0:
                exec.tmp = memory.read8(PC);
                PC++;
                break;
            case 1:
                exec.tmp16 = (memory.read8(PC) << 8) | exec.tmp;
                PC++;
                break;
            case 2:
                PC = exec.tmp16;
                break;
        }
    } else if (exec.opcode == 0xE9) {
        PC = getHL();
    } else {
        switch (exec.step) {
            case 0: {
                switch ((exec.opcode >> 3) & 0x03) {
                    case 0: exec.condition_taken = (F & FLAG_Z) == 0; break; //NZ
                    case 1: exec.condition_taken = (F & FLAG_Z) != 0; break; //Z
                    case 2: exec.condition_taken = (F & FLAG_C) == 0; break; //NC
                    case 3: exec.condition_taken = (F & FLAG_C) != 0; break; //C
                    default: throw std::runtime_error("Invalid opcode");
                }
                exec.tmp = memory.read8(PC);
                PC++;
                break;
            }
            case 1:
                exec.tmp16 = (memory.read8(PC) << 8) | exec.tmp;
                PC++;
                break;
            case 2:
                if (!exec.condition_taken) break;
                PC = exec.tmp16;
                break;            
        }
    }
}

void SM83::call_nn(ExecState& exec) {
    if (exec.opcode == 0xCD) {
        switch (exec.step) {
            case 0:
                exec.tmp = memory.read8(PC);
                PC++;
                break;
            case 1:
                exec.tmp16 = (memory.read8(PC) << 8) | exec.tmp;
                PC++;
                break;
            case 2:
                SP--;
                break;
            case 3:
                memory.write8(SP, mask8(PC >> 8));
                SP--;
                break;
            case 4:
                memory.write8(SP, mask8(PC));
                PC = exec.tmp16;
                break;
        }
    } else {
        switch (exec.step) {
            case 0:
                switch ((exec.opcode >> 3) & 0x03) {
                    case 0: exec.condition_taken = (F & FLAG_Z) == 0; break; //NZ
                    case 1: exec.condition_taken = (F & FLAG_Z) != 0; break; //Z
                    case 2: exec.condition_taken = (F & FLAG_C) == 0; break; //NC
                    case 3: exec.condition_taken = (F & FLAG_C) != 0; break; //C
                    default: throw std::runtime_error("Invalid opcode");
                }
                exec.tmp = memory.read8(PC);
                PC++;
                break;
            case 1:
                if (!exec.condition_taken) break;
                SP--;
                break;
            case 2:
                if (!exec.condition_taken) break;
                exec.tmp16 = (memory.read8(PC) << 8) | exec.tmp;
                PC++;
                break;
            case 3:
                if (!exec.condition_taken) break;
                memory.write8(SP, mask8(PC >> 8));
                SP--;
                break;
            case 4:
                if (!exec.condition_taken) break;
                memory.write8(SP, mask8(PC));
                PC = exec.tmp16;
                break;
        }
    }
}

void SM83::push_nn(ExecState& exec) {
    if (exec.opcode == 0xF5) {
        switch (exec.step) {
            case 0:
                SP--;
                break;
            case 1:
                memory.write8(SP, A);
                SP--;
                break;
            case 2:
                memory.write8(SP, F & 0xF0);
                break;
         }
    } else {
        switch (exec.step) {
            case 0:
                exec.tmp16 = getRR((exec.opcode >> 4) & 0x03);
                SP--;
                break;
            case 1:
                memory.write8(SP, ((exec.tmp16 >> 8) & 0xFF));
                SP--;
                break;
            case 2:
                memory.write8(SP, exec.tmp16 & 0xFF);
                break;
         }
    }
}

void SM83::rst_n(ExecState& exec) {
    uint8_t n = (exec.opcode >> 3) & 0x07;
    switch (exec.step) {
        case 0:
            SP--;
            break;
        case 1:
            memory.write8(SP, static_cast<uint8_t>(PC >> 8)); //write msb
            SP--;
            break;
        case 2:
            memory.write8(SP, static_cast<uint8_t>(mask8(PC))); //write lsb
            PC = static_cast<uint16_t>(n * 0x08); //PC jumps to specififed memory location, return address in stack memory
            break;
    }
}

void SM83::pop_rr(ExecState& exec) {
    uint8_t rr = (exec.opcode >> 4) & 0x03;
    switch (exec.step) {
        case 0:
            exec.tmp = memory.read8(SP++);  // read low byte
            break;
        case 1:
            exec.tmp16 = (memory.read8(SP++) << 8) | exec.tmp;  // read high byte
            break;
        case 2:
            if (rr == 3) {
                A = exec.tmp16 >> 8;
                F = exec.tmp16 & 0xF0;
            } else {
                setRR(rr, exec.tmp16);
            }
            break;
    }
}

void SM83::ldh_a_n(ExecState& exec) {
    if (exec.opcode == 0xF0) {
        switch (exec.step) {
            case 0:
                exec.tmp16 = memory.read8(PC);
                PC++;
                break;
            case 1:
                exec.tmp = memory.read8(0xFF00 + exec.tmp16);
                break;
            case 2:
                A = exec.tmp;
                break;
        }
    } else if (exec.opcode == 0xF2) {
        switch (exec.step) {
            case 0:
                exec.tmp = memory.read8(0xFF00 + C);
                break;
            case 1:
                A = exec.tmp;
                break;
        }
    }
}

void SM83::ldh_n_a(ExecState& exec) {
    if (exec.opcode == 0xE2) {
        memory.write8(0xFF00 + C, A);
    } else if (exec.opcode == 0xE0) {
        switch (exec.step) {
            case 0:
                exec.tmp = memory.read8(PC);
                PC++;
                break;
            case 1:
                memory.write8(0xFF00 + exec.tmp, A);
                break;
        }
    }
}

void SM83::ei(ExecState& exec) {
    IME_next = 1;
}

//FIX: SECTION: Unprefixed
void SM83::bit_r(ExecState& exec) {
    uint8_t r = exec.opcode & 0x07; //01bb brrr
    uint8_t b = (exec.opcode >> 3) & 0x07;
    if ( r == 6 ) {
        switch (exec.step) {
            case 1:
                exec.tmp = memory.read8(getHL());
                break;
            case 2:
                uint8_t bit_masker = 1 << b;
                uint8_t bit_test = exec.tmp & bit_masker;

                setFlags(
                        F,
                        bit_test == 0,
                        0,
                        1,
                        (F & FLAG_C) != 0
                );
                break;
        }
    } else {
        r = getR(r);
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
}

void SM83::reset_r(ExecState& exec) {
    uint8_t b = (exec.opcode >> 3) & 0x07; //0 bb brrr
    uint8_t r = exec.opcode & 0x07;
    uint8_t reg;
    uint8_t bit_masker = ~(1 << b);

    if (r == 6) {
        switch (exec.step) {
            case 1:
                exec.tmp = memory.read8(getHL());
                break;
            case 2:
                memory.write8(getHL(), exec.tmp & bit_masker);
                break;
        }
    } else {
        reg = getR(r);
        setR(r, reg & bit_masker);
    }
}

void SM83::set_r(ExecState& exec) {
    uint8_t b = (exec.opcode >> 3) & 0x07; //0 bb brrr
    uint8_t r = exec.opcode & 0x07;
    uint8_t reg;
    uint8_t bit_masker = (1 << b);

    if (r == 6) {
        switch (exec.step) {
            case 1:
                exec.tmp = memory.read8(getHL());
                break;
            case 2:
                memory.write8(getHL(), exec.tmp | bit_masker);
                break;
        }
    } else {
        reg = getR(r);
        setR(r, reg | bit_masker);
    }
}

void SM83::swap_r(ExecState& exec) {
    uint8_t r = exec.opcode & 0x07;
    uint8_t reg, result;
    if (r == 6) {
        switch (exec.step) {
            case 1:
                exec.tmp = memory.read8(getHL());
                break;
            case 2: {
                reg = exec.tmp;
                uint8_t lo = reg & 0xF;
                uint8_t hi = (reg >> 4) & 0xF;
                result = (lo << 4) | hi;
                memory.write8(getHL(), result);
                    setFlags(
        F,
        (result == 0),
        0,
        0,
        0
    );
                break;
            }
        }
    } else {
        reg = getR(r);
        uint8_t lo = reg & 0xF;
        uint8_t hi = (reg >> 4) & 0xF;
        result = (lo << 4) | hi;
        setR(r, result);
        setFlags(
                F,
                (result == 0),
                0,
                0,
                0
                );
    }
}

void SM83::srl_r(ExecState& exec) {
    uint8_t r =exec.opcode & 0x07;
    uint8_t reg, oldReg;
    if (r == 6) {
        switch (exec.step) {
            case 1:
                exec.tmp = memory.read8(getHL());
                break;
            case 2: {
                reg = exec.tmp >> 1;
                memory.write8(getHL(), reg);
                uint8_t carry = exec.tmp & 0x01;
                setFlags(
                        F,
                        reg == 0,
                        0,
                        0,
                        carry
                );
                break;
            }
        }
    } else {
        oldReg = getR(r);
        reg = oldReg >> 1;
        setR(r, reg);
        uint8_t carry = oldReg & 0x01;
        setFlags(
                F,
                reg == 0,
                0,
                0,
                carry
                );
    }
}

void SM83::sla_r(ExecState& exec) {
    uint8_t r = exec.opcode & 0x07;
    uint8_t reg, oldReg;
    if (r == 6) {
        switch (exec.step) {
            case 1:
                exec.tmp = memory.read8(getHL());
                break;
            case 2:
                reg = exec.tmp << 1;
                memory.write8(getHL(), reg);
                uint8_t carry = (exec.tmp >> 7) & 0x01;
                setFlags(
                        F,
                        reg == 0,
                        0,
                        0,
                        carry
                );
                break;
        }
    } else {
        oldReg = getR(r);
        reg = oldReg << 1;
        setR(r, reg);
        uint8_t carry = (oldReg >> 7) & 0x01;
        setFlags(
                F,
                reg == 0,
                0,
                0,
                carry
                );
    }

}

void SM83::sra_r(ExecState& exec) {
    uint8_t r = exec.opcode & 0x07;
    uint8_t reg, oldReg;
    if (r == 6) {
        switch (exec.step) {
            case 1:
                exec.tmp = memory.read8(getHL());
                break;
            case 2:
                reg = (exec.tmp & 0x80) | (exec.tmp >> 1);
                memory.write8(getHL(), reg);
                uint8_t carry = exec.tmp & 0x01;
                setFlags(
                        F,
                        reg == 0,
                        0,
                        0,
                        carry
                );
                break;
        }
    } else {
        oldReg = getR(r);
        reg = (oldReg & 0x80) | (oldReg >> 1);
        setR(r, reg);
        uint8_t carry = oldReg & 0x01;
        setFlags(
                F,
                reg == 0,
                0,
                0,
                carry
        );
    }

}

void SM83::rr_r(ExecState& exec) {
    uint8_t r = exec.opcode & 0x07;
    uint8_t reg, oldReg;
    if (r == 6) {
        switch (exec.step) {
            case 1:
                exec.tmp = memory.read8(getHL());
                break;
            case 2:
                reg = (((F & FLAG_C) ? 1 : 0) << 7) | (exec.tmp >> 1);
                memory.write8(getHL(), reg);
                uint8_t carry = exec.tmp & 0x01;
                setFlags(
                        F,
                        reg == 0,
                        0,
                        0,
                        carry
                );
                break;
        }
    } else {
        oldReg = getR(r);
        reg = (((F & FLAG_C) ? 1 : 0) << 7) | (oldReg >> 1);
        setR(r, reg);
        uint8_t carry = oldReg & 0x01;
        setFlags(
                F,
                reg == 0,
                0,
                0,
                carry
        );
    }

}

void SM83::rl_r(ExecState& exec) {
    uint8_t r = exec.opcode & 0x07;
    uint8_t reg, oldReg;
    if (r == 6) {
        switch (exec.step) {
            case 1:
                exec.tmp = memory.read8(getHL());
                break;
            case 2:
                reg = (exec.tmp << 1) | ((F & FLAG_C) ? 1 : 0);
                memory.write8(getHL(), reg);
                uint8_t carry = (exec.tmp >> 7) & 0x01;
                setFlags(
                        F,
                        reg == 0,
                        0,
                        0,
                        carry
                );
                break;
        }
    } else {
        oldReg = getR(r);
        reg = (oldReg << 1) | ((F & FLAG_C) ? 1 : 0);
        setR(r, reg);
        uint8_t carry = (oldReg >> 7) & 0x01;
        setFlags(
                F,
                reg == 0,
                0,
                0,
                carry
        );
    }

}

void SM83::rrc_r(ExecState& exec) {
    uint8_t r = exec.opcode & 0x07;
    uint8_t reg, oldReg, carry;
    if (r == 6) {
        switch (exec.step) {
            case 1:
                exec.tmp = memory.read8(getHL());
                break;
            case 2:
                reg = (carry << 7) | (exec.tmp >> 1);
                memory.write8(getHL(), reg);
                carry = exec.tmp & 0x01;
                setFlags(
                        F,
                        reg == 0,
                        0,
                        0,
                        carry
                );
                break;
        }
    } else {
        oldReg = getR(r);
        carry = oldReg & 0x01;
        reg = (carry << 7) | (oldReg >> 1);
        setR(r, reg);
        setFlags(
                F,
                reg == 0,
                0,
                0,
                carry
        );
    }

}

void SM83::rlc_r(ExecState& exec) {
    uint8_t r = exec.opcode & 0x07;
    uint8_t reg, oldReg, carry;
    if (r == 6) {
        switch (exec.step) {
            case 1:
                exec.tmp = memory.read8(getHL());
                break;
            case 2:
                reg = (exec.tmp << 1) | (carry >> 7);
                memory.write8(getHL(), reg);
                carry = exec.tmp & 0x80;
                setFlags(
                        F,
                        reg == 0,
                        0,
                        0,
                        carry != 0
                );
                break;
        }
    } else {
        oldReg = getR(r);
        carry = oldReg & 0x80;
        reg = (oldReg << 1) | (carry >> 7);
        setR(r, reg);
        setFlags(
                F,
                reg == 0,
                0,
                0,
                carry != 0
        );
    }

}

void SM83::handle_interrupts() {

}

void SM83::cpu_tick(void) {  
    switch(phase) {
        case FETCH: {
            handle_interrupts();
            if (IME_next) IME = 1;

            exec.opcode = memory.read8(PC);
            PC++;
            exec.step = 0;
            phase = DECODE;
            break;
        }

        case DECODE: {
            if (exec.opcode == (uint8_t)(0xCB)) {
                exec.step = 0;
                phase = EXECUTE;
            } else {
                exec.instr = &unprefixed_instruction_table[exec.opcode];
                phase = EXECUTE;
            }
            break;
        }

        case EXECUTE: {
            if (exec.opcode == 0xCB && exec.step == 0) {
                exec.instr = &prefixed_instruction_table[memory.read8(PC)];
                PC++;
                exec.step++;
            }
            if (exec.instr->execute) {
                (this->*exec.instr->execute)(exec);
                int cost = exec.instr->cycles / 4;

                //if (exec.opcode == 0xCB) cost--;
                if (exec.condition_taken) {
                    cost += exec.instr->alt_cycles / 4;
                }

                if (++exec.step >= cost) {
                    exec.step = 0;
                    exec.condition_taken = 0;
                    phase = FETCH;
                }
            }
            break;
        }
    }    
}
