#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears the byte 0x0082fffc, runs 0x0054b686 (a block send through 0x0054b10c) and, when it returns with
// CF clear, 0x0054b6a0. Registers and flags are those the last callee leaves.
// FUNCTION: GP4 0x0054ba7a
void __cdecl FUN_0054ba7a(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x0082fffc) = 0u;
    gp4::call_regs(0x0054b686, *r);
    if ((r->eflags & 1u) == 0u) {
        gp4::call_regs(0x0054b6a0, *r);
    }
}
GP4_IMPL(0x0054ba7a, FUN_0054ba7a, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_007aafd3; emu=skip:win32 PeekMessageA")
