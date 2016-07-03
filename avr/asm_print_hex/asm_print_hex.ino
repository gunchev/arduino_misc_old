/**
 * The simple task was to write a routine that prints hex numbers via the serial interface.
 *
 * I am saving all steps and disassembly if someone wants to look. I also added clock cycle counter
 * by hand. If you find any mistakes - pleas say so!
 *
 * The pure C++ function uses:             62 bytes code and 50-52 CPU clock cycles to execute.
 * The function with inline assembly uses: 48 bytes code and 29    CPU clock cycles to execute.
 * Saved                                   22.6% code and    42-44% CPU clock cycles
 *
 * NB: I am counting CPU clock cycles only in my code, the Serial.write is not taken into account!
 *
 * Links:
 * - https://gcc.gnu.org/wiki/avr-gcc
 * - http://www.atmel.com/Images/Atmel-0856-AVR-Instruction-Set-Manual.pdf
 * - http://web.stanford.edu/class/ee281/projects/aut2002/yingzong-mouse/media/GCCAVRInlAsmCB.pdf
 * - http://www.avr-asm-tutorial.net/avr_en/calc/CONVERT.html#bin2hex
 *
 * Notes:
 * - I am by no means AVR assembly language guru, this is my first touch with it. I leadned some 6502
 *   and 80x86/80x386 assembly language back in 1992-1994 and this is mostly copy from GCC and the manuals.
 *
 * 2017-07-03 Doncho Gunchev <dgunchev@gmail.com>
 *   You can use this code freely, credits would be appriciated :-)
 */

// Easy Arduino asm dump on Linux console:
//   avr-objdump -d -S -j .text /tmp/build*/*.elf | less
// This gives you the last compiled project's disassembly. Just search for your function there.

void setup() {
    Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
}

// Pure C++, 62 bytes code, 50-52 clock cycles (each octed > 9 (0xA - 0xF) adds one cycle).
void printHex62(const uint8_t n) {
    unsigned char c = n >> 4;
    c += '0';
    if (c > '9') {
        c += 'A' - '9' - 1;
    }
    Serial.write(c);
    c = '0' + (n & 0x0F);
    if (c > '9') {
        c += 'A' - '9' - 1;
    }
    Serial.write(c);
}
/*
000000f0 <_Z10printHex62h>:
  f0:   cf 93           push    r28                                            ; CLK= 0  (2 clock cycles)
  f2:   c8 2f           mov     r28, r24                                       ; CLK= 2
  f4:   28 2f           mov     r18, r24                                       ; CLK= 3
  f6:   30 e0           ldi     r19, 0x00       ; 0                            ; CLK= 4
  f8:   84 e0           ldi     r24, 0x04       ; 4                            ; CLK= 5
  fa:   35 95           asr     r19                                            ; CLK= 6, 11, 16, 21
  fc:   27 95           ror     r18                                            ; CLK= 7, 12, 17, 22
  fe:   8a 95           dec     r24                                            ; CLK= 8, 13, 18, 23
 100:   e1 f7           brne    .-8             ; 0xfa <_Z10printHex62h+0xa>   ; CLK= 9, 14, 19, 24 (1 clock cycles if no jump, 2 clock cycles if jump)
 102:   60 e3           ldi     r22, 0x30       ; 48                           ; CLK=25
 104:   62 0f           add     r22, r18                                       ; CLK=26
 106:   6a 33           cpi     r22, 0x3A       ; 58                           ; CLK=27
 108:   10 f0           brcs    .+4             ; 0x10e <_Z10printHex62h+0x1e> ; CLK=28          (1 clock cycles if no jump, 2 clock cycles if jump)
 10a:   67 e3           ldi     r22, 0x37       ; 55                           ; CLK=   29
 10c:   62 0f           add     r22, r18                                       ; CLK=   30
 10e:   8b e2           ldi     r24, 0x2B       ; 43                           ; CLK=30|31
 110:   91 e0           ldi     r25, 0x01       ; 1                            ; CLK=32|33
 112:   0e 94 58 03     call    0x6b0   ; 0x6b0 <_ZN14HardwareSerial5writeEh>  ; CLK=33|34       (4 clock cycles for devices with 16 bit PC)
 116:   cf 70           andi    r28, 0x0F       ; 15                           ; CLK=37|38
 118:   60 e3           ldi     r22, 0x30       ; 48                           ; CLK=38|39
 11a:   6c 0f           add     r22, r28                                       ; CLK=39|40
 11c:   6a 33           cpi     r22, 0x3A       ; 58                           ; CLK=40|41
 11e:   10 f0           brcs    .+4             ; 0x124 <_Z10printHex62h+0x34> ; CLK=41|42       (1 clock cycles if no jump, 2 clock cycles if jump)
 120:   67 e3           ldi     r22, 0x37       ; 55                           ; CLK=      42|43
 122:   6c 0f           add     r22, r28                                       ; CLK=      43|44
 124:   8b e2           ldi     r24, 0x2B       ; 43                           ; CLK=43|44|44|45
 126:   91 e0           ldi     r25, 0x01       ; 1                            ; CLK=44|45|45|46
 128:   cf 91           pop     r28                                            ; CLK=45|46|46|47 (2 clock cycles)
 12a:   0c 94 58 03     jmp     0x6b0   ; 0x6b0 <_ZN14HardwareSerial5writeEh>  ; CLK=47|48|48|49 (3 clock cycles)
                                                                               ; CLK=50|51|51|52
*/


