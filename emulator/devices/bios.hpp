#pragma once

#include "../aliases.hpp"
#include "../device.hpp"

#include <fstream>
#include <array>

namespace machine {
    class bios : public device {
        using device_access = device::access_mode;
        typedef std::array<u8, 4095> array_t;

        array_t binary = { 0 };

    public:
        bios(std::string name) : device(name, 0ull, 0xfffull, 0u, device_access::a_rx) {}

        array_t& get_binary() { return binary; }

        void load_binary(const std::string bios) {
            std::ifstream f(bios, std::ios::binary);

            if (!f.is_open()) {
                // Issue fatal error: couldn't open BIOS
            }

            for (size_t i = 0; (i < binary.size()) && (!f.eof()); i++) {
                binary[i] = f.get();
            }

            f.close();
        }

        // device-inherited functions
        u8 rb(u64 addr) override {
            if (addr > 4095) {
                // Issue warning, Out of bounds read while reading from device (ARCH64_SIGSEGV)
                return 0xff;
            }
            return binary[addr];
        }
        u16 rw(u64 addr) override {
            u16 word = 0;
            for (int off = 1; off >= 0; off--) {
                word |= ((u16)rb(addr+off) << (8 * off));
            }
            return word;
        }
        u32 rd(u64 addr) override {
            u32 dword = 0;
            for (int off = 3; off >= 0; off--) {
                dword |= ((u32)rb(addr+off) << (8 * off));
            }
            return dword;
        }
        u64 rq(u64 addr) override {
            u64 qword = 0;
            for (int off = 7; off >= 0; off--) {
                qword |= ((u64)rb(addr+off)) << (8 * off);
            }
            return qword;
        }
    };
};