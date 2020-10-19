# Printing Hex Number efficiently on AVR CPUs

I was playing with the idea of using ASM to save a few bytes of code for arduino.
The simple task was to write a routine that prints hex numbers via the serial interface.

I am saving all steps and disassembly if someone wants to look. I also added clock cycle counter by hand. If you find any mistakes - pleas say so!

The pure C version is 62 bytes of code and between 50 and 52 clock cycles.

The best ASM version is 48 bytes of code and 29 clock cycles.

This saves 22.6% code and 42-44% CPU clock cycles, not bad.