// First try, just optimize the shift loop which wastes space and time.
// Saved 4 bytes FLASH and 17 CPU clock cycles. (62 -> 58 FLASH, 50-52 -> 33-35 clocks).
void printHex58(const uint8_t n) {
    unsigned char c = n;
    asm(
        "lsr  %A0"                "\n\t"
        "lsr  %A0"                "\n\t"
        "lsr  %A0"                "\n\t"
        "lsr  %A0"                "\n\t"
        : "+r"(c)
    );
    c += '0';
    if (c > '9') {
        c += 'A' - '9' - 1;
    }
    Serial.write(c);
    c = '0' + (n & 0x0F);
    if (c > '9') {
        c += 'A' - '9' - 1;
    }
    Serial.write(c);
}
/*
000000f0 <_Z10printHex58h>:
  f0:   cf 93           push    r28                                             ; CLK= 0 (2 clock cycles)
  f2:   c8 2f           mov     r28, r24                                        ; CLK= 2
  f4:   98 2f           mov     r25, r24                                        ; CLK= 3
  f6:   96 95           lsr     r25                                             ; CLK= 4
  f8:   96 95           lsr     r25                                             ; CLK= 5
  fa:   96 95           lsr     r25                                             ; CLK= 6
  fc:   96 95           lsr     r25                                             ; CLK= 7
  fe:   60 e3           ldi     r22, 0x30       ; 48                            ; CLK= 8
 100:   69 0f           add     r22, r25                                        ; CLK= 9
 102:   6a 33           cpi     r22, 0x3A       ; 58                            ; CLK=10
 104:   10 f0           brcs    .+4             ; 0x10a <_Z10printHex58h+0x1a>  ; CLK=11     (1 clock cycles if no jump, 2 clock cycles if jump)
 106:   67 e3           ldi     r22, 0x37       ; 55                            ; CLK=   12
 108:   69 0f           add     r22, r25                                        ; CLK=   13
 10a:   8b e2           ldi     r24, 0x2B       ; 43                            ; CLK=13|14
 10c:   91 e0           ldi     r25, 0x01       ; 1                             ; CLK=14|15
 10e:   0e 94 56 03     call    0x6ac   ; 0x6ac <_ZN14HardwareSerial5writeEh>   ; CLK=15|16  (4 clock cycles for devices with 16 bit PC)
 112:   cf 70           andi    r28, 0x0F       ; 15                            ; CLK=19|20
 114:   60 e3           ldi     r22, 0x30       ; 48                            ; CLK=20|21
 116:   6c 0f           add     r22, r28                                        ; CLK=21|22
 118:   6a 33           cpi     r22, 0x3A       ; 58                            ; CLK=22|23
 11a:   10 f0           brcs    .+4             ; 0x120 <_Z10printHex58h+0x30>  ; CLK=23|24
 11c:   67 e3           ldi     r22, 0x37       ; 55                            ; CLK=      24|25
 11e:   6c 0f           add     r22, r28                                        ; CLK=      25|26
 120:   8b e2           ldi     r24, 0x2B       ; 43                            ; CLK=25|26|26|27
 122:   91 e0           ldi     r25, 0x01       ; 1                             ; CLK=27|28|28|29
 124:   cf 91           pop     r28                                             ; CLK=28|29|29|30  (2 clock cycles)
 126:   0c 94 56 03     jmp     0x6ac   ; 0x6ac <_ZN14HardwareSerial5writeEh>   ; CLK=30|31|31|32  (3 clock cycles)
                                                                                ; CLK=33|34|34|35
*/

