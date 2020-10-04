# For WSL/WSL2 users running an X server thingy, uncomment the two following commands:

# export DISPLAY=$(awk '/nameserver / {print $2; exit}' /etc/resolv.conf 2>/dev/null):0

# Compile the emulator
cd ..
c++ -c risc64.cc -o build/risc64.o -std=c++2a -Ofast -m64 -I"emulator/" -Wno-format

# Link everything into a neat little file, also link OpenGL and X11
c++ build/risc64.o build/imgui.o build/imgui_draw.o build/imgui_widgets.o build/imgui-SFML.o -o risc64-e -Ofast -m64 -std=c++2a -lsfml-graphics -lsfml-window -lsfml-system -lGL -lX11

# Cleanup
cd build
rm "risc64.o"
