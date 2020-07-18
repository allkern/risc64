#include <functional>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <atomic>
#include <array>

#include "aliases.hpp"
#include "decoder.hpp"
#include "device.hpp"
#include "bus.hpp"

// Macro defining how many CPU threads might be created
#define CPU_THREAD_COUNT 8

namespace machine {
	// Debug only
    template <class T> std::string bin(T v) {
        std::string s;
        for (int b = (sizeof(T) * 8) - 1; b >= 0; b--) {
            s += (v & (1 << b)) ? '1' : '0';
        }
        return s;
    }

    // Thread-safe registers
    std::array <std::atomic<u64>, CPU_THREAD_COUNT> tsr = { 0 };

    template <const int thread_id = 0> class cpu : public device {
        // GPRs
        std::array <u64, 32> gpr = { 0ull };

        // Floating Point Registers
        std::array <float, 16> fpr = { +0.0f };

        // Thread-local register
        std::atomic<u64>& tlr = machine::tsr[thread_id];

		// Program Counter
        u64 pc = 0ull;

		// Status Register
		u16 sr = 0ull;

        // PC Increment
        size_t pci = 0;
        
        // sr = 00cz 0000 0000 00cz
        //

        enum flags {
            zf = 0b0000000000000001, // Zero flag
            cf = 0b0000000000000010, // Carry flag
            nf = 0b0000000000000100, // Negative flag
            tf = 0b0000000000001000  // IRQ (interrupT) flag
        };

        void set_flags(u16 f) { sr |= f; }
        void reset_flags(u16 f) { sr &= (~f); }
        bool test_flag(u16 f) { return (sr & f); }

        bool is_executed() {
            switch (exec.cond) {
                case decoder::condition::nz: if ( test_flag(flags::zf)) return false;
                case decoder::condition::nc: if ( test_flag(flags::cf)) return false;
                case decoder::condition::p : if ( test_flag(flags::nf)) return false;
                case decoder::condition::z : if (!test_flag(flags::zf)) return false;
                case decoder::condition::c : if (!test_flag(flags::cf)) return false;
                case decoder::condition::n : if (!test_flag(flags::nf)) return false;
                case decoder::condition::nv: return false;
            }
            return true;
        }

        struct alu {
            std::array <std::function<u64(u64&, u64&)>, 0x10> operation = {
                [] (u64& s0, u64& s1) -> u64 { return s0 + s1; },
                [] (u64& s0, u64& s1) -> u64 { return s0 - s1; },
                [] (u64& s0, u64& s1) -> u64 { return s0 * s1; },
                [] (u64& s0, u64& s1) -> u64 { return s0 / s1; },
                [] (u64& s0, u64& s1) -> u64 { return s0 & s1; },
                [] (u64& s0, u64& s1) -> u64 { return s0 | s1; },
                [] (u64& s0, u64& s1) -> u64 { return s0 ^ s1; },
                [] (u64& s0, u64& s1) -> u64 { return ~s0; },
            };

            static void op_u3(u64& d, u64& s0, u64& s1, std::function<u64(u64&, u64&)> op, size_t operand_size) {
                u64 res = op(s0, s1);
                if (res > 0) reset_flags(flags::nf);
                if (res == 0) { set_flags(flags::zf); };
                if (res & (0xffffffffffffffffull << operand_size)) set_flags(flags::cf);
                if (sign) { if (res < 0) { set_flags(flags::nf); }; }
                if (res&&)
                d = (operand_t)res;
            }
            static void op_s3(u64& d, u64& s0, u64& s1, std::function<u64(u64&, u64&)> op) {
                s64 res = (s64)op(s0, s1);
                if (res > 0) reset_flags(flags::nf);
                if (res == 0) { set_flags(flags::zf); };
                if (res < 0) { set_flags(flags::nf); };
                if constexpr (std::is_same<operand_t, u8 >::value) { if (res & 0x80              ) { set_flags(flags::nf); }; if (res & 0xff00            ) set_flags(flags::cf); }
                if constexpr (std::is_same<operand_t, u16>::value) { if (res & 0x8000            ) { set_flags(flags::nf); }; if (res & 0xffff0000        ) set_flags(flags::cf); }
                if constexpr (std::is_same<operand_t, u32>::value) { if (res & 0x80000000        ) { set_flags(flags::nf); }; if (res & 0xffffffff00000000) set_flags(flags::cf); }
                if constexpr (std::is_same<operand_t, u64>::value) { if (res & 0x8000000000000000) { set_flags(flags::nf); } }
                d = (operand_t)res;
            }
        };

        decoder::instruction exec;

    public:
        cpu() : device("arch64_cpu" + std::to_string(thread_id), 0xfffffffffffffffe, 1, 1, device::access_mode::a_none) {};
        
        u64& get_register(size_t w) { return gpr[w]; }
        u64& get_pc() { return pc; }
        

        void fetch_decode() {
            exec.opcode = bus::rq(pc);
            exec.extj = bus::rw(pc + 8);
            pci = decoder::decode(exec);
        }


        void execute() {
            using namespace decoder;

            bool jump = false;
            if (!is_executed()) { goto end; }

            switch (get_class(exec)) {
                case instruction_type::sys: {
                    switch (get_subclass(exec)) {
                        case instruction_type::s_operand_const: {
                            if (exec.id & 0x01) {
                                if (exec.id & 0x02) {
                                    switch (exec.id) {
                                        // Special case: farj const64
                                        // ccc1111011111111aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                                        // b-------b-------b-------b-------b-------b-------b-------b-------b-------b------- 10 bytes
                                        case 0xff: {
                                            jump = true;
                                            pc = exec.target;
                                        }
                                    }
                                } else {
                                    gpr[exec.dest] = exec.target;
                                }
                            }
                        }
                    }
                }
                case instruction_type::alu: {
                    switch (get_subclass(exec)) {
                        case instruction_type::t_operand_register_all: {
                            auto func = alu::operation[((int)exec.operand_sign * 8) + exec.id];
                            if (exec.operand_sign) {
                                switch (exec.operand_size) {
                                    case operand_size::hw: alu::op_s3<u8 >(gpr[exec.dest], gpr[exec.operand0], gpr[exec.operand1], func); break;
                                    case operand_size::w : alu::op_s3<u16>(gpr[exec.dest], gpr[exec.operand0], gpr[exec.operand1], func); break;
                                    case operand_size::dw: alu::op_s3<u32>(gpr[exec.dest], gpr[exec.operand0], gpr[exec.operand1], func); break;
                                    case operand_size::qw: alu::op_s3<u64>(gpr[exec.dest], gpr[exec.operand0], gpr[exec.operand1], func); break;
                                }
                            }
                            switch (exec.operand_size) {
                                case operand_size::hw: alu::op_u3<u8 >(gpr[exec.dest], gpr[exec.operand0], gpr[exec.operand1], func); break;
                                case operand_size::w : alu::op_u3<u16>(gpr[exec.dest], gpr[exec.operand0], gpr[exec.operand1], func); break;
                                case operand_size::dw: alu::op_u3<u32>(gpr[exec.dest], gpr[exec.operand0], gpr[exec.operand1], func); break;
                                case operand_size::qw: alu::op_u3<u64>(gpr[exec.dest], gpr[exec.operand0], gpr[exec.operand1], func); break;
                            }
                        }
                    }
                }
            }

            end:
            if (!jump) pc += pci;
        }
    };
}