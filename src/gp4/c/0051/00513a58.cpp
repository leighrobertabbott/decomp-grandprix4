#include <gp4/gp4.h>
#include <gp4/flags.h>

// One helper per arm so the compiler keeps both targets as written.
static __declspec(noinline) void RunSessionReset(gp4::Regs& c) {
    gp4::call_regs(0x00513aa0, c);
}

// Dispatcher command 0x20 (a front-end screen, 0x00532552) answers 0 or another value. The answer is
// clamped to 0/1 and indexes the two-entry jump table at 0x00513a78 (`shl eax, 2; add eax, 0x513a78;
// jmp [eax]`, so eax = the table slot address on entry to the arm):
//   0: the session reset 0x00513aa0, then CF is cleared (`clc`);
//   1: the bytes [0x007c2ead] = 0, the word [0x007c2e70] = 0 and the byte [0x007a93b6] = 0xff are stored
//      and CF is set (`stc`).
// The other flags are those of the `add eax, 0x513a78` (or, in arm 0, whatever 0x00513aa0 leaves).
// FUNCTION: GP4 0x00513a58
void __cdecl FUN_00513a58(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = (c.eax & 0xffff0000u) | 0x20u;
    gp4::call_regs(0x00531f9c, c);

    if (c.eax >= 2u) {
        c.eax = 1u;
    }
    const uint32_t slot = c.eax << 2;
    c.eax = slot + 0x00513a78u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(slot, 0x00513a78u));
    if (slot == 0u) {
        RunSessionReset(c);
        c.eflags &= ~gp4::flags::CF;
    } else {
        GP4_GLOBAL(uint8_t, 0x007c2ead) = 0u;
        GP4_GLOBAL(uint16_t, 0x007c2e70) = 0u;
        GP4_GLOBAL(uint8_t, 0x007a93b6) = 0xffu;
        c.eflags |= gp4::flags::CF;
    }
    *r = c;
}
GP4_IMPL(0x00513a58, FUN_00513a58, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