// Second try, move the add to inline assembly, 2 more bytes FLASH (1 instruction) saved.
// Here I got the hint "subi    r22, 249" from GCC! Thanks!
// There is no ADDI, but "ADDI reg, value" == "SUBI reg, 256 - value" or just "SUBI reg, -value"!
void printHex56(const uint8_t n) {
    unsigned char c = n;
    const char chr0 = '0';
    asm(
        "lsr  %[c]"                "\n\t"
        "lsr  %[c]"                "\n\t"
        "lsr  %[c]"                "\n\t"
        "lsr  %[c]"                "\n\t"
        "add  %[c], %[chr0]"       "\n\t"
        : /* Output registers */ \
          [c] "+r" (c) \
        : /* Input registers */ \
          [chr0] "r" (chr0) \
        : /* Clobbered registers */ \
    );
    if (c > '9') {
        c += 'A' - '9' - 1;
    }
    Serial.write(c);
    c = (n & 0x0F) + chr0;
    if (c > '9') {
        c += 'A' - '9' - 1;
    }
    Serial.write(c);
}
/*
000000f0 <_Z10printHex56h>:
  f0:   cf 93           push    r28
  f2:   c8 2f           mov     r28, r24
  f4:   68 2f           mov     r22, r24
  f6:   80 e3           ldi     r24, 0x30       ; 48
  f8:   66 95           lsr     r22
  fa:   66 95           lsr     r22
  fc:   66 95           lsr     r22
  fe:   66 95           lsr     r22
 100:   68 0f           add     r22, r24
 102:   6a 33           cpi     r22, 0x3A       ; 58
 104:   08 f0           brcs    .+2             ; 0x108 <_Z10printHex56h+0x18>
 106:   69 5f           subi    r22, 0xF9       ; 249
 108:   8b e2           ldi     r24, 0x2B       ; 43
 10a:   91 e0           ldi     r25, 0x01       ; 1
 10c:   0e 94 55 03     call    0x6aa   ; 0x6aa <_ZN14HardwareSerial5writeEh>
 110:   cf 70           andi    r28, 0x0F       ; 15
 112:   60 e3           ldi     r22, 0x30       ; 48
 114:   6c 0f           add     r22, r28
 116:   6a 33           cpi     r22, 0x3A       ; 58
 118:   10 f0           brcs    .+4             ; 0x11e <_Z10printHex56h+0x2e>
 11a:   67 e3           ldi     r22, 0x37       ; 55
 11c:   6c 0f           add     r22, r28
 11e:   8b e2           ldi     r24, 0x2B       ; 43
 120:   91 e0           ldi     r25, 0x01       ; 1
 122:   cf 91           pop     r28
 124:   0c 94 55 03     jmp     0x6aa   ; 0x6aa <_ZN14HardwareSerial5writeEh>
*/

