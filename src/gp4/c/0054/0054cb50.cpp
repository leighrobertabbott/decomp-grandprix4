#include <gp4/gp4.h>

// Resets the 0x0054cb7e/0x0054cc96 pair: first (eax, ebx) = (0, 0), then eax = 0x7fff, and clears the
// byte 0x00833cf3. Both callees preserve every register, so eax = 0x7fff and ebx = 0 stay; ecx and
// edx pass through to 0x0054cc96. The flags are those of its final `add edx, edx`.
// FUNCTION: GP4 0x0054cb50
void __cdecl FUN_0054cb50(gp4::Regs* r) {
    r->ebx = 0u;
    r->eax = 0u;
    gp4::call_regs(0x0054cb7e, *r);
    r->eax = 0x7fffu;
    gp4::call_regs(0x0054cc96, *r);
    GP4_GLOBAL(uint8_t, 0x00833cf3) = 0u;
}
GP4_IMPL(0x0054cb50, FUN_0054cb50, "regs(ecx:u32, edx:u32) -> (eax, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=reset_0054cb50")
