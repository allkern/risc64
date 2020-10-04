#pragma once

#include <algorithm>
#include <vector>
#include <memory>

#include "device.hpp"
#include "aliases.hpp"
#include "devices/bios.hpp"

#include "log.hpp"

#define BUS_MAX_DEVICES 20

namespace machine {
	namespace bus {
        size_t hid_count = 0;

		std::vector <machine::device*> devices;

        inline u64 read(u64 addr, size_t size) {
            for (auto d : devices) {
                u64 base_addr = d->get_base(),
                    end_addr = base_addr + d->get_size();
                if ((addr >= base_addr) && (addr <= end_addr)) {
                    if (!(d->get_access_mode() & device::access_mode::a_r)) {
                        _log(warning, "Invalid read on device %s @ 0x%llx, addr = 0x%llx, size = 0x%llx (RISC64_SIGSEGV)", d->get_name().c_str(), base_addr, addr, size);
                        return 0xffffffffffffffff;
                    }
                    return d->read(addr, size);
                }
            }
            _log(warning, "Read on unmapped memory, addr = 0x%llx, size = 0x%llx (RISC64_ENOENT)", addr, size);
            return 0xffffffffffffffff;
        }

        inline void write(u64 addr, u64 value, size_t size) {
            for (auto d : devices) {
                u64 base_addr = d->get_base(),
                    end_addr = base_addr + d->get_size();
                if ((addr >= base_addr) && (addr <= end_addr)) {
                    if (!(d->get_access_mode() & device::access_mode::a_w)) {
                        _log(warning, "Invalid write on device %s @ 0x%llx, addr = 0x%llx, size = 0x%llx (RISC64_SIGSEGV)", d->get_name().c_str(), base_addr, addr, size);
                    }
                    return d->write(addr, value, size);
                }
            }
            _log(warning, "Write on unmapped memory, addr = 0x%llx, size = 0x%llx (RISC64_ENOENT)", addr, size);
        }
        

        template <class Device> inline void attach_device(Device& d) {
            devices.push_back(&d);
        }

        template <class Device> inline Device* get_device(u16 hid) {
            for (auto d : devices) {
                if (hid == d->get_hid()) return static_cast<Device*>(d);
            }
            // Device not found, UI warning
            return nullptr;
        }

        void init() {
            devices.reserve(BUS_MAX_DEVICES);
        }
	};
};