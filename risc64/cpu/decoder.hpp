#pragma once

#include "../aliases.hpp"

namespace machine {
    namespace decoder {
        // Execution conditions
        enum condition {
            z = 0,  // z: zero/equal (0, 0b000)
            c,      // c: overflow/carry (1, 0b001)
            n,      // n: negative (2, 0b010)
            a,      // a: always (3, 0b011)
            nv,     // nv: never (4, 0b100)
            p,      // p/nn: not negative/positive (5, 0b101)
            nc,     // nc: not carry/overflow (6, 0b110)
            nz,     // nz: not zero/equal (7, 0b111)         
        };

        // Instruction classes and subclasses
        enum instruction_type {
            alu = 0b00,                           // ALU (Arithmetic-Logic Unit) instruction class
            lsu = 0b01,                           // LSU (Load-Store Unit) instruction class
            bnj = 0b10,                           // BNJ (Branches aNd Jumps) instruction class
            sys = 0b11,                           // SYS (SYStem) instruction class
            t_operand_register_all     = 0b00000, // <ins> <r0> <r1> <r2>
            t_operand_single_const     = 0b00100, // <ins> <r0> <r1> <const>
            d_operand_register_all     = 0b01000, // <ins> <r0> <r1>
            d_operand_single_const     = 0b01100, // <ins> <r0> <const>
            d_operand_double_const     = 0b10000, // <ins> <const> <const>
            s_operand_register         = 0b10100, // <ins> <r0>
            s_operand_const            = 0b11000, // <ins> <const>
            no_operand                = 0b11100,  // <ins>
        };

        // Operand sizes
        enum operand_size {
            hw = 0b00,  // 8-bit halfword (byte)
            w  = 0b01,  // 16-bit word
            dw = 0b10,  // 32-bit doubleword
            qw = 0b11   // 64-bit quadword
        };

        // Models an instruction
        struct instruction {
            u64     opcode = 0, target = 0;
            u8        id = 0,
            #ifdef _WIN32
                    type             : 5,
                    cond             : 3,
                    operand_size     : 2,
                    dest             : 5;
            #else
                    type             : 5,
                    cond             : 3,
                    operand_size     : 2,
                    dest             : 5;
            #endif
            u16     ext64 = 0;
            u32     operand0 = 0,
                    operand1 = 0,
                    ext = 0;
            bool    operand_sign = 0;

            // Reset execution state
            inline void reset() {
                opcode          = 0;
                target          = 0;
                id              = 0;
                type            = 0;
                cond            = 0;
                operand_size    = 0;
                dest            = 0;
                ext64           = 0;
                ext             = 0;
                operand_sign    = false;
            }

            instruction() = default;
            instruction(u64 opcode, u16 ext64 = 0x0) : opcode(opcode), ext64(ext64) {}
        };

        // Map an operand_size to its actual size in bytes
        static inline constexpr size_t get_operand_sizeof(u8 s) {
            switch (s) {
                case 0: return 1;
                case 1: return 2;
                case 2: return 4;
                case 3: return 8;
                default: return 0;
            }
        }

        // Decode subclass
        static inline u8 get_subclass(instruction& i) {
            return i.type & 0x1c;
        }

        // Decode class
        static inline u8 get_class(instruction& i) {
            return i.type & 0x3;
        }

        // Decode special-case instructions that have a 64-bit immediate as operand
        static inline std::size_t decode_imm64_instructions(instruction& i) {
            if (get_class(i) == instruction_type::sys) {
                switch (get_subclass(i)) {
                    case instruction_type::s_operand_const: {
                        switch (i.id) {
                            case 0xff: { // fj
                                i.target = ((i.opcode & 0xffffffffffff0000ull) >> 0x10) | ((u64)i.ext64 << 0x30);
                                return 10;
                            }
                            case 0xfe: { // lrq
                                i.dest = (i.id & 0xf8) >> 3;
                                i.target = ((i.opcode & 0xffffffffffff0000ull) >> 0x10) | ((u64)i.ext64 << 0x30);
                                return 10;
                            }
                        }
                    } break;
                    default: return 0; break;
                }
            }
            return 0;
        }

        // Decode class/subclass-agnostic values
        static inline void decode_common(instruction& i) {
            i.cond = i.opcode & 0x7;
            i.type = (i.opcode & 0xf8) >> 3;
            i.id = (i.opcode & 0xff00) >> 8;
            i.operand_sign = (bool)((i.opcode & (0x1ul << 0x10)) >> 0x10);
            i.operand_size = (i.opcode & (0x6ul << 0x10)) >> 0x11;
            i.dest = (i.opcode & (0x1ful << 0x13)) >> 0x13;

            #ifdef DEBUG
            std::cout
                << "cond = 0x" << std::hex << (unsigned int)i.cond
                << ", type = 0x" << std::hex << (unsigned int)i.type
                << ", id = 0x" << std::hex << (unsigned int)i.id
                << ", operand_sign = 0x" << std::hex << (unsigned int)i.operand_sign
                << ", operand_size = 0x" << std::hex << (unsigned int)i.operand_size
                << ", dest = 0x" << std::hex << (unsigned int)i.dest << std::endl;
            #endif
        }

