#include <gp4/gp4.h>

// Stores the outgoing packet of slot [0x007aafd0] with 0x0054b978 and re-reads it with 0x0054b910
// (ax = the slot byte; the upper half of eax is whatever 0x0054b978 left). Registers and flags come
// back as 0x0054b910 leaves them.
// FUNCTION: GP4 0x0054b8fe
void __cdecl FUN_0054b8fe(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054b978, c);
    c.eax = (c.eax & 0xffff0000u) | GP4_GLOBAL(uint8_t, 0x007aafd0);
    gp4::call_regs(0x0054b910, c);
    *r = c;
}
GP4_IMPL(0x0054b8fe, FUN_0054b8fe, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0054b978+mode_007aafd3")
