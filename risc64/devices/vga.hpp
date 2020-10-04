#include <array>

#include "../device.hpp"
#include "../aliases.hpp"

#include "../lgw/threaded_window.hpp"

namespace machine {
    namespace detail {
        inline u8 clamp(u16 res) { return res > 0xff ? 0xff : res; }
    }

    class vga : public device, public lgw::threaded_window {
        using device::access_mode = device_access;

        struct character {
            u8 ascii, attr;

            character(u16 v) : ascii(v & 0xff), attr((v & 0xff00) >> 8) {};
        };

        // 512K of VRAM, double of what a normal VGA chip would have
        // Supported video modes:
        // 640x480@8-bit color
        // 320x240@16-bit color
        // Both of these video modes use around 300K of video memory
        std::array <0x80000, u8> vram;
        
        // VGA_CONTROL register:
        // m: Mode select
        //   Video modes:
        //   00 -> 640x480@8-bit color
        //   01 -> 320x240@16-bit color
        //   Text modes:
        //   00 -> 80x25
        // t: Text mode
        //   0  -> Bitmap/video mode
        //   1  -> Text mode
        // r: Frame ready
        //   Makes the controller raster this frame, when its done, the controller
        //   will reset this bit. The user will be unable to read or write to/from vram
        //   during this time.
        // c: Clear
        //   Makes the controller clear the screen, when its done, this bit will be
        //   reset.
        // 000crtmm
        u8 r[1] = { 0 };

        // 80x25 Text mode
        // 80x25x2 (character+attribute) = 4000 = 0xfa0, 0xa0000 + 0xfa0 = 0xa0fa0
        std::array <0x7d0, character> text_mode_buf;

        inline sf::Color get_rgb32_8bit(u8 c) {
            u8  r = detail::clamp((((u16)c & 0xe0 /*0b11100000*/) >> 5) * 0x20),
                g = detail::clamp((((u16)c & 0x1c /*0b00011100*/) >> 2) * 0x20),
                b = detail::clamp( ((u16)c & 0x03 /*0b00000011*/)       * 0x40);
            return sf::Color(r, g, b);
        }

        void render_video_buffer() {
            u8 video_mode = r[0] & 3;
            switch (video_mode) {
                case 0x0: {
                    for (int y = 0; y < 640; y++) {
                        for (int x = 0; x < 480; x++) {
                            buffer_draw(x, y, get_rgb32_8bit(vram[x+(y*480)]));
                        }
                    }
                } break;
            }
        }

    public:
        // Default mmio_base maps to the real VGA base
        vga(u64 mmio_base = 0xa0000) :
            device("VGA Display Controller", mmio_base, vram.size() + 1, 0xa, device_access::a_rw) {
                vram.fill(0xaa);
        };
        
        u64 read(u64 addr, size_t size) override {
            addr = addr - base;
            // Handle register reads
            if (addr >= vram.size()) {
                if (size > 1) {
                    // Reads of size > 1 are invalid on VGA registers
                    return 0xffffffffffffffff;
                }
                return r[addr - vram.size()];
            }
            u64 q = 0;

            for (int off = size - 1; off >= 0; off--) {
                q |= ((u64)vram[addr+off]) << (8 * off);
            }
            return q;
        }

        #define test_bit(reg, b) (r[reg] & (1 << b))
        void write(u64 addr, u64 value, size_t size) {
            addr = addr - base;
            // Handle register writes
            if (addr >= vram.size()) {
                if (size > 1) {
                    // Writes of size > 1 are invalid on VGA registers
                    return;
                }
                r[addr - vram.size()] = value;

                // Side-effects:
                if (test_bit(0, 5)) buffer_clear(sf::Color(0, 0, 0));
                if (test_bit(0, 4)) render_vga_buffer();
            }
        }
    }
}