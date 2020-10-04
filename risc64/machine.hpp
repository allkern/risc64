#pragma once

#define CPU_ENABLE_STEPPING

#include "../risc64/cpu/cpu.hpp"
#include "../risc64/cpu/cpu_proc.hpp"
#include "../risc64/devices/bios.hpp"
#include "../risc64/devices/memory.hpp"
#include "../risc64/devices/ioctl.hpp"

#include "log.hpp"

namespace machine {
    typedef std::array<std::shared_ptr<sf::Thread>, CPU_THREAD_COUNT> cpu_thread_array_t;
    typedef machine::memory<0xffff> dev_memory_t;

    // Devices
    machine::ioctl  dev_ioctl(0x2000ull, 1);
    machine::bios   dev_bios ("SimpleBIOS");
    machine::cpu    dev_proc (0);
    dev_memory_t    dev_mmem(0x10000ull);

    // Pointers to CPU thread instances
    cpu_thread_array_t cpu_thread_sp_array;
}
