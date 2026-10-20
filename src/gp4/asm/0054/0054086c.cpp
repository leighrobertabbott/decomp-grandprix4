#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores al at [ebx], runs 0x005406bf (with eax unchanged) and the pushal/popal wrapper 0x00540648,
// then clears the dword [0x0083000c]. The store and the clear leave the flags alone, so the flags
// (and any register) are those of the last callee.
// FUNCTION: GP4 0x0054086c
void __cdecl FUN_0054086c(gp4::Regs* r) {
    *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx)) = static_cast<uint8_t>(r->eax);
    gp4::Regs c = *r;
    gp4::call_regs(0x005406bf, c);
    gp4::call_regs(0x00540648, c);
    GP4_GLOBAL(uint32_t, 0x0083000c) = 0u;
    *r = c;
}
GP4_IMPL(0x0054086c, FUN_0054086c, "regs(eax:u32{1,2,3,0xff,0x20,0x80}, ebx:ptr[0x40]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=blocks_0054045b+drv22_0052d402+keys_007c2a1c")
