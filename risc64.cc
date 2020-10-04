#include "risc64/log.hpp"
#include "risc64/machine.hpp"
#include "risc64/control_window.hpp"
#include "risc64/cli.hpp"
#include "risc64/global.hpp"

namespace machine {
    void init(const std::string bios_file) {
        #ifdef __linux__
	        if (!XInitThreads()) {
                _log(error, "XInitThreads() call unsuccessful");
                std::exit(0);
            }
        #endif

        // Initialize BIOS
        dev_bios.load_binary(bios_file);
        _log(ok, "Initialized BIOS");

        // Attach devices
        machine::bus::attach_device(dev_proc);
        machine::bus::attach_device(dev_bios);
        machine::bus::attach_device(dev_ioctl);
        machine::bus::attach_device(dev_mmem);
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
    // Hide the console (win32)
#ifdef _WIN32
    ::ShowWindow(::GetConsoleWindow(), SW_HIDE);
#endif

    cli::init(argc, argv);

    cli::parse();

    _log::log::init("risc64", cli::settings.contains("log") ? cli::settings["log"] : "main.log");

    machine::init(cli::settings["bios"]);

    cw.start();

    machine::cpu_thread_sp_array[0]->terminate();

    return 0;
}
