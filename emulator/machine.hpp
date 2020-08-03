#pragma once

#include "../emulator/cpu.hpp"
#include "../emulator/cpu_proc.hpp"
#include "../emulator/devices/bios.hpp"
#include "../emulator/devices/memory.hpp"
#include "../emulator/devices/ioctl.hpp"

#include "../log.hpp"

namespace machine {
    typedef std::array<std::shared_ptr<sf::Thread>, CPU_THREAD_COUNT> cpu_thread_array_t;

    machine::ioctl  dev_ioctl(0x2000ull, 1);
    machine::bios   dev_bios ("SimpleBIOS");
    machine::cpu    dev_proc (0);

    cpu_thread_array_t cpu_thread_sp_array;
}