// Mostly inline assembly, save 4 more bytes FLASH => 62 -> 52 bytes, 50-52 -> 31 clock cycles.
// The constant execution time is also a plus!
void printHex52(const uint8_t n) {
    unsigned char c = n;
    asm(
        "lsr  %[c]"                           "\n\t"
        "lsr  %[c]"                           "\n\t"
        "lsr  %[c]"                           "\n\t"
        "lsr  %[c]"                           "\n\t"
        "subi %[c], 256 - '0'"                "\n\t" // c += '0';
        "cpi  %[c], '9' + 1"                  "\n\t" // if (c > '9')
        "brcs .+2"                            "\n\t"
        "subi %[c], 256 - ('A' - '9' - 1)"    "\n\t" //   c += 'A' - '9' - 1;
        : /* Output registers */ \
          [c] "+r" (c) \
        : /* Input registers */ \
        : /* Clobbered registers */ \
    );
    Serial.write(c);
    c = n;
    asm(
        "mov  %[c], %[n]"                     "\n\t" // c = n;
        "andi %[c], 0x0F"                     "\n\t" // c = c & 0x0F;
        "subi %[c], 256 - '0'"                "\n\t" // c += '0';
        "cpi  %[c], '9' + 1"                  "\n\t" // if (c > '9')
        "brcs .+2"                            "\n\t"
        "subi    %[c], 256 - 7"               "\n\t" //   c += 'A' - '9' - 1; // 'A' - '9' - 1 = 7
        : /* Output registers */ \
          [c] "=r" (c) \
        : /* Input registers */ \
          [n] "r" (n) \
        : /* Clobbered registers */ \
    );
    Serial.write(c);
}
/*
000000f0 <_Z10printHex52h>:
  f0:   cf 93           push    r28                                             ; CLK= 0 (2 clock cycles)
  f2:   c8 2f           mov     r28, r24                                        ; CLK= 2
  f4:   68 2f           mov     r22, r24                                        ; CLK= 3
  f6:   66 95           lsr     r22                                             ; CLK= 4
  f8:   66 95           lsr     r22                                             ; CLK= 5
  fa:   66 95           lsr     r22                                             ; CLK= 6
  fc:   66 95           lsr     r22                                             ; CLK= 7
  fe:   60 5d           subi    r22, 0xD0       ; 208                           ; CLK= 8
 100:   6a 33           cpi     r22, 0x3A       ; 58                            ; CLK= 9
 102:   08 f0           brcs    .+2             ; 0x106 <_Z10printHex52h+0x16>  ; CLK=10 (1 clock cycles if no jump, 2 clock cycles if jump)
 104:   69 5f           subi    r22, 0xF9       ; 249                           ; CLK=11
 106:   8b e2           ldi     r24, 0x2B       ; 43                            ; CLK=12 (9 + 2 or 10 + 1)
 108:   91 e0           ldi     r25, 0x01       ; 1                             ; CLK=13
 10a:   0e 94 53 03     call    0x6a6   ; 0x6a6 <_ZN14HardwareSerial5writeEh>   ; CLK=14 (4 clock cycles for devices with 16 bit PC)
 10e:   6c 2f           mov     r22, r28                                        ; CLK=18
 110:   6f 70           andi    r22, 0x0F       ; 15                            ; CLK=19
 112:   60 5d           subi    r22, 0xD0       ; 208                           ; CLK=20
 114:   6a 33           cpi     r22, 0x3A       ; 58                            ; CLK=21
 116:   08 f0           brcs    .+2             ; 0x11a <_Z10printHex52h+0x2a>  ; CLK=22
 118:   69 5f           subi    r22, 0xF9       ; 249                           ; CLK=23
 11a:   8b e2           ldi     r24, 0x2B       ; 43                            ; CLK=24
 11c:   91 e0           ldi     r25, 0x01       ; 1                             ; CLK=25
 11e:   cf 91           pop     r28                                             ; CLK=26 (2 clock cycles)
 120:   0c 94 53 03     jmp     0x6a6   ; 0x6a6 <_ZN14HardwareSerial5writeEh>   ; CLK=28 (3 clock cycles)
                                                                                ; CLK=31
*/


