#include <iostream>
#include <memory>
#include <fstream>
#include <sstream>

#include <array>
#include <regex>

struct instruction {
    const std::string 
        mnemonic,
        condition,
        operand_size,
        operand_sign;
    const std::array <std::string, 3> operands;

    instruction(
        const std::string mnemonic,
        const std::string condition,
        const std::string operand_size,
        const std::string operand_sign,
        const std::array <std::string, 3> operands) :
        mnemonic(mnemonic),
        condition(condition),
        operand_size(operand_size),
        operand_sign(operand_sign),
        operands(operands) {}
};

class assembler {
    std::unique_ptr <std::istream> stream;
    int l;

public:
    // Constructors
    assembler() = default;
    assembler(std::ifstream&& file)       : stream(&file)   {};
    assembler(std::stringstream&& string) : stream(&string) {};
    assembler(std::istream&& stream)      : stream(&stream) {};

    std::string lex_instruction() {
        std::string ins;
        while(std::isspace(l)) { l = stream->get(); }
        while(!std::isspace(l)) {
            if (std::isalpha(l)) {
                ins += l;
            } else {
                if (l == ';') return ins;
                std::cout << "Unknown token\n";
                break;
            }
            l = stream->get();
        }
        return ins;
    }

    std::pair <std::string, bool> lex_operand() {
        std::string operand;
        bool last = false;
        while(std::isspace(l)) { l = stream->get(); }
        while(!(l == ',' || l == ';')) {
            operand += l;
            l = stream->get();
        }
        if (l == ';') { last = true; }
        l = stream->get();
        return { operand, last };
    }

    std::unique_ptr <std::vector <instruction>> parse() {
        std::vector <instruction> code;
        std::array <std::string, 3> operands;
        size_t op_counter = 0;
        l = stream->get();
        while (!stream->eof()) {
            std::string i = lex_instruction();
            std::cout << i << std::endl;
            std::pair <std::string, bool> operand = { "", false };
            while (!operand.second) {
                operand = lex_operand();
                if (operand.first.size()) { operands[op_counter++] = operand.first; }
                if (operand.second) break;
                if (op_counter == 3) {
                    std::cout << "Invalid operand count\n";
                    return nullptr;
                }
            }
            for (std::string o : operands) {
                std::cout << o << std::endl;
            }
        }
        return nullptr;
    }

    ~assembler() {
        stream.release()->clear();
    }
};

int main() {
    std::ifstream my_file("test.lasm");
    assembler a(std::move(my_file));
    a.parse();
    return 0;
}