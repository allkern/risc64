#include <iostream>
#include <memory>

class preprocessor {
public:
    enum tokens {
        point   = -1,
        section = -2,
        import  = -3,
        base    = -4
    };
private:
    std::unique_ptr <std::istream> stream;
    
};