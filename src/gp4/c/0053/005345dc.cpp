#include <gp4/gp4.h>
#include <gp4/flags.h>

// One stage: looks the key byte up with 0x0052c689 (eax = -1 when unknown -> fail). With a nonzero ebx from the
// lookup the stage also requires the bit (1 << cl) of the mask at 0x7ab0ec, where cl is first raised by 6 when the
// lookup returned eax == 1; with ebx == 0 the stage passes at once. `arith` follows the last flag writer.
static inline bool Stage(gp4::Regs& c, uint32_t key, uint32_t& arith) {
    c.eax = (c.eax & 0xffffff00u) | key;
    gp4::call_regs(0x0052c689, c);
    arith = gp4::flags::sub32(c.eax, 0xffffffffu);
    if (c.eax == 0xffffffffu) {
        return false;
    }
    arith = gp4::flags::logic32(c.ebx);
    if (c.ebx != 0u) {
        if (c.eax == 1u) {
            c.ecx += 6u;
        }
        c.eax = 1u << (c.ecx & 31u);
        const uint32_t hit = GP4_GLOBAL(uint32_t, 0x007ab0ec) & c.eax;
        arith = gp4::flags::logic32(hit);
        if (hit == 0u) {
            return false;
        }
    }
    return true;
}

// Checks the three control selector bytes 0x7ab54a, 0x7ab550 and 0x7ab553 in turn: CF = 1 as soon as one stage fails,
// CF = 0 when all three pass. pushal/popal restore every register; the other flags are those of the last flag writer
// (the `cmp eax,-1`, the `or ebx,ebx` or the `test` of the deciding stage).
// FUNCTION: GP4 0x005345dc
void __cdecl FUN_005345dc(gp4::Regs* r) {
    gp4::Regs c = *r;
    uint32_t arith = 0u;
    uint32_t carry = 1u;

    if (Stage(c, GP4_GLOBAL(uint8_t, 0x007ab54a), arith) &&
        Stage(c, GP4_GLOBAL(uint8_t, 0x007ab550), arith) &&
        Stage(c, GP4_GLOBAL(uint8_t, 0x007ab553), arith)) {
        carry = 0u;
    }

    r->eflags = gp4::flags::merge(r->eflags, (arith & ~static_cast<uint32_t>(gp4::flags::CF)) | carry);
}
GP4_IMPL(0x005345dc, FUN_005345dc, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ctl_007ab548")
