#include <catch2/catch_test_macros.hpp>
#include "../src/GameBoy.cpp"
GameBoy gb;
auto& cpu = gb.CPU;
auto& memory = gb.MMU;

TEST_CASE("LD r, r works!") {
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

    cpu.PC = 0xC000;
    memory.write8(cpu.PC+1, 0xAB);

    for (int dest = 0; dest < 8; dest++) {
            if (dest == 6) continue;
           uint8_t opcode = 0x06 | (dest << 3);
           cpu.ld_r_d8(opcode);
           REQUIRE(cpu.getR(dest) == 0xAB);
    }
}

TEST_CASE("LD RR, D16 works!") {

    cpu.PC = 0xC000;
    memory.write16(cpu.PC+1, 0xABCD);

    for (int dest = 0; dest < 4; dest++) {
        uint8_t opcode = (dest << 4) | 0x01;
        cpu.ld_rr_d16(opcode);
        REQUIRE(cpu.getRR(dest) == 0xABCD);
    }
}

TEST_CASE("LD A,(rr) works!") {
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
    cpu.PC = 0;
    memory.write8(0x1234, 0xAB);
    memory.write16(cpu.PC+1, 0x1234);
    cpu.ld_a_nn(0);
    REQUIRE(cpu.A == 0xAB);
}

TEST_CASE("LD nn, A") {
    cpu.PC = 0;
    cpu.A = 0x12;
    memory.write8(0xAB, 0x12);
    memory.write16(cpu.PC+1, 0xAB);
    cpu.ld_nn_a(0);
    REQUIRE(cpu.A == 0x12);
}

TEST_CASE("INC r works!") {
    //00RRRXXX
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
    cpu.A = 0x00;
    cpu.B = 0x00;
    cpu.C = 0x00;
    cpu.D = 0x00;
    cpu.E = 0x00;
    cpu.H = 0x00;
    cpu.L = 0x00;

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
        cpu.SP = 0x00;
        cpu.inc_rr(0x33);
        REQUIRE(cpu.SP == 0x01);
    }
}

TEST_CASE("DEC r works!") {
    //00RRRXXX

    cpu.A = 0x01;
    cpu.B = 0x01;
    cpu.C = 0x01;
    cpu.D = 0x01;
    cpu.E = 0x01;
    cpu.H = 0x01;
    cpu.L = 0x01;

    for (int dest = 0; dest < 8; dest++) {
        if (dest == 6) continue;
        const uint8_t opcode = 0x05 | (dest << 3);
        cpu.dec_r(opcode);
        REQUIRE(cpu.getR(dest) == 0x00);
    }
}

TEST_CASE("DEC RR works!") {
    cpu.A = 0x01;
    cpu.B = 0x00;
    cpu.C = 0x01;
    cpu.D = 0x00;
    cpu.E = 0x01;
    cpu.H = 0x00;
    cpu.L = 0x01;

    SECTION("DEC BC") {
        cpu.dec_rr(0x0B);
        REQUIRE(cpu.getBC() == 0x00);
    }
    SECTION("DEC DE") {
        cpu.dec_rr(0x1B);
        REQUIRE(cpu.getDE() == 0x00);
    }
    SECTION("DEC HL") {
        cpu.dec_rr(0x2B);
        REQUIRE(cpu.getHL() == 0x00);
    }
    SECTION("DEC SP") {
        cpu.SP = 0x01;
        cpu.dec_rr(0x3B);
        REQUIRE(cpu.SP == 0x00);
    }
}

TEST_CASE("ADD A, r") {
    for (int dest = 0; dest < 8; dest++) {
        if (dest == 6) continue;

        uint8_t opcode = (dest >> 3);
        cpu.add_a_r(opcode);
        // REQUIRE();
    }
}

