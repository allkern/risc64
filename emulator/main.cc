#include <cxxabi.h>

#include <iostream>
#include <iomanip>
#include <string>
#include <typeinfo>
#include <fstream>
#include <cassert>

const std::string demangle(const char* name) {
    int status = -4;
    char* res = abi::__cxa_demangle(name, NULL, NULL, &status);
    const char* const demangled_name = (status==0)?res:name;
    std::string ret_val(demangled_name);
    free(res);
    return ret_val;
}

#include "emulator/cpu.hpp"
#include "emulator/decoder.hpp"
#include "emulator/aliases.hpp"
#include "emulator/bus.hpp"
#include "emulator/devices/bios.hpp"



int main() {
    using namespace machine;

    machine::bios my_bios("SimpleBIOS");
    machine::cpu<0> proc0;

    my_bios.load_binary("bin/boot.bin");

    bus::attach_device(proc0);
    bus::attach_device(my_bios);

    for (int i = 0; i < 7; i++) {
        proc0.fetch_decode();
        proc0.execute();
    }

    for (int i = 0; i < 7; i++) {
        std::cout << std::hex << "gpr0 = 0x" << (unsigned long long)proc0.get_register(i) << std::endl;
    }
    
    std::cout << "pc = 0x" << proc0.get_pc() << std::endl;

    return 0;
}