        // Decode an instruction, returns instruction size
        static std::size_t decode(instruction& i) {
            decode_common(i);
            size_t imm64_ins_decoded = decode_imm64_instructions(i);
            if (imm64_ins_decoded) return imm64_ins_decoded;

            switch (get_subclass(i)) {
                case instruction_type::t_operand_register_all: {
                    // ccctttttiiiiiiiisSSdddddoooooOOOOOeeeeee
                    // b-------b-------b-------b-------b------- 5 bytes
                    i.operand0 = (i.opcode & (0x1ful << 0x18)) >> 0x18;
                    i.operand1 = (i.opcode & (0x1full << 0x1d)) >> 0x1d;
                    i.ext = (i.opcode & (0x3full << 0x22)) >> 0x22;
                    return 5;
                } break;

                case instruction_type::t_operand_single_const: {
                    std::size_t isz = 8, el = 0x3d;
                    u64 mask = 0xffffffff;
                    // operand_size >= dw
                    // ccctttttiiiiiiiisSSdddddoooooOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOeee
                    // b-------b-------b-------b-------b-------b-------b-------b------- 8 bytes
                    // operand_size = w
                    // ccctttttiiiiiiiisSSdddddoooooOOOOOOOOOOOOOOOOeee
                    // b-------b-------b-------b-------b-------b------- 6 bytes
                    // operand_size = hw
                    // ccctttttiiiiiiiisSSdddddoooooOOOOOOOOeee
                    // b-------b-------b-------b-------b------- 5 bytes
                    switch (i.operand_size) {
                        case operand_size::w: {
                            mask >>= 16;
                            isz = 6;
                            el = 0x2d;
                        } break;
                        case operand_size::hw: {
                            mask >>= 24;
                            isz = 5;
                            el = 0x25;
                        } break;
                    }
                    i.operand0 = (i.opcode & (0x1ful << 0x18)) >> 0x18;
                    i.operand1 = (i.opcode & (mask << 0x1d)) >> 0x1d;
                    i.ext = (i.opcode & (0x7ull << el)) >> el;
                    return isz;
                } break;

                case instruction_type::d_operand_register_all: {
                    // ccctttttiiiiiiiisSSdddddoooooeee
                    // b-------b-------b-------b------- 4 bytes
                    i.operand0 = (i.opcode & (0x1ful << 0x18)) >> 0x18;
                    i.ext = (i.opcode & (0x7ul << 0x1d)) >> 0x1d;
                    return 4;
                } break;
                
                case instruction_type::d_operand_single_const: {
                    std::size_t isz = 7;
                    u64 mask = 0xffffffff;
                    // operand_size >= dw
                    // ccctttttiiiiiiiisSSdddddoooooooooooooooooooooooooooooooo
                    // b-------b-------b-------b-------b-------b-------b------- 7 bytes
                    // operand_size = w
                    // ccctttttiiiiiiiisSSdddddoooooooooooooooo
                    // b-------b-------b-------b-------b------- 5 bytes
                    // operand_size = hw
                    // ccctttttiiiiiiiisSSdddddoooooooo
                    // oooooooodddddSSsiiiiiiiitttttccc
                    // b-------b-------b-------b------- 4 bytes
                    switch (i.operand_size) {
                        case operand_size::w: {
                            mask >>= 16;
                            isz = 5;
                        } break;
                        case operand_size::hw: {
                            mask >>= 24;
                            isz = 4;
                        } break;
                    }
                    i.operand0 = (i.opcode & (mask << 0x18)) >> 0x18;
                    return isz;
                } break;

                case instruction_type::s_operand_register: {
                    // ccctttttiiiiiiiisSSooooo
                    // b-------b-------b------- 3 bytes
                    return 3;
                } break;

                case instruction_type::s_operand_const: {
                    std::size_t isz = 7, el = 0x33;
                    u64 mask = 0xffffffff;
                    switch (i.operand_size) {
                        case operand_size::w: {
                            mask >>= 16;
                            isz = 5;
                            el = 0x23;
                        } break;
                        case operand_size::hw: {
                            mask >>= 24;
                            isz = 4;
                            el = 0x1b;
                        } break;
                    }
                    // ccctttttiiiiiiiisSSooooooooooooooooooooooooooooooooeeeee
                    // b-------b-------b-------b-------b-------b-------b------- 7 bytes
                    // ccctttttiiiiiiiisSSooooooooooooooooeeeee
                    // b-------b-------b-------b-------b------- 5 bytes
                    // ccctttttiiiiiiiisSSooooooooeeeee
                    // b-------b-------b-------b------- 4 bytes
                    i.operand0 = (i.opcode & (mask << 0x13)) >> 0x13;
                    i.ext = (i.opcode & (0x1ful << el)) >> el;
                    return isz;
                } break;

                case instruction_type::no_operand: {
                    return 2;
                }
            }
            return 0;
        }
    };
};