#include <iostream>
#include <bitset>
#include "CPU.h"
#include "Memory.h"

using namespace std;

void printCPUState(SM83& cpu, MEMORY& memory) {
    cout << "A: " << int(cpu.A) << " | B: " << int(cpu.B) << " | C: " << int(cpu.C) << endl;
    cout << "PC: " << hex << cpu.PC << " | cycles: " << dec << cpu.cycles << endl;

    cout << "Memory[0x0100..0x0105]: ";
    for (int i = 0x0100; i <= 0x0105; i++) {
        cout << hex << int(memory.ram[i]) << " ";
    }

    std::bitset<8> flags(cpu.F);
    std::cout << "Flags: " << flags << std::endl;

    // Optional: label the bits
    std::cout <<  "Z=" << flags[7]
              << " N=" << flags[6]
              << " H=" << flags[5]
              << " C=" << flags[4] << std::endl;
    cout << endl << "----------------------" << endl;
}

int main() {
    MEMORY memory;
    SM83 cpu(memory);

    // load test program
    memory.write8(0x0100, 0x00); // NOP
    memory.write8(0x0101, 0x06); // LD B,d8
    memory.write8(0x0102, 0x10); // value
    memory.write8(0x0103, 0x04); // INC B
    memory.write8(0x0104, 0x05); // DEC B
    memory.write8(0x0105, 0x00); // NOP

    cpu.PC = 0x0100;

    for (int i = 0; i < 6; i++) {
        cpu.execute();
        printCPUState(cpu, memory);
        cout << endl << "Press enter to continue" << endl;
        cin.get();
    }

    return 0;
}
