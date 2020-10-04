#pragma once

#include <iostream>
#include <array>

#include "../aliases.hpp"
#include "../device.hpp"

#define LGW_OPTIMIZE
//#define LGW_ENABLE_MUTEXES
#include "../lgw/threaded_window.hpp"

namespace machine {
    class ioctl : public device, public lgw::threaded_window {
        friend class control_window;

        using device_access = device::access_mode;

        u8 registers[10] = { 0 };

        size_t window_scale = 2;

        std::string data = "";
        sf::Text str;
        sf::Font term;
        sf::Clock cursor_clk;
        
        bool cursor_on;
        
        // r[0] -> term_char_out
        // r[1] -> term_status
        // r[2] -> term_print_x
        // r[3] -> term_print_y
        // r[4] -> term_print_c
        // r[5] -> keyb_key_code
        // r[6] -> keyb_status
        // r[7] -> mouse_status
        // r[8] -> mouse_x
        // r[9] -> mouse_y

    public:
        u8* get_memory() { return &registers[0]; }

        ioctl(u64 mmio_base, size_t scale) :
            device("Generic I/O Controller", mmio_base, 9, 2, device_access::a_rw),
            window_scale(scale) {};

        void init_display() {
            init(640, 480, "IOCTL Terminal Display", sf::Style::Default, false, true);
        }

        u64 read(u64 addr, size_t size) override {
            // Hardware fault, terminal cannot read more than a byte at once
            if (size > 1) { return 0xffffffffffffffff; }
            u64 qword = (u64)registers[addr-base];
 
            return qword;
        }

        void write(u64 addr, u64 value, size_t size) override {
            //std::cout << "write to _mmio_ioctl_base+0x" << std::hex << (unsigned int)(addr-base) << ", value = 0x" << (unsigned int)value << std::endl;
            // Hardware fault, terminal cannot write more than a byte at once
            if (size > 1) { return; }
            registers[addr - base] = value;

            // A0000R00 -> R = READY, A = KEY_ACK
            // Key acknowledgement
            if (registers[6] & 0x80) {
                registers[5] = 0;
                registers[6] &= (~0x80);
            }
            // Check READY bit and react accordingly
            if (registers[1] & 0x4) {
                if (registers[0] > 0) {
                    data += (char)registers[0];
                    str.setString(data);
                };
                registers[1] &= (~0x4);
            }
        }

        void on_key(sf::Uint32 key) override {
            switch (key) {
                case 0xd: registers[5] = 0xa; break;
                case 0x8: if (data.size()) { data.pop_back(); str.setString(data); }; break;
                default: registers[5] = key; break;
            }
        }
    
        void setup() override {
            scale(window_scale);
#ifdef _WIN32
            term.loadFromFile("res/terminal.ttf");
#else
            term.loadFromFile("risc64/res/terminal.ttf");
#endif
            str.setCharacterSize(20);
            str.setPosition(2, -8);
            str.setFont(term);
            str.setString(data);
            str.setFillColor(sf::Color(0xfcfcfcff));
            cursor_clk.restart();
            get_window()->setMouseCursorVisible(false);
            get_window()->setPosition(sf::Vector2i(350, 25));
        }


        void draw() override {
            auto w = get_window();
            clear(sf::Color::Black);

            //if (round(cursor_clk.getElapsedTime().asSeconds()) == 0.5f) {
            //    if (cursor_on) {
            //        data.pop_back();
            //    } else {
            //        data += "_";
            //    }
            //    str.setString(data);
            //    cursor_on = !cursor_on;
            //    cursor_clk.restart();
            //}

            get_window()->draw(str);
        }

        void on_close() override {
            close();
        }
    };
}