#include <gp4/gp4.h>
#include <gp4/flags.h>

// Adds ebx to the (unaligned) dword at 0x00833cc5 and eax to the one at 0x00833cc9, then sets the byte
// at 0x00833f74 to 0xff. The flags are those of the second add (the mov leaves them alone).
// FUNCTION: GP4 0x0054cc0a
void __cdecl FUN_0054cc0a(gp4::Regs* r) {
    const uint32_t x = GP4_GLOBAL(uint32_t, 0x00833cc5);
    GP4_GLOBAL(uint32_t, 0x00833cc5) = x + r->ebx;
    const uint32_t y = GP4_GLOBAL(uint32_t, 0x00833cc9);
    GP4_GLOBAL(uint32_t, 0x00833cc9) = y + r->eax;
    GP4_GLOBAL(uint8_t, 0x00833f74) = 0xffu;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(y, r->eax));
}
GP4_IMPL(0x0054cc0a, FUN_0054cc0a, "regs(eax:u32, ebx:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
