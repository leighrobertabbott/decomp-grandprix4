#include <gp4/gp4.h>
#include <gp4/flags.h>

// al = the byte 0x007aafdb, flipped in bit 0 when the byte 0x007ab1a0 is nonzero. Only the low byte
// of eax changes; the flags are those of `cmp byte [0x7ab1a0], 0` or of the `xor al, 1`.
// FUNCTION: GP4 0x0054b710
void __cdecl FUN_0054b710(gp4::Regs* r) {
    uint8_t al = GP4_GLOBAL(uint8_t, 0x007aafdb);
    const uint8_t selector = GP4_GLOBAL(uint8_t, 0x007ab1a0);
    uint32_t flags = gp4::flags::sub8(selector, 0u);
    if (selector != 0u) {
        al = static_cast<uint8_t>(al ^ 1u);
        flags = gp4::flags::logic8(al);
    }
    r->eax = (r->eax & 0xffffff00u) | al;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054b710, FUN_0054b710, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
