#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the byte at 0x00828102 is 0xff, marks the next slot 0x00828103 unused (0xff) and sets bit 0 of the
// byte at 0x0082fd6a. The flags are those of `cmp byte [0x828102], 0xff`, or of the `or` on that path.
// FUNCTION: GP4 0x00533d49
void __cdecl FUN_00533d49(gp4::Regs* r) {
    const uint8_t slot = GP4_GLOBAL(uint8_t, 0x00828102);
    uint32_t flags = gp4::flags::sub8(slot, 0xffu);
    if (slot == 0xffu) {
        GP4_GLOBAL(uint8_t, 0x00828103) = 0xffu;
        const uint8_t bits = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x0082fd6a) | 1u);
        GP4_GLOBAL(uint8_t, 0x0082fd6a) = bits;
        flags = gp4::flags::logic8(bits);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00533d49, FUN_00533d49, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