TEST_CASE("INC r flag tests") {
    SECTION("Z flag set when result is 0") {
        cpu.A = 0xFF;
        cpu.inc_r(0x3C); // INC A
        REQUIRE(cpu.A == 0x00);
        REQUIRE((cpu.F & FLAG_Z) != 0);
        REQUIRE((cpu.F & FLAG_N) == 0);
        REQUIRE((cpu.F & FLAG_H) != 0);
    }
    SECTION("H flag set on half carry") {
        cpu.A = 0x0F;
        cpu.inc_r(0x3C); // INC A
        REQUIRE(cpu.A == 0x10);
        REQUIRE((cpu.F & FLAG_H) != 0);
        REQUIRE((cpu.F & FLAG_Z) == 0);
    }
    SECTION("No flags set on normal INC") {
        cpu.A = 0x01;
        cpu.inc_r(0x3C); // INC A
        REQUIRE(cpu.A == 0x02);
        REQUIRE((cpu.F & FLAG_Z) == 0);
        REQUIRE((cpu.F & FLAG_N) == 0);
        REQUIRE((cpu.F & FLAG_H) == 0);
    }
    SECTION("C flag unchanged") {
        cpu.F = FLAG_C;
        cpu.A = 0x01;
        cpu.inc_r(0x3C);
        REQUIRE((cpu.F & FLAG_C) != 0); // C preserved
    }
}

TEST_CASE("DEC r flag tests") {
    SECTION("Z flag set when result is 0") {
        cpu.A = 0x01;
        cpu.dec_r(0x3D); // DEC A
        REQUIRE(cpu.A == 0x00);
        REQUIRE((cpu.F & FLAG_Z) != 0);
        REQUIRE((cpu.F & FLAG_N) != 0);
        REQUIRE((cpu.F & FLAG_H) == 0);
    }
    SECTION("H flag set on half borrow") {
        cpu.A = 0x10;
        cpu.dec_r(0x3D); // DEC A
        REQUIRE(cpu.A == 0x0F);
        REQUIRE((cpu.F & FLAG_H) != 0);
        REQUIRE((cpu.F & FLAG_N) != 0);
    }
    SECTION("Wrap around 0x00 to 0xFF") {
        cpu.A = 0x00;
        cpu.dec_r(0x3D);
        REQUIRE(cpu.A == 0xFF);
        REQUIRE((cpu.F & FLAG_Z) == 0);
        REQUIRE((cpu.F & FLAG_N) != 0);
        REQUIRE((cpu.F & FLAG_H) != 0);
    }
    SECTION("C flag unchanged") {
        cpu.F = FLAG_C;
        cpu.A = 0x01;
        cpu.dec_r(0x3D);
        REQUIRE((cpu.F & FLAG_C) != 0); // C preserved
    }
}

TEST_CASE("ADD A, r flag tests") {
    SECTION("Z flag set when result is 0") {
        cpu.A = 0xFF;
        cpu.B = 0x01;
        cpu.add_a_r(0x80); // ADD A, B
        REQUIRE(cpu.A == 0x00);
        REQUIRE((cpu.F & FLAG_Z) != 0);
        REQUIRE((cpu.F & FLAG_C) != 0);
        REQUIRE((cpu.F & FLAG_N) == 0);
    }
    SECTION("H flag on half carry") {
        cpu.A = 0x0F;
        cpu.B = 0x01;
        cpu.add_a_r(0x80);
        REQUIRE(cpu.A == 0x10);
        REQUIRE((cpu.F & FLAG_H) != 0);
        REQUIRE((cpu.F & FLAG_C) == 0);
    }
    SECTION("C flag on carry") {
        cpu.A = 0xFF;
        cpu.B = 0x02;
        cpu.add_a_r(0x80);
        REQUIRE(cpu.A == 0x01);
        REQUIRE((cpu.F & FLAG_C) != 0);
        REQUIRE((cpu.F & FLAG_Z) == 0);
    }
    SECTION("No flags on normal add") {
        cpu.A = 0x01;
        cpu.B = 0x01;
        cpu.add_a_r(0x80);
        REQUIRE(cpu.A == 0x02);
        REQUIRE(cpu.F == 0x00);
    }
}

