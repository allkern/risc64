#include "../aliases.hpp"
#include "../device.hpp"

#include <iostream>
#include <array>

namespace machine {
    class tty : public device {
        using device_access = device::access_mode;

        u8 registers[5] = { 0 };
        
        // r[0] -> coutr
        // r[1] -> statr
        // r[2] -> prntx
        // r[3] -> prnty
        // r[4] -> prntc

    public:
        tty(u64 mmio_base) : device("Terminal Controller", mmio_base, 4, 4, device_access::a_rw) {};

        u64 read(u64 addr, size_t size) override {
            #ifdef DEBUG
            std::cout << "[read] tty_mmio_base+0x" << std::hex << (addr - base) << ", size = 0x" << size << std::endl;
            #endif
            // Hardware fault, terminal cannot read more than a byte at once
            if (size > 1) { return 0xffffffffffffffff; }
            return ((u64)registers[addr - base]);
        }

        void write(u64 addr, u64 value, size_t size) override {
            #ifdef DEBUG
            std::cout << "[write] tty_mmio_base+0x" << std::hex << (addr - base) << ", value = 0x" << value << ", size = 0x" << size << std::endl;
            #endif
            // Hardware fault, terminal cannot write more than a byte at once
            if (size > 1) { return; }
            registers[addr - base] = value;
            
            // Check READY bit and react accordingly
            if (registers[1] & 0x4) {
                if (registers[0] > 0) {
                    std::cout << (char)registers[0];
                };
                registers[1] &= (~0x4);
            }
        }
        
    };
}