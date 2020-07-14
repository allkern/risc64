#include <cxxabi.h>

#include <iostream>
#include <iomanip>
#include <string>
#include <typeinfo>
#include <fstream>


const std::string demangle(const char* name) {
    int status = -4;
    char* res = abi::__cxa_demangle(name, NULL, NULL, &status);
    const char* const demangled_name = (status==0)?res:name;
    std::string ret_val(demangled_name);
    free(res);
    return ret_val;
}

#include "cpu.hpp"

machine::cpu<0> proc;

int main() {
    std::cout << machine::bin(0xc012c0003ull) << std::endl;
    machine::cpu<0>::instruction i(0xc012c0003ull);
    machine::cpu<0>::instruction::decode(i);
    std::cout << machine::cpu<0>::instruction::disassemble(i) << std::endl;
    return 0;
}

/*
#include "elf/header.hpp"

elf::header <elf::target::bits64> e_hdr;

template <elf::target tprocw> void print_dump(elf::header <tprocw>& hdr) {
    std::ofstream f("test.elf", std::ios::binary);
    size_t idx = 0;
    for (const elf::u8& d : hdr) {
        bool line_begin = ((idx) % 0x10) == 0;
        if (line_begin) { std::cout << (idx != 0 ? "\n" : "") << "+" << std::hex << std::setfill('0') << std::setw(4) << (elf::u16)idx << ": "; }
        std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)d << ' ';
        f.put(d);
        idx++;
    }
    f.close();
    std::cout << std::endl;
}

int main() {
    print_dump(e_hdr);
    return 0;
}
*/