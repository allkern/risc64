# For WSL/WSL2 users running an X server thingy, uncomment the two following commands:

# export DISPLAY=$(awk '/nameserver / {print $2; exit}' /etc/resolv.conf 2>/dev/null):0
# export LIBGL_ALWAYS_INDIRECT=1

# Compile the emulator
c++ -c main.cc -o main.o -std=c++2a -Ofast -m64 -I"emulator/" -Wno-format

# Compile ImGui stuff
# c++ -c imgui.cpp -o imgui.o -std=c++2a -Ofast -m64 -I"emulator/imgui"
# c++ -c imgui_draw.cpp -o imgui_draw.o -std=c++2a -Ofast -m64 -I"emulator/imgui"
# c++ -c imgui_widgets.cpp -o imgui_widgets.o -std=c++2a -Ofast -m64 -I"emulator/imgui"
# c++ -c imgui-SFML.cpp -o imgui-SFML.o -std=c++2a -Ofast -m64 -I"emulator/imgui"

# Link them all into a neat little file, also link OpenGL
c++ main.o imgui.o imgui_draw.o imgui_widgets.o imgui-SFML.o -o main -Ofast -m64 -std=c++2a -lsfml-graphics -lsfml-window -lsfml-system -lGL -lX11

# Cleanup
rm "main.o"
