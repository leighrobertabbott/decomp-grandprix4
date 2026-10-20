#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal wrapper: fills the 64 KiB byte table at 0x0096a3d8, indexed by (bh << 8) | bl. Each entry is a
// signed, clamped product of the two sign-extended index bytes: q = (int8(bh) * int8(bl) * 0x80) / 0x78,
// |q| >> 7 clamped to 0x78, with the sign of q. popal restores every register; the flags are those of the last
// `inc bh` (bh wraps 0xff -> 0, CF kept from the last iteration's `or edx,edx` / `neg eax`).
// FUNCTION: GP4 0x0051ed84
void __cdecl FUN_0051ed84(gp4::Regs* r) {
    uint8_t* table = GP4_ARRAY(uint8_t, 0x0096a3d8);
    uint32_t carry = 0u;
    for (uint32_t hi = 0u; hi < 0x100u; ++hi) {
        const int32_t x = static_cast<int8_t>(hi);
        for (uint32_t lo = 0u; lo < 0x100u; ++lo) {
            const int32_t y = static_cast<int8_t>(lo);
            const int32_t product = x * y * 0x80;
            const int32_t q = product / 0x78;
            uint32_t mag = static_cast<uint32_t>(q);
            if (q < 0) {
                mag = 0u - mag;
            }
            mag >>= 7;
            if (static_cast<int32_t>(mag) > 0x78) {
                mag = 0x78u;
            }
            carry = 0u;
            if (q < 0) {
                carry = (mag != 0u) ? 1u : 0u;
                mag = 0u - mag;
            }
            table[(hi << 8) | lo] = static_cast<uint8_t>(mag);
        }
    }
    uint32_t arith = gp4::flags::add8(0xffu, 1u);
    arith = (arith & ~gp4::flags::CF) | carry;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0051ed84, FUN_0051ed84, "regs() -> (cf, pf, af, zf, sf, of); wall=3000")
