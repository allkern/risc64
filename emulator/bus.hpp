#pragma once

#include <algorithm>
#include <vector>
#include <memory>

#include "device.hpp"
#include "devices/bios.hpp"

#define BUS_MAX_DEVICES 20

#define read_function(s, b, def) \
    inline u##b r##s(u64 addr) { \
        for (auto d : devices) { \
            u64 base_addr = d->get_base(), \
                end_addr = base_addr + d->get_size(); \
            if ((addr >= base_addr) && (addr <= end_addr)) { \
                if (!(d->get_access_mode() & device::access_mode::a_r)) { \
                    return def; \
                } \
                return d->r##s(addr); \
            } \
        } \
    }

#define write_function(s, b) \
    inline void wb(u64 addr, u##b value) { \
        for (auto d : devices) { \
            u64 base_addr = d->get_base(), \
                end_addr = base_addr + d->get_size(); \
            if ((addr >= base_addr) && (addr <= end_addr)) { \
                if (!(d->get_access_mode() & device::access_mode::a_w)) { \
                } \
                d->w##s(addr, value); \
            } \
        } \
    }

namespace machine {
	namespace bus {
        size_t hid_count = 0;

		std::vector <machine::device*> devices;

        read_function(b, 8 , 0xff);
        read_function(w, 16, 0xffff);
        read_function(d, 32, 0xffffffff);
        read_function(q, 64, 0xffffffffffffffff);

        write_function(b, 8 );
        write_function(w, 16);
        write_function(d, 32);
        write_function(q, 64);

        template <class Device> inline void attach_device(Device& d) {
            devices.push_back(&d);
        }

        template <class Device> inline Device& get_device(u16 hid) {
            for (auto d : devices) {
                if (hid == d->get_hid()) return static_cast<Device&>(*d);
            }
            // issue warning, device not found
        }

        void init() {
            devices.reserve(BUS_MAX_DEVICES);
        }
	};
};