#pragma once

#include <functional>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <atomic>
#include <array>

#include "../aliases.hpp"
#include "../device.hpp"
#include "../bus.hpp"

#include "decoder.hpp"

// Macro defining how many CPU threads might be created
#define CPU_THREAD_COUNT 8

// Define this so the control window CPU controls work
#define CPU_STEPPING_ENABLED

// Will probably remove this in the future
#define ALU_OPERATION_COUNT 0xc

// Aliases
#define dest_r this->gpr[exec.dest]
#define operand0_r this->gpr[exec.operand0]
#define operand1_r this->gpr[exec.operand1]
#define dest_c this->exec.dest
#define operand0_c this->exec.operand0
#define operand1_c this->exec.operand1

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
    std::array <std::atomic<u64>, CPU_THREAD_COUNT> tsr;

    class cpu : public device {
    public:
        // Register array type aliases
        typedef std::array <u64, 32> gpr_array_t;
        typedef std::array <float, 32> fpr_array_t;

        // This is so we don't need accessor functions
        friend class control_window;

    private:
        // These are pretty self explanatory
#ifdef CPU_STEPPING_ENABLED
        std::atomic<bool> step, stepping_enabled;
#endif
        // Thread ID
        size_t thread_id = 0;

        // GPRs
        gpr_array_t gpr = { 0ull };

        // Floating Point Registers
        fpr_array_t fpr = { +0.0f };

        // Thread-local register
        std::atomic<u64>* tlr = nullptr;

		// Program Counter
        u64 pc = 0ull;

        // Stack Pointer
		u64 sp = 0ull;

		// Status Register
		u16 sr = 0ull;

        // PC Increment
        size_t pci = 0;

        // This allows the programmer to stop execution of the emulator
        bool is_halted = false;

        // Execution state
        decoder::instruction exec;
        
        // sr = 0000 0000 0000 tncz

        // SR flags
        enum flags {
            zf = 0b0000000000000001, // Zero flag
            cf = 0b0000000000000010, // Carry flag
            nf = 0b0000000000000100, // Negative flag
            tf = 0b0000000000001000  // IRQ (interrupT) flag
        };

        // Flag ops
        inline void set_flags(u16 f) { sr |= f; }
        inline void reset_flags(u16 f) { sr &= (~f); }
        inline bool test_flag(u16 f) { return (sr & f); }

        // Tests the execution condition
        bool is_executed() {
            switch (exec.cond) {
                case decoder::condition::nz: if ( test_flag(flags::zf)) { return false; } break;
                case decoder::condition::nc: if ( test_flag(flags::cf)) { return false; } break;
                case decoder::condition::p : if ( test_flag(flags::nf)) { return false; } break;
                case decoder::condition::z : if (!test_flag(flags::zf)) { return false; } break;
                case decoder::condition::c : if (!test_flag(flags::cf)) { return false; } break;
                case decoder::condition::n : if (!test_flag(flags::nf)) { return false; } break;
                case decoder::condition::nv: return false;
            }
            return true;
        }

        // The ID of an operation is its index on this array
        std::array <std::function<u64(u64&, u64&)>, ALU_OPERATION_COUNT> alu_binary_operation = {
            [] (u64& s0, u64& s1) -> u64 { return s0 + s1; },
            [] (u64& s0, u64& s1) -> u64 { return s0 - s1; },
            [] (u64& s0, u64& s1) -> u64 { return s1 - s0; },
            [] (u64& s0, u64& s1) -> u64 { return s0 * s1; },
            [] (u64& s0, u64& s1) -> u64 { return s0 / s1; },
            [] (u64& s0, u64& s1) -> u64 { return s1 / s0; },
            [] (u64& s0, u64& s1) -> u64 { return s0 % s1; },
            [] (u64& s0, u64& s1) -> u64 { return s0 & s1; },
            [] (u64& s0, u64& s1) -> u64 { return s0 | s1; },
            [] (u64& s0, u64& s1) -> u64 { return s0 ^ s1; },
            [] (u64& s0, u64& s1) -> u64 { return s0 << s1; },
            [] (u64& s0, u64& s1) -> u64 { return s0 >> s1; },
        };

        // Same as alu_binary_operation
        std::array <std::function<u64(u64&)>, 0x4> alu_unary_operation = {
            [] (u64& s) -> u64 { return ~s; },
            [] (u64& s) -> u64 { return s+1; },
            [] (u64& s) -> u64 { return s-1; },
            [] (u64& s) -> u64 { return std::llabs(s); }
        };

        inline u64 mask(u64& v, size_t b) {
            return v & (~((0xffffffffffffffffull * !(b == 64)) << b));
        }

        void apply_flags(u64 res, size_t size, bool sign) {
            size_t bits = decoder::get_operand_sizeof(size) * 8;
            u64 masked = mask(res, bits);
            if (masked > 0) reset_flags(flags::nf);
            if (masked == 0) set_flags(flags::zf); else reset_flags(flags::zf);
            if (res & (0xffffffffffffffffull << bits)) set_flags(flags::cf);
            if (sign) {
                if (masked & (0x80ull << bits - 8)) { set_flags(flags::nf); }
            }
        }

        // ALU operation t_operand overload
        void alu_op(u64& d, u64& s0, u64& s1, std::function<u64(u64&, u64&)>& op, size_t operand_size, bool sign) {
            d = op(s0, s1);
            apply_flags(d, operand_size, sign);
        }

        // ALU operation d_operand overload
        void alu_op(u64& d, u64& s, std::function<u64(u64&, u64&)>& op, size_t operand_size, bool sign) {
            d = op(d, s);
            apply_flags(d, operand_size, sign);
        }

        // ALU operation s_operand_register overload
        void alu_op(u64& d, std::function<u64(u64&)>& op, size_t operand_size, bool sign) {
            d = op(d);
            apply_flags(d, operand_size, sign);
        }


    public:
        // Default constructor
        cpu(size_t thread_id = 0) :
            device("cpu" + std::to_string(thread_id), 0xfffffffffffffffe, 1, thread_id, device::access_mode::a_none),
            tlr(&tsr[thread_id]),
            thread_id(thread_id) {
        #ifdef CPU_STEPPING_ENABLED
                step = true;
                stepping_enabled = true;
        #endif
        };

        // Get Stack Pointer
        u64& get_sp() { return sp; }

        // Get GPRs array
        gpr_array_t& get_gpr_array() { return gpr; }

        // Get FPRs array
        fpr_array_t& get_fpr_array() { return fpr; }
    
        // Get Program Counter
        u64& get_pc() { return pc; }

        // Get Status Register
        u16& get_sr() { return sr; }

        // Get PC Increment
        size_t& get_pci() { return pci; }

        // Get this CPU's thread ID
        size_t& get_thread_id() { return thread_id; }

        // Query whether the CPU is halted or not
        bool& cpu_halted() { return is_halted; }

        // Get a pointer to the execution state struct
        decoder::instruction* get_execution_state() { return &exec; }

        // Read an instruction from the bus and decode it
        void fetch_decode() {
            exec.opcode = bus::read(pc, 8);
            exec.ext64 = bus::read(pc+8, 2);
            pci = decoder::decode(exec);
        }

        // Execute the decoded instruction
        void execute() {
            using namespace decoder;

#ifdef CPU_STEPPING_ENABLED
            if (stepping_enabled) { step = true; }
            while (step) {}
#endif

            bool jump = false;
            if (!is_executed()) { goto end; }

            switch (get_class(exec)) {
                case instruction_type::sys: {
                    switch (get_subclass(exec)) {
                        case instruction_type::s_operand_register: {
                            switch (exec.id) {
                                case 0xfd: { size_t op = decoder::get_operand_sizeof(exec.operand_size); bus::write(sp, dest_r, op); sp += op; } break;
                                case 0xfc: { size_t op = decoder::get_operand_sizeof(exec.operand_size); sp -= op; dest_r = bus::read(sp, op); } break;
                            }
                        } break;
                        case instruction_type::s_operand_const: {
                            switch (exec.id) {
                                case 0xff: { jump = true; pc = exec.target; } break; // fj
                                case 0xfe: { gpr[exec.dest] = exec.target; } break; // lrq
                                case 0xfd: { size_t op = decoder::get_operand_sizeof(exec.operand_size); bus::write(sp, operand0_c, op); sp += op; } break;
                            }
                        } break;
                        case instruction_type::no_operand: {
                            switch (exec.id) {
                                case 0xfe: { is_halted = true; } break; // halt
                            }
                        } break;
                    }
                } break;

                // ALU Instruction Class
                case instruction_type::alu: {
                    switch (get_subclass(exec)) {
                        case instruction_type::t_operand_register_all: {
                            auto func = alu_binary_operation[exec.id % alu_binary_operation.size()];
                            alu_op(gpr[exec.dest], gpr[exec.operand0], gpr[exec.operand1], func, exec.operand_size, (bool)exec.operand_sign);
                        } break;

                        case instruction_type::t_operand_single_const: {
                            auto func = alu_binary_operation[exec.id % alu_binary_operation.size()];
                            u64 c = exec.operand1;
                            alu_op(gpr[exec.dest], gpr[exec.operand0], c, func, exec.operand_size, (bool)exec.operand_sign);
                        } break;

                        case instruction_type::d_operand_register_all: {
                            // Implement cmp and test
                            if (exec.id >= ALU_OPERATION_COUNT) {
                                switch (exec.id) {
                                    case ALU_OPERATION_COUNT: {
                                        apply_flags(gpr[exec.dest] - gpr[exec.operand0], exec.operand_size, (bool)exec.operand_sign);
                                    } break;
                                    case ALU_OPERATION_COUNT + 1: {
                                        apply_flags(gpr[exec.dest] & (1ull << gpr[exec.operand0]), exec.operand_size, (bool)exec.operand_sign);
                                    } break;
                                }
                                break;
                            }
                            auto func = alu_binary_operation[exec.id % alu_binary_operation.size()];
                            alu_op(gpr[exec.dest], gpr[exec.operand0], func, exec.operand_size, (bool)exec.operand_sign);
                        } break;

                        case instruction_type::d_operand_single_const: {
                            // Implement cmp and test
                            if (exec.id >= ALU_OPERATION_COUNT) {
                                switch (exec.id) {
                                    case ALU_OPERATION_COUNT: {
                                        apply_flags(gpr[exec.dest] - operand0_c, exec.operand_size, (bool)exec.operand_sign);
                                    } break;
                                    case ALU_OPERATION_COUNT + 1: {
                                        apply_flags(gpr[exec.dest] & (1ull << operand0_c), exec.operand_size, (bool)exec.operand_sign);
                                    } break;
                                }
                                break;
                            }
                            auto func = alu_binary_operation[exec.id % alu_binary_operation.size()];
                            u64 c = exec.operand0;
                            alu_op(gpr[exec.dest], c, func, exec.operand_size, (bool)exec.operand_sign);
                        } break;
                        case instruction_type::s_operand_register: {
                            auto func = alu_unary_operation[exec.id % alu_unary_operation.size()];
                            alu_op(gpr[exec.dest], func, exec.operand_size, (bool)exec.operand_sign);
                        } break;

                        case instruction_type::s_operand_const: {
                            switch (exec.id) {
                                case 0xe0: { // add %sp, #v;
                                    sp += operand0_c;
                                } break;
                                case 0xe1: { // sub %sp, #v;
                                    sp -= operand0_c;
                                } break;
                            }
                        }
                    }
                } break;

                // LSU Instruction Class
                case instruction_type::lsu: {
                    switch (get_subclass(exec)) {
                        case instruction_type::t_operand_register_all: {
                            switch (exec.id) {
                                case 0x00: { // l{b, w, d, q} %rD, %rS0, %rS1;
                                    dest_r = bus::read(operand0_r + operand1_r, decoder::get_operand_sizeof(exec.operand_size));
                                } break;
                            }
                        } break;

                        case instruction_type::t_operand_single_const: {
                            switch (exec.id) {
                                case 0x00: {
                                    dest_r = bus::read(operand0_r + operand1_c, decoder::get_operand_sizeof(exec.operand_size));
                                } break;
                            }
                        } break;

                        case instruction_type::d_operand_register_all: {
                            switch (exec.id) {
                                case 0x0: { dest_r = bus::read(operand0_r, decoder::get_operand_sizeof(exec.operand_size)); } break;
                                case 0x1: { bus::write(operand0_r, dest_r, decoder::get_operand_sizeof(exec.operand_size)); } break;
                                case 0x2: { dest_r = operand0_r; } break;
                            }
                        } break;

                        case instruction_type::d_operand_single_const: {
                            // lr{b, w, d}
                            // lrq has a special encoding
                            switch (exec.id) {
                                case 0x02: {
                                    dest_r = operand0_c;
                                } break;
                            }
                        } break;

                        case instruction_type::s_operand_const: {
                            switch (exec.id) {
                                case 0xe0: { // lsp #const;
                                    sp = operand0_c;
                                } break;
                            }
                        } break;

                        case instruction_type::s_operand_register: {
                            switch (exec.id) {
                                case 0xe0: {
                                    sp = operand0_r;
                                } break;
                                case 0xd0: { // push %rD;
                                    size_t size = decoder::get_operand_sizeof(exec.operand_size);
                                    bus::write(sp, dest_r, size);
                                    sp -= size;
                                } break;
                                case 0xd1: { // pop %rD;
                                    size_t size = decoder::get_operand_sizeof(exec.operand_size);
                                    sp += size;
                                    dest_r = bus::read(sp, size);
                                } break;
                            }
                        }
                    }
                } break;

                // BNJ Instruction Class
                case instruction_type::bnj: {
                    jump = true;
                    switch (get_subclass(exec)) {
                        case instruction_type::s_operand_register: {
                            switch (exec.id) {
                                // b
                                case 0x0: {
                                    pc += (s32)mask(operand0_r, decoder::get_operand_sizeof(exec.operand_size)*8);
                                } break;
                                // call %rD
                                case 0xfe: {
                                    bus::write(sp, pc+3, 8);
                                    sp -= 8;
                                    pc = dest_r;
                                } break;
                            }
                        }
                        case instruction_type::s_operand_const: {
                            switch (exec.id) {
                                case 0x0: {
                                    u64 v = operand0_c;
                                    pc += (s32)mask(v, decoder::get_operand_sizeof(exec.operand_size)*8);
                                    pc = mask(pc, decoder::get_operand_sizeof(exec.operand_size)*8);
                                } break;
                                // call #const
                                case 0xfe: {
                                    bus::write(sp, pc+3+decoder::get_operand_sizeof(exec.operand_size), 8);
                                    sp -= 8;
                                    pc = operand0_c;
                                } break;
                            }
                        } break;

                        case instruction_type::no_operand: {
                            // ret
                            switch (exec.id) {
                                case 0xff: {
                                    sp += 8;
                                    pc = bus::read(sp, 8);
                                } break;
                            }
                        }
                    }
                } break;
            }

        end:
            if (!jump) pc += pci; else jump = false;
        }
    };
}