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

	private:
		// Physical bus base address
		u64 base;

		// Bus allocation size
		u64 size;
		
		// Hardware ID
		u16 hid;

		// A human-readable name for the device
		std::string name;

		// Specify access permissions for the device
		access_mode access;
	
	protected:
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

		virtual u64 translate(u64 addr) { return addr - base; };
		virtual u8 rb(u64) {}
		virtual u16 rw(u64) {}
		virtual u32 rd(u64) {}
		virtual u64 rq(u64) {}
		virtual u8 wb(u64, u8) {}
		virtual u16 ww(u64, u16) {}
		virtual u32 wd(u64, u32) {}
		virtual u64 wq(u64, u64) {}
	};
}