// Use swap => 62 -> 48 bytes, 50-52 -> 29 clock cycles.
// Got the idea from http://www.avr-asm-tutorial.net/avr_en/calc/CONVERT.html#bin2hex
void printHex48(const uint8_t n) {
    unsigned char c = n;
    asm(
        "swap %[c]"                           "\n\t"
        "andi %[c], 0x0F"                     "\n\t"
        "subi %[c], -'0'"                     "\n\t" // c += '0';
        "cpi  %[c], '9' + 1"                  "\n\t" // if (c > '9')
        "brcs .+2"                            "\n\t"
        "subi %[c], -('A' - '9' - 1)"         "\n\t" //   c += 'A' - '9' - 1;
        : /* Output registers */ \
          [c] "+r" (c) \
        : /* Input registers */ \
        : /* Clobbered registers */ \
    );
    Serial.write(c);
    c = n;
    asm(
        "mov  %[c], %[n]"                     "\n\t" // c = n;
        "andi %[c], 0x0F"                     "\n\t" // c = c & 0x0F;
        "subi %[c], -'0'"                     "\n\t" // c += '0';
        "cpi  %[c], '9' + 1"                  "\n\t" // if (c > '9')
        "brcs .+2"                            "\n\t"
        "subi %[c], -('A' - '9' - 1)"         "\n\t" //   c += 'A' - '9' - 1; // 'A' - '9' - 1 = 7
        : /* Output registers */ \
          [c] "=r" (c) \
        : /* Input registers */ \
          [n] "r" (n) \
        : /* Clobbered registers */ \
    );
    Serial.write(c);
}
/*
000000f0 <_Z10printHex48h>:
  f0:   cf 93           push    r28                                             ; CLK= 0 (2 clock cycles)
  f2:   c8 2f           mov     r28, r24                                        ; CLK= 2
  f4:   68 2f           mov     r22, r24                                        ; CLK= 3
  f6:   62 95           swap    r22                                             ; CLK= 4
  f8:   6f 70           andi    r22, 0x0F       ; 15                            ; CLK= 5
  fa:   60 5d           subi    r22, 0xD0       ; 208                           ; CLK= 6
  fc:   6a 33           cpi     r22, 0x3A       ; 58                            ; CLK= 7
  fe:   08 f0           brcs    .+2             ; 0x102 <_Z10printHex48h+0x12>  ; CLK= 8 (1 clock cycles if no jump, 2 clock cycles if jump)
 100:   69 5f           subi    r22, 0xF9       ; 249                           ; CLK= 9
 102:   8b e2           ldi     r24, 0x2B       ; 43                            ; CLK=10 (9 + 2 or 10 + 1)
 104:   91 e0           ldi     r25, 0x01       ; 1                             ; CLK=11
 106:   0e 94 51 03     call    0x6a2   ; 0x6a2 <_ZN14HardwareSerial5writeEh>   ; CLK=12 (4 clock cycles for devices with 16 bit PC)
 10a:   6c 2f           mov     r22, r28                                        ; CLK=16
 10c:   6f 70           andi    r22, 0x0F       ; 15                            ; CLK=17
 10e:   60 5d           subi    r22, 0xD0       ; 208                           ; CLK=18
 110:   6a 33           cpi     r22, 0x3A       ; 58                            ; CLK=19
 112:   08 f0           brcs    .+2             ; 0x116 <_Z10printHex48h+0x26>  ; CLK=20
 114:   69 5f           subi    r22, 0xF9       ; 249                           ; CLK=21
 116:   8b e2           ldi     r24, 0x2B       ; 43                            ; CLK=22
 118:   91 e0           ldi     r25, 0x01       ; 1                             ; CLK=23
 11a:   cf 91           pop     r28                                             ; CLK=24 (2 clock cycles)
 11c:   0c 94 51 03     jmp     0x6a2   ; 0x6a2 <_ZN14HardwareSerial5writeEh>   ; CLK=26 (3 clock cycles)
                                                                                ; CLK=29
*/


void loop() {
    while (Serial.available() == 0) {
        delay(100);
    }
    Serial.println(F("\nHex numbers from 0x00 to 0xFF:"));
    Serial.read();
    uint8_t i = 0;
    do {
        printHex48(i);
        if (i % 16 == 15) {
            Serial.println();
        } else {
            Serial.write(' ');
        }
        ++i;
    } while (i != 0);
}
