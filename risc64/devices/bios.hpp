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
        bios(std::string name) : device(name, 0ull, 0xfffull, 8u, device_access::a_rx) {}

        array_t& get_binary() { return binary; }

        void load_binary(const std::string name) {
            std::ifstream f(name, std::ios::binary);

            if (!f.is_open()) {
                // Issue fatal error: couldn't open BIOS
            }

            for (size_t i = 0; (i < binary.size()) && (!f.eof()); i++) {
                binary[i] = f.get();
            }

            f.close();
        }

        // device-inherited functions
        u64 read(u64 addr, size_t size) override {
            u64 q = 0;
            for (int off = size - 1; off >= 0; off--) {
                q |= ((u64)binary[addr+off]) << (8 * off);
            }
            return q;
        }
    };
};