#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sets the byte 0x0081d1e9 to 0xff around the front-end screen loop 0x00528b34 and clears it afterwards.
// Registers and flags come back as the loop leaves them (the loop polls the message pump).
// FUNCTION: GP4 0x00532fc9
void __cdecl FUN_00532fc9(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x0081d1e9) = 0xffu;
    gp4::call_regs(0x00528b34, *r);
    GP4_GLOBAL(uint8_t, 0x0081d1e9) = 0u;
}
GP4_IMPL(0x00532fc9, FUN_00532fc9, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
