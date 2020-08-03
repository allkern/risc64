#pragma once

#include <array>

#include "../aliases.hpp"
#include "../device.hpp"

namespace machine {
	template <size_t a_size> class memory : public machine::device {
		using device_access = device::access_mode;

	public:
		typedef std::array <machine::u8, a_size> array_t;

	private:
		array_t m = { 0 };

	public:
		memory(u64 mmio_base) :
			device("Main Memory Controller", mmio_base, a_size, 9, device_access::a_all) {};

#ifdef DEBUG
		inline const std::string& get_symbol(u64 addr) override {
			addr -= base;
			if (addr < size) { return "_pbus_main_memory"; }
		};
#endif

		u64 read(u64 addr, size_t size) override {
			u64 q = 0;
            for (int off = size - 1; off >= 0; off--) {
                q |= ((u64)m[addr+off]) << (8 * off);
            }
            return q;
		}

		void write(u64 addr, u64 value, size_t size) override {
			for (int off = size - 1; off >= 0; off--) {
                m[addr+off] = (value & (0xff << (off*8))) >> (off*8);
            }
		}
	};
}