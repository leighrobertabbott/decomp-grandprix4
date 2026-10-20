#include <gp4/gp4.h>
#include <gp4/flags.h>

// After a control-bit query (0x51dd08/0x51dd3e/0x51dd74): with CF set (eax = 0/1) the byte at
// `cell` gets bit `shift` = !eax; eax becomes (eax ^ 1) << shift.
static inline void ApplyAnswer(gp4::Regs& c, uint8_t* cell, uint8_t keep, uint32_t shift) {
    if ((c.eflags & gp4::flags::CF) == 0u) {
        return;
    }
    c.eax = (c.eax ^ 1u) << shift;
    *cell = static_cast<uint8_t>((*cell & keep) | static_cast<uint8_t>(c.eax));
}

// pushal/popal routine: polls the input devices (0x51d801), then builds two "active-low" control
// masks from the 36 {key, bit} pairs at 0x89108d..0x8910d4: for every pair whose key is down in the
// key bitmap 0x8245a0 (bit key & 7 of byte key >> 3), bit (value & 0x1f) is cleared in the first
// mask when the value is below 0x20 (signed) and in the second otherwise. The first mask is stored
// to the bytes 0x679d18..0x679d1b, the low byte of the second to 0x679d1c; the three control-bit
// queries 0x51dd08/0x51dd3e/0x51dd74 then override bit 5 of 0x679d1b and bits 2 and 0 of 0x679d1c
// when they answer, and bit 6 of 0x679d1b is set unless key 0x10 is down (0x510e04). Every register
// is preserved; the flags are those of the last 0x510e04 / `or byte [0x679d1b], 0x40`.
// FUNCTION: GP4 0x0051d509
void __cdecl FUN_0051d509(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0051d801, c);

    uint32_t low = 0xffffffffu;
    uint32_t high = 0xffffffffu;
    const uint8_t* keys = GP4_ARRAY(uint8_t, 0x008245a0);
    for (const uint8_t* pair = GP4_ARRAY(uint8_t, 0x0089108d); pair < GP4_ARRAY(uint8_t, 0x008910d5);
         pair += 2) {
        const uint8_t key = pair[0];
        const uint8_t mask = static_cast<uint8_t>(1u << (key & 7u));
        if ((keys[key >> 3] & mask) == 0u) {
            continue;
        }
        const int8_t value = static_cast<int8_t>(pair[1]);
        if (value < 0x20) {
            low &= ~(1u << (static_cast<uint32_t>(value) & 0x1fu));
        } else {
            high &= ~(1u << (static_cast<uint8_t>(value - 0x20) & 0x1fu));
        }
    }

    uint8_t* bytes = GP4_ARRAY(uint8_t, 0x00679d18);
    bytes[0] = static_cast<uint8_t>(low);
    bytes[1] = static_cast<uint8_t>(low >> 8);
    bytes[2] = static_cast<uint8_t>(low >> 16);
    bytes[3] = static_cast<uint8_t>(low >> 24);
    bytes[4] = static_cast<uint8_t>(high);

    c.ebp = low;
    c.esi = high;
    c.edi = 0x008910d5u;
    c.eax = high;
    gp4::call_regs(0x0051dd08, c);
    ApplyAnswer(c, &bytes[3], 0xdfu, 5u);
    gp4::call_regs(0x0051dd3e, c);
    ApplyAnswer(c, &bytes[4], 0xfbu, 2u);
    gp4::call_regs(0x0051dd74, c);
    ApplyAnswer(c, &bytes[4], 0xfeu, 0u);

    c.ecx = 0x10u;
    gp4::call_regs(0x00510e04, c);
    uint32_t flags = c.eflags;
    if ((flags & gp4::flags::ZF) == 0u) {
        bytes[3] = static_cast<uint8_t>(bytes[3] | 0x40u);
        flags = gp4::flags::logic8(bytes[3]);
    }

    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051d509, FUN_0051d509, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=save_00510309+keys_008245a0+bits_006f2d90+codes_0051dd08")
