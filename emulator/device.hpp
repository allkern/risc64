#pragma once

#include <utility>
#include <string>

#include "aliases.hpp"

namespace machine {
	// Hardware device model class
	class device {
	public:
		// Hardware device access mode/permissions
		enum access_mode {
			a_none,					// No access mode
			a_r = 0b0001,			// Read-only access mode
			a_w = 0b0010,			// Write-only access mode
			a_x = 0b0100,			// Execute-only access mode
			a_rw = a_r | a_w,		// Read-write access mode
			a_rx = a_r | a_x,		// Read-execute access mode
			a_wx = a_w | a_x,		// Write-execute access mode
			a_all = a_r | a_w | a_x	// All access permissions
		};

	protected:
		// Physical bus base address
		u64 base = 0;

		// Bus allocation size
		u64 size = 0;
		
		// Hardware ID
		u16 hid  = 0;

		// A human-readable name for the device
		std::string name = "";

		// Specify access permissions for the device
		access_mode access = a_none;
	
		device() = default;
		device(
			std::string name,
			u64 base,
			u64 size,
			u16 hid,
			access_mode access = access_mode::a_r) : name(name), base(base), size(size), hid(hid), access(access) {}

	public:
		std::string get_name() const { return name; }
		u64 get_base() const { return base; }
		u64 get_size() const { return size; }
		u64 get_hid() const { return hid; }
		u8 get_access_mode() const { return access; }

#ifdef DEBUG	
		virtual const std::string& get_symbol(u64) { return ""; };
#endif
		virtual u64 translate(u64 addr) { return addr - base; };
		virtual u64 read(u64, size_t) { return 0xffffffffffffffffull; };
		virtual void write(u64, u64, size_t) {};
	};
}