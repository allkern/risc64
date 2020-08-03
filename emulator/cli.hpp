#include <vector>
#include <string>
#include <unordered_map>

#include "../log.hpp"

namespace cli {
    std::vector <std::string> cli;
    std::unordered_map <std::string, std::string> settings;

    namespace detail {
        static inline std::string ltrim(std::string s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
                return !std::isspace(ch);
            }));
            return s; 
        }

        static inline std::string rtrim(std::string s) {
            s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
                return !std::isspace(ch);
            }).base(), s.end());
            return s;
        }

        static inline std::string trim(std::string s) {
            return ltrim(rtrim(s));
        }
    }

    void init(size_t argc, const char* argv[]) {
        cli.reserve(argc-1);

        for (int i = 1; i < argc; i++) {
            cli.push_back(std::string(argv[i]));
        }
    }

    void parse() {
        int i = 1;
        for (std::string& s : cli) {
            detail::trim(s);
            std::string key   = detail::rtrim(s.substr(0, s.find_first_of('=')));
            std::string value = detail::ltrim(s.substr(s.find_first_of('=')+1));

            size_t q_opening = value.find_first_of('\"');
            size_t q_closing = value.find_first_of('\"', q_opening+1);
            
            if (q_opening != std::string::npos) {
                if (q_closing == std::string::npos) {
                    _log(error, "Error parsing command-line argument %i (missing closing quotes)", i);
                }
                value = value.substr(1);
            }

            settings.insert({key, value});
        }
    }
}