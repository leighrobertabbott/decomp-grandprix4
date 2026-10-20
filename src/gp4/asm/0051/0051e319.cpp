#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0051e319
// dl is stored at 0x6f2d6c. The signed value eax is clamped by magnitude to 10000 and its sign
// flipped (positive -> -min(x, 10000), negative -> +min(|x|, 10000)); it is stored into one of
// four slots chosen by the byte 0x7ab54a (6: 0x6f2d54, 7: 0x6f2d58, 8: 0x6f2d64, 9: 0x6f2d68).
// CF is clear when a slot was written, set otherwise (stc); the other flags are those of the last
// `cmp bl, k` (k = 9 when nothing matched). bl is left holding the mode byte.
void __cdecl FUN_0051e319(gp4::Regs* r) {
    const uint32_t input = r->eax;
    GP4_GLOBAL(uint8_t, 0x006f2d6c) = static_cast<uint8_t>(r->edx);

    uint32_t value;
    if (input & 0x80000000u) {
        uint32_t magnitude = 0u - input;
        if (magnitude >= 0x2710u) {
            magnitude = 0x2710u;
        }
        value = magnitude;
    } else {
        uint32_t magnitude = input;
        if (magnitude >= 0x2710u) {
            magnitude = 0x2710u;
        }
        value = 0u - magnitude;
    }

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007ab54a);
    uint32_t flags;
    uint32_t carry = 0u;
    if (mode == 6u) {
        flags = gp4::flags::sub8(mode, 6u);
        GP4_GLOBAL(uint32_t, 0x006f2d54) = value;
    } else if (mode == 7u) {
        flags = gp4::flags::sub8(mode, 7u);
        GP4_GLOBAL(uint32_t, 0x006f2d58) = value;
    } else if (mode == 8u) {
        flags = gp4::flags::sub8(mode, 8u);
        GP4_GLOBAL(uint32_t, 0x006f2d64) = value;
    } else if (mode == 9u) {
        flags = gp4::flags::sub8(mode, 9u);
        GP4_GLOBAL(uint32_t, 0x006f2d68) = value;
    } else {
        flags = gp4::flags::sub8(mode, 9u);
        carry = 1u;
    }

    r->eax = value;
    r->ebx = (r->ebx & 0xffffff00u) | mode;
    r->eflags = gp4::flags::merge(r->eflags, (flags & ~gp4::flags::CF) | carry);
}
GP4_IMPL(0x0051e319, FUN_0051e319, "regs(eax:u32, ebx:u32, edx:u32) -> (eax, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
