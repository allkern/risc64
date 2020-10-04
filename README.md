<p align="center">
  <img width=684 height=148 src="https://user-images.githubusercontent.com/15825466/87863971-5afb6d80-c938-11ea-8e07-f9cc1391c43f.png">
</p>

> Basically just an architecture I created to mess around with compilers and stuff

This is the RISC64 architecture, its based on similar RISC-type architectures such as ARM, MIPS, etc.

The emulator included in this repo is capable of emulating a complete RISC64 system, including devices such as a terminal, VGA screen, BIOS chip. It also supports connecting peripherals through a generic I/O controller, IOCTL.

The main focus of this architecture is to be simple, and easily targetable for use with compilers, so that developers don't have to learn a real architecture from the ground up, such as x86-64 or ARM.
