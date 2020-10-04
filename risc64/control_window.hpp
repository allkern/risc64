#pragma once

#include <vector>
#include <string>

#define LGW_OPTIMIZE
//#define LGW_ENABLE_MUTEXES
#include "lgw/threaded_window.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include "imgui/imgui_internal.h"

#include "cpu/cpu.hpp"
#include "devices/bios.hpp"
#include "devices/ioctl.hpp"
#include "bus.hpp"
#include "machine.hpp"
#include "memory_editor.hpp"

#include "utility.hpp"

namespace machine {
    class control_window : public lgw::threaded_window {
        sf::Clock delta;

        machine::dev_memory_t* memory = nullptr;
        machine::cpu* cpu     = nullptr;
        machine::bios* bios    = nullptr;
        machine::ioctl* ioctl   = nullptr;

        std::vector <std::string> device_names;

        inline void cpu_menu() {
            std::string file = "";
            using namespace ImGui;
            if (BeginMenuBar()) {
                if (BeginMenu("File")) {
                    #ifdef _WIN32
                        if (MenuItem("Dump CPU state (Windows only)")) {
                            file = utility::open_file_save_dialog("Save CPU Dump", "arch64 Dump File\x0", "dmp");
                        }
                    #endif
                    ImGui::EndMenu();
                }
                EndMenuBar();
            };
        }

        inline void cpu_control_tab() {
            using namespace ImGui;
            SetNextItemOpen(true);
            if (TreeNode("Power")) {
                Separator();
                Columns(2, "cpu_ioctl_power", true);
                Text("There will be CPU power controls here!");
                NextColumn();
#ifndef _WIN32
                if (Button("Turn on display")) {
                    machine::dev_ioctl.init_display();
                } SameLine();
#endif
                //if (Button("Turn off display")) {
                //    machine::dev_ioctl.on_close();
                //}
                Columns(1);
                Separator();
                TreePop();
            }

            SetNextItemOpen(true);
            if (TreeNode("Control")) {
                Separator();
                if (Button("Run")) {
                    cpu->stepping_enabled = false;
                    cpu->step = false;
                } SameLine();
                if (Button("Step")) {
                    if (!cpu->stepping_enabled) {
                        cpu->stepping_enabled = true;
                        cpu->step = true;
                    }
                    cpu->step = false;
                }
                TreePop();
            }
        }

        inline void cpu_registers_tab() {
            using namespace ImGui;
            SetNextItemOpen(true);
            bool b = false;
            if (TreeNode("Registers")) {
                char label[32], inp[32];
                Separator();
                Text("Main:");
                Columns(3, "main_table", true);
                Separator();
                sprintf(label, "pc: 0x%x", cpu->pc); if (Selectable(label)) {}; NextColumn();
                sprintf(label, "sp: 0x%x", cpu->sp); if (Selectable(label)) {}; NextColumn();
                sprintf(label, "pci: 0x%x (%i)", cpu->pci, cpu->pci); if (Selectable(label)) {};
                Columns(1);
                Separator();
                Text("GPRs:");
                Columns(4, "gpr_table", true);
                Separator();
                for (int r = 0; r < 32; r++) {
                    sprintf(label, "r%-2i: 0x%x", r, cpu->gpr[r]);
                    if (Selectable(label)) {}
                    NextColumn();
                }
                b = false;
                Columns(1);
                Separator();
                Text("FPRs:");
                Columns(4, "fpr_table", true);
                Separator();
                for (int r = 0; r < 32; r++) {
                    sprintf(label, "f%-2i: %+f", r, cpu->fpr[r]);
                    if (Selectable(label)) {}
                    NextColumn();
                }
                Columns(1);
                Separator();
                TreePop();
            }
        }

        inline void cpu_panel() {
            enum class cpu_panel_tab {
                t_control,
                t_registers,
            } selected_tab = cpu_panel_tab::t_registers;

            using namespace ImGui;

            SetNextWindowPos(ImVec2(0, 0));
            SetNextWindowSize(ImVec2(750, 600));

            std::ostringstream ss;

            char title[32];
            sprintf(title, "CPU (thread %lld)", cpu->thread_id);
            Begin(title, NULL,
                ImGuiWindowFlags_MenuBar    |
                ImGuiWindowFlags_NoResize   |
                ImGuiWindowFlags_NoMove
            );

            cpu_menu();
            cpu_control_tab();
            cpu_registers_tab();

            End();
        }

        inline void memory_panel() {
            using namespace ImGui;

            static int item = 0;
            static memory_editor editor;

            #define memory_editor_window(t, px, py, sx, sy, c, sz, b) \
                SetNextWindowPos(ImVec2(px, py)); \
                SetNextWindowSize(ImVec2(sx, sy)); \
                Begin("Address Space", NULL, \
                    ImGuiWindowFlags_NoResize   | \
                    ImGuiWindowFlags_NoMove \
                ); \
                    editor.DrawContents(c, sz+1, b); \
                    SameLine(); \
                    if (BeginCombo(device_names[item].c_str(), device_names[item].c_str(), ImGuiComboFlags_NoPreview)) { \
                        for (int n = 0; n < device_names.size(); n++) { \
                            bool is_selected = (item == n); \
                            if (Selectable(device_names[n].c_str(), is_selected)) \
                                item = n; \
                            if (is_selected) \
                                SetItemDefaultFocus(); \
                        } \
                    EndCombo(); \
                    } \
                End();

            switch (item) {
                case 0: {
                    memory_editor_window(device_names[item], 750, 0, 600, 600, bios->get_binary().data(), bios->get_size(), bios->get_base());
                } break;
                case 1: {
                    memory_editor_window(device_names[item], 750, 0, 600, 600, ioctl->get_memory(), ioctl->get_size(), ioctl->get_base());
                } break;
                case 2: {
                    memory_editor_window(device_names[item], 750, 0, 600, 600, memory->get_memory(), memory->get_size(), memory->get_base());
                } break;
            }
        }

    protected:
        void on_event(sf::Event& event) override {
            ImGui::SFML::ProcessEvent(event);
        }

        void setup() override {
            ImGui::CreateContext();
            ImGui::SFML::Init(*get_window());
        }

        void draw() override {
            using namespace ImGui;
            auto w = get_window();
            ImGui::SFML::Update(*w, delta.restart());
            clear(sf::Color(25, 25, 25));

            cpu_panel();
            memory_panel();

            ImGui::SFML::Render(*w);
        }

        void on_close() override {
            //ImGui::PopFont();
            ImGui::DestroyContext();
            ImGui::SFML::Shutdown();
            close();
        }

    public:
        control_window() = default;

        inline void start() {
            this->cpu = machine::bus::get_device<machine::cpu>(0);
            this->bios = machine::bus::get_device<machine::bios>(8);
            this->ioctl = machine::bus::get_device<machine::ioctl>(2);
            this->memory = machine::bus::get_device<machine::dev_memory_t>(9);

            device_names.push_back(bios->get_name() + " @ 0x" + utility::hexnzf(bios->get_base()));
            device_names.push_back(ioctl->get_name() + " @ 0x" + utility::hexnzf(ioctl->get_base()));
            device_names.push_back(memory->get_name() + " @ 0x" + utility::hexnzf(memory->get_base()));
            
            init(1350, 600, "risc64 Control Window", sf::Style::Default, false, false);
        }
    };
}