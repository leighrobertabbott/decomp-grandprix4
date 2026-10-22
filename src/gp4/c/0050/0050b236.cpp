#include <gp4/gp4.h>
#include <gp4/flags.h>

// Loads the word at 0x00876f46; when its high byte is nonzero subtracts that byte from the bytes
// at 0x009e169c, 0x009e16a8 and 0x009e1698. Leaves ax loaded and the flags of the last operation.
// FUNCTION: GP4 0x0050b236
void __cdecl FUN_0050b236(gp4::Regs* r) {
    const uint16_t ax = GP4_GLOBAL(uint16_t, 0x00876f46);
    const uint8_t ah = static_cast<uint8_t>(ax >> 8);
    uint32_t f = gp4::flags::logic8(ah);
    if (ah != 0) {
        uint8_t& a = GP4_GLOBAL(uint8_t, 0x009e169c);
        f = gp4::flags::sub8(a, ah);
        a = static_cast<uint8_t>(a - ah);
        uint8_t& b = GP4_GLOBAL(uint8_t, 0x009e16a8);
        f = gp4::flags::sub8(b, ah);
        b = static_cast<uint8_t>(b - ah);
        uint8_t& c = GP4_GLOBAL(uint8_t, 0x009e1698);
        f = gp4::flags::sub8(c, ah);
        c = static_cast<uint8_t>(c - ah);
    }
    r->eax = (r->eax & 0xffff0000u) | ax;
    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x0050b236, FUN_0050b236, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
