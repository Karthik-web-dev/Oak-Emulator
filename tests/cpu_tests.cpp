#include <catch2/catch_test_macros.hpp>
#include "CPU.h"
#include "Memory.h"


TEST_CASE("LD r, r works!") {
    MEMORY memory;
    SM83 cpu(memory);
    cpu.A = 0x00;
    cpu.B = 0x12;
    cpu.C = 0x34;
    cpu.D = 0x56;
    cpu.E = 0x78;
    cpu.H = 0x91;

    for (int dest = 0; dest < 8; dest++) {
        for (int src = 0; src < 8; src++) {
            if (dest == src) continue;
            if (dest == 6 || src == 6) continue;
            cpu.A = 0x00; cpu.B = 0x12; cpu.C = 0x34;
            cpu.D = 0x56; cpu.E = 0x78; cpu.H = 0x91;

            uint8_t opcode = 0x40 | (dest << 3) | src;
            uint8_t srcVal = cpu.getR(src);

            cpu.ld_r_r(opcode);

            REQUIRE(cpu.getR(dest) == srcVal);
        }
    }
}

TEST_CASE("LD R, d8 works!") {
    MEMORY memory;
    SM83 cpu(memory);

    cpu.PC = 0;
    memory.write8(cpu.PC+1, 0xAB);

    for (int dest = 0; dest < 8; dest++) {
            if (dest == 6) continue;
           uint8_t opcode = 0x06 | (dest << 3);
           cpu.ld_r_d8(opcode);
           REQUIRE(cpu.getR(dest) == 0xAB);
    }
}

TEST_CASE("LD RR, D16 works!") {
    MEMORY memory;
    SM83 cpu(memory);

    cpu.PC = 0;
    memory.write16(cpu.PC+1, 0xABCD);

    for (int dest = 0; dest < 4; dest++) {
        uint8_t opcode = (dest << 4) | 0x01;
        cpu.ld_rr_d16(opcode);
        REQUIRE(cpu.getRR(dest) == 0xABCD);
    }
}

TEST_CASE("LD A,(rr) works!") {
    MEMORY memory;
    SM83 cpu(memory);

    // Set BC and DE registers
    cpu.B = 0x12;
    cpu.C = 0x34;
    cpu.D = 0x56;
    cpu.E = 0x78;

    // Write values to memory at addresses BC and DE
    cpu.memory.write8(0x1234, 0xAB);
    cpu.memory.write8(0x5678, 0xCD);

    SECTION("LD A,(BC)") {
        cpu.ld_a_rr(0x0A);
        REQUIRE(cpu.A == 0xAB);
    }

    SECTION("LD A,(DE)") {
        cpu.ld_a_rr(0x1A);
        REQUIRE(cpu.A == 0xCD);
    }
}

TEST_CASE("LD (rr), A works!") {
    MEMORY memory;
    SM83 cpu(memory);

    cpu.A = 0x99;
    cpu.B = 0x12;
    cpu.C = 0x34;
    cpu.D = 0x56;
    cpu.E = 0x78;

    memory.write8(0x1234, 0xAB);
    memory.write8(0x5678, 0xCD);

    SECTION("LD (BC), A") {
        cpu.ld_rr_a(0x02);
        REQUIRE(int(memory.read8(0x1234)) == 0x99);
    }

    SECTION("LD (DE), A") {
        cpu.ld_rr_a(0x12);
        REQUIRE(int(memory.read8(0x5678)) == 0x99);
    }
}

TEST_CASE("LD A, nn") {
    MEMORY memory;
    SM83 cpu(memory);

    cpu.PC = 0;
    memory.write8(0x1234, 0xAB);
    memory.write16(cpu.PC+1, 0x1234);
    cpu.ld_a_nn(0);
    REQUIRE(cpu.A == 0xAB);
}

TEST_CASE("LD nn, A") {
    MEMORY memory;
    SM83 cpu(memory);

    cpu.PC = 0;
    cpu.A = 0x12;
    memory.write8(0xAB, 0x12);
    memory.write16(cpu.PC+1, 0xAB);
    cpu.ld_nn_a(0);
    REQUIRE(cpu.A == 0x12);
}

TEST_CASE("INC r works!") {
    //00RRRXXX
    MEMORY memory;
    SM83 cpu(memory);

    cpu.A = 0x01;
    cpu.B = 0x01;
    cpu.C = 0x01;
    cpu.D = 0x01;
    cpu.E = 0x01;
    cpu.H = 0x01;
    cpu.L = 0x01;

    for (int dest = 0; dest < 8; dest++) {
        if (dest == 6) continue;
        uint8_t opcode = 0x04 | (dest << 3);
        cpu.inc_r(opcode);
        REQUIRE(cpu.getR(dest) == 0x02);
    }
}

TEST_CASE("INC RR works!") {
    MEMORY memory;
    SM83 cpu(memory);
    SECTION("INC BC") {
        cpu.inc_rr(0x03);
        REQUIRE(cpu.getBC() == 0x01);
    }
    SECTION("INC DE") {
        cpu.inc_rr(0x13);
        REQUIRE(cpu.getDE() == 0x01);
    }
    SECTION("INC HL") {
        cpu.inc_rr(0x23);
        REQUIRE(cpu.getHL() == 0x01);
    }
    SECTION("INC SP") {
        cpu.inc_rr(0x33);
        REQUIRE(cpu.SP == 0x01);
    }
}

TEST_CASE("DEC r works!") {
    //00RRRXXX
    MEMORY memory;
    SM83 cpu(memory);

    cpu.A = 0x01;
    cpu.B = 0x01;
    cpu.C = 0x01;
    cpu.D = 0x01;
    cpu.E = 0x01;
    cpu.H = 0x01;
    cpu.L = 0x01;

    for (int dest = 0; dest < 8; dest++) {
        if (dest == 6) continue;
        const uint8_t opcode = 0x04 | (dest << 3);
        cpu.dec_r(opcode);
        REQUIRE(cpu.getR(dest) == 0x00);
    }
}

TEST_CASE("DEC RR works!") {
    MEMORY memory;
    SM83 cpu(memory);

    cpu.A = 0x01;
    cpu.B = 0x01;
    cpu.C = 0x01;
    cpu.D = 0x01;
    cpu.E = 0x01;
    cpu.H = 0x01;
    cpu.L = 0x01;

    SECTION("DEC BC") {
        cpu.dec_rr(0x03);
        REQUIRE(cpu.getBC() == 0x00);
    }
    SECTION("DEC DE") {
        cpu.dec_rr(0x13);
        REQUIRE(cpu.getDE() == 0x00);
    }
    SECTION("DEC HL") {
        cpu.dec_rr(0x23);
        REQUIRE(cpu.getHL() == 0x00);
    }
    SECTION("DEC SP") {
        cpu.dec_rr(0x33);
        REQUIRE(cpu.SP == 0x00);
    }
}

TEST_CASE("ADD A, r") {
    MEMORY memory;
    SM83 cpu(memory);

    for (int dest = 0; dest < 8; dest++) {
        if (dest == 6) continue;

        uint8_t opcode = (dest >> 3);
        cpu.add_a_r(opcode);
        // REQUIRE();
    }
}
void test_add_a_n();
void test_add_hl_rr();
void test_add_sp_n();

void test_sub_a_r();
void test_sub_a_n();
void test_sub_hl_rr();
void test_sub_sp_n();

void test_rlca();
void test_rrca();
void test_rla();
void test_stop();
