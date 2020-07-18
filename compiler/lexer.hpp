#include <iostream>
#include <optional>
#include <fstream>
#include <sstream>
#include <memory>
#include <cctype>
#include <regex>

/* Language structure:
    Numbers: (0123456789)*
    Identifiers: alpha alnum-_*
    Other tokens: []()+-/*
*/

class lexer {
public:
    enum tokens {
        k_unknown     = 0,
        k_instruction = 1,
        k_operand     = 1,
        k_comma       = 4
    };

private:
    std::unique_ptr <std::istream> stream; // Wrapper around T*
    int_least64_t lexed_number;
    std::string   lexed_string,
                  lexed_id;
    char current_char = ' ';

    // This function extracts the next character in the stream
    inline int get_next_char() { return stream.get()->get(); }

    // This function skips whitespace characters until a non-whitespace character is found
    inline void ignore_whitespace() { while (std::isspace(current_char)) { current_char = get_next_char(); } }

    // This function returns a number
    std::optional <int> lex_number() {
    }

    // This 
    std::optional <int> lex_identifier() {
        std::string id_str = "";
        if (!(std::isalpha(current_char) || current_char == '_')) {
            return {};
        }

        while (std::isalnum(current_char) || current_char == '_' || current_char == '-') {
            id_str += current_char;
            current_char = get_next_char();
        }

        lexed_id = id_str;

        if (lexed_id == "var") return tokens::k_var;

        return tokens::k_identifier;
    }

    // This function lexes all our operands
    std::optional <int> lex_operator() {
        int token;
        switch (current_char) {
            case '+': token = tokens::k_binary_operator; break;
            case '-': token = tokens::k_binary_operator; break;
            case '*': token = tokens::k_binary_operator; break;
            case '/': token = tokens::k_binary_operator; break;
            case '%': token = tokens::k_binary_operator; break;
            case '&': token = tokens::k_binary_operator; break;
            case '|': token = tokens::k_binary_operator; break;
            case '^': token = tokens::k_binary_operator; break;
            case '!': token = tokens::k_unary_operator; break;
            case '=': token = tokens::k_assignment_operator; break;
            default: return {};
        }
        current_char = get_next_char();
        return token;
    }

    // This function lexes structural tokens
    std::optional <int> lex_structural() {
        int token;
        switch (current_char) {
            case '(': token = tokens::k_opening_parent; break;
            case ')': token = tokens::k_closing_parent; break;
            case '[': token = tokens::k_opening_bracket; break;
            case ']': token = tokens::k_closing_bracket; break;
            case ';': token = tokens::k_semicolon; break;
            case ':': token = tokens::k_colon; break;
            case ',': token = tokens::k_comma; break;
            default: return {};
        }
        current_char = get_next_char();
        return token;
    }

public:
    // Constructors
    lexer() = default;

    // Construct from a file
    lexer(std::ifstream&& file) : stream(&file) {};

    // Construct from a string
    lexer(std::stringstream&& string) : stream(&string) {};

    // Construct from an arbitrary stream
    lexer(std::istream&& stream) : stream(&stream) {};

#define INIT_OPT_HANDLER std::optional <int> t;
#define HANDLE_OPT(k) t = lex_##k(); if (t) return t.value();

    int get_next_token() {
        if (stream.get()->eof()) return tokens::k_eof;

        ignore_whitespace();

        INIT_OPT_HANDLER

        HANDLE_OPT(structural);
        HANDLE_OPT(operator);
        HANDLE_OPT(identifier);
        HANDLE_OPT(number);

        return tokens::k_unknown;
    }

#undef INIT_OPT_HANDLER
#undef HANDLE_OPT

    ~lexer() {
        stream.release()->clear();
    }
};