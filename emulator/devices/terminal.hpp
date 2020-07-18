#include "../aliases.hpp"
#include "../device.hpp"

#include <array>

namespace machine {
    class terminal : public device {
        using device_access = device::access_mode;

        u8 registers[6] = { 0 };

    public:
        
    };
}