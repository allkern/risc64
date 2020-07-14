#pragma once

#include "iterator.hpp"
#include "aliases.hpp"

namespace elf {
    namespace traits {
        class iterable {};
        class writable {
            virtual iterator<u8> begin() const = 0;
            virtual iterator<u8> end() const = 0;
        };
    }
}