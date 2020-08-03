
//#define A64_LOG_FILE
#include "log.hpp"
#include "emulator/machine.hpp"
#include "emulator/control_window.hpp"
#include "emulator/cli.hpp"
#include "emulator/global.hpp"

namespace machine {
    void init(const std::string bios_file) {
        #ifdef __linux__
	        XInitThreads();
            _log(ok, "XInitThreads() call successfull");
        #endif

        // Initialize BIOS
        dev_bios.load_binary(bios_file);
        _log(ok, "Initialized BIOS");

        // Attach devices
        machine::bus::attach_device(dev_proc);
        machine::bus::attach_device(dev_bios);
        machine::bus::attach_device(dev_ioctl);
        _log(ok, "Attached devices to bus");

        // Initialize CPU loop threads
        machine::cpu_thread_sp_array[0] = std::make_shared<sf::Thread>(&cpu_loop, &dev_proc);
        _log(ok, "Initialized CPU loop threads");

#ifdef _WIN32
        dev_ioctl.init_display();
#endif

        machine::cpu_thread_sp_array[0]->launch();
    };
}

machine::control_window cw;

int main(int argc, const char* argv[]) {
    arch64::log::init("main.log");
    
    // Hide the console (win32)
#ifdef _WIN32
    ::ShowWindow(::GetConsoleWindow(), SW_HIDE);
#endif

    cli::init(argc, argv);

    cli::parse();

    machine::init(cli::settings["bios"]);

    cw.start();

    machine::cpu_thread_sp_array[0]->terminate();

    return 0;
}
