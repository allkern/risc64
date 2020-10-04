#pragma once

#include "cpu.hpp"

void cpu_loop(machine::cpu* proc) {
#ifdef A64_DEBUG
    auto exec = proc->get_execution_state();
#endif

    while (!proc->cpu_halted()) {
        proc->fetch_decode();

        #ifdef A64_DEBUG
        std::cout << "memory[pc] = 0x" << std::hex << exec->opcode << std::endl;
        #endif
        proc->execute();

        #ifdef A64_DEBUG

        std::cout << "sr = 0b" << machine::bin(proc->get_sr()) << std::endl;
        std::cout << "pc = 0x" << std::hex << proc->get_pc() << std::endl << std::endl;
        std::cout << "pci = 0x" << std::hex << proc->get_pci() << std::endl << std::endl;

        //system("pause");
        system("clear");
        #endif
    }

    std::cout << "cpu" << proc->get_thread_id() << " was halted!\n";
}