TEST_CASE("SUB A, r flag tests") {
    SECTION("Z flag set when result is 0") {
        cpu.A = 0x05;
        cpu.B = 0x05;
        cpu.sub_a_r(0x90); // SUB A, B
        REQUIRE(cpu.A == 0x00);
        REQUIRE((cpu.F & FLAG_Z) != 0);
        REQUIRE((cpu.F & FLAG_N) != 0);
        REQUIRE((cpu.F & FLAG_C) == 0);
    }
    SECTION("C flag set when borrow") {
        cpu.A = 0x00;
        cpu.B = 0x01;
        cpu.sub_a_r(0x90);
        REQUIRE(cpu.A == 0xFF);
        REQUIRE((cpu.F & FLAG_C) != 0);
        REQUIRE((cpu.F & FLAG_N) != 0);
    }
    SECTION("H flag set on half borrow") {
        cpu.A = 0x10;
        cpu.B = 0x01;
        cpu.sub_a_r(0x90);
        REQUIRE(cpu.A == 0x0F);
        REQUIRE((cpu.F & FLAG_H) != 0);
        REQUIRE((cpu.F & FLAG_N) != 0);
    }
}

TEST_CASE("AND flag tests") {
    SECTION("Z flag when result is 0") {
        cpu.A = 0xF0;
        cpu.B = 0x0F;
        cpu._and_r(0xA0); // AND A, B
        REQUIRE(cpu.A == 0x00);
        REQUIRE((cpu.F & FLAG_Z) != 0);
        REQUIRE((cpu.F & FLAG_H) != 0);
        REQUIRE((cpu.F & FLAG_N) == 0);
        REQUIRE((cpu.F & FLAG_C) == 0);
    }
    SECTION("H always set") {
        cpu.A = 0xFF;
        cpu.B = 0xFF;
        cpu._and_r(0xA0);
        REQUIRE((cpu.F & FLAG_H) != 0);
    }
}

TEST_CASE("OR flag tests") {
    SECTION("Z flag when result is 0") {
        cpu.A = 0x00;
        cpu.B = 0x00;
        cpu.or_r(0xB0); // OR A, B
        REQUIRE(cpu.A == 0x00);
        REQUIRE((cpu.F & FLAG_Z) != 0);
        REQUIRE((cpu.F & FLAG_N) == 0);
        REQUIRE((cpu.F & FLAG_C) == 0);
        REQUIRE((cpu.F & FLAG_H) == 0);
    }
    SECTION("No flags on normal OR") {
        cpu.A = 0xF0;
        cpu.B = 0x0F;
        cpu.or_r(0xB0);
        REQUIRE(cpu.A == 0xFF);
        REQUIRE(cpu.F == 0x00);
    }
}

TEST_CASE("XOR flag tests") {
    SECTION("Z flag when result is 0") {
        cpu.A = 0xFF;
        cpu.B = 0xFF;
        cpu._xor_r(0xA8); // XOR A, B
        REQUIRE(cpu.A == 0x00);
        REQUIRE((cpu.F & FLAG_Z) != 0);
        REQUIRE((cpu.F & FLAG_N) == 0);
        REQUIRE((cpu.F & FLAG_C) == 0);
        REQUIRE((cpu.F & FLAG_H) == 0);
    }
}

TEST_CASE("CP flag tests") {
    SECTION("Z flag when equal") {
        cpu.A = 0x05;
        cpu.B = 0x05;
        cpu.cp_r(0xB8); // CP A, B
        REQUIRE(cpu.A == 0x05); // A unchanged
        REQUIRE((cpu.F & FLAG_Z) != 0);
        REQUIRE((cpu.F & FLAG_N) != 0);
    }
    SECTION("C flag when A < r") {
        cpu.A = 0x01;
        cpu.B = 0x02;
        cpu.cp_r(0xB8);
        REQUIRE((cpu.F & FLAG_C) != 0);
        REQUIRE((cpu.F & FLAG_N) != 0);
    }
}

//Prefixed:
TEST_CASE("BIT R") {

}
