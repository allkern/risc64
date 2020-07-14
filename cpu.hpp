#include <iostream>
#include <sstream>
#include <cstdint>
#include <atomic>
#include <array>

typedef uint_least8_t  u8;
typedef uint_least16_t u16;
typedef uint_least32_t u32;
typedef uint_least64_t u64;

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

    template <const int thread_id = 0> class cpu {
        // GPRs
        std::array <u64, 32> gpr = { 0x0ull };

        // Floating Point Registers
        std::array <float, 16> fpr = { 0.0f };

        // Thread-local register
        std::atomic<u64>& tlr = machine::tsr[thread_id];

		// Program Counter
        u64 pc = 0x1000ull;

		// Stack Pointer
		u64 sp = 0xfffffffffffffffe;

        enum condition {
            z = 0,	// z: zero/equal (0, 0b000)
            c,		// c: overflow/carry (1, 0b001)
            n,		// n: negative (2, 0b010)
            a,		// a: always (3, 0b011)
            nv,		// n: never (4, 0b100)
            p,		// p/nn: not negative/positive (5, 0b101)
            nc,		// nc: not carry/overflow (6, 0b110)
            nz,		// nz: not zero/equal (7, 0b111)		 
        };

        enum instruction_type {
            alu_3rr, // alu_rdrr: ALU r/m dest, r/m source0, r/m source1
            alu_3rc, // alu_rrcc: ALU r/m dest, r/m source0, c32/m source1
            alu_2r,  // alu_rrcc: ALU r/m dest, r/m source0
            alu_2c,  // alu_rrcc: ALU r/m dest, c32/m source0
        };

		enum operand_size {
			w = 0,
			hw = 1,
			dw = 2,
			qw = 3
		};

    public:
        cpu() = default;
        
        struct instruction {
            u64 	opcode;
            u8		id,
					type 			: 5,
            		cond 			: 3,
            		operand_size 	: 2,
            		memory_operand 	: 3,
					dest			: 5;
			u32		operand0,
					operand1;
            bool 	operand_sign 	: 1;

			// Placeholder disassemble function
            // ccctttttiiiiiiii: c = condition, t = type, i = id
            // b-------b-------
			static std::string disassemble(instruction& i) {
				std::cout << "cond = 0x" << std::hex << (int)i.cond << ", type = 0x" << (int)i.type << ", id = 0x" << (int)i.id
						  << ", operand_size = 0x" << (int)i.operand_size << ", operand_sign = " << (i.operand_sign ? "signed" : "unsigned")
						  << ", memory_operand = 0x" << (int)i.memory_operand << ", dest = 0x" << (int)i.dest << ", operand0 = 0x" << (int)i.operand0
						  << ", operand1 = 0x" << (int)i.operand1 << std::endl;
				std::stringstream ss;
				switch (i.type) {
					case instruction_type::alu_3rr: {
						switch (i.id) {
							case 0x00: ss << "add"; break;
							case 0x01: ss << "sub"; break;
							case 0x02: ss << "mod"; break;
						}


						switch (i.operand_size) {
							case operand_size::hw: ss << 'h'; break;
							case operand_size::dw: ss << 'd'; break;
							case operand_size::qw: ss << 'q'; break;
						}

						switch (i.cond) {
							case condition::z: ss << "z"; break;
							case condition::nz: ss << "nz"; break;
							case condition::c: ss << "c"; break;
							case condition::nc: ss << "nc"; break;
							case condition::n: ss << "n"; break;
							case condition::p: ss << "p"; break;
							case condition::nv: ss << "nv"; break;
						}

						if (i.operand_sign) { ss << 's'; }

						ss << " " << ((i.memory_operand & 0b100) ? "*" : "") << "r" << std::dec << (int)i.dest << ", ";
						ss << ((i.memory_operand & 0b010) ? "*" : "") << "r" << (int)i.operand0 << ", ";
						ss << ((i.memory_operand & 0b001) ? "*" : "") << "r" << (int)i.operand1 << ";";
					}
				}
				return ss.str();
			}

            static void decode(instruction& i) {
                i.cond = i.opcode & 0x7;
                i.type = (i.opcode & 0xf8) >> 3;
                i.id = (i.opcode & 0xff00) >> 8;
				switch (i.type) {
                    // alu_3_rr: m = memory operand, d = destination, o = operand0, O = operand1, - = padding
                    // ccctttttiiiiiiiisSSmmmdddddoooooOOOOO---
                    // b-------b-------b-------b-------b-------b-------b-------b-------
                    case instruction_type::alu_3rr: {
                        i.operand_sign = (bool)((i.opcode & (0x1ul << 0x10)) >> 0x10);
						i.operand_size = (i.opcode & (0x6ul << 0x10)) >> 0x11;
						i.memory_operand = (i.opcode & (0x38ul << 0x10)) >> 0x13;
						i.dest = (i.opcode & (0x7cul << 0x14)) >> 0x16;
						i.operand0 = (i.opcode & (0xf8ul << 0x18)) >> 0x19;
						i.operand1 = (i.opcode & (0x1full << 0x20)) >> 0x20;
                    }

					// alu_3_rc: m = memory operand, d = destination, o = operand0, O = operand1
                    // ccctttttiiiiiiiisSSmmmdddddoooooOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
                    // b-------b-------b-------b-------b-------b-------b-------b-------
					case instruction_type::alu_3rc: {
                        i.operand_sign = (bool)((i.opcode & (0x1ul << 0x10)) >> 0x10);
						i.operand_size = (i.opcode & (0x6ul << 0x10)) >> 0x11;
						i.memory_operand = (i.opcode & (0x38ul << 0x10)) >> 0x13;
						i.dest = (i.opcode & (0x7cul << 0x14)) >> 0x16;
						i.operand0 = (i.opcode & (0xf8ul << 0x18)) >> 0x19;
						i.operand1 = (i.opcode & (0xffffffffull << 0x20)) >> 0x20;
                    }
                }
            }

            instruction() = default;
			instruction(u64 opcode) : opcode(opcode) {}
        } exec;
    };
}