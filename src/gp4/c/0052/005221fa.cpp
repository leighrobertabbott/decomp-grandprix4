#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the pushal wrapper 0x0044174b on the record in esi, then toggles state: with byte [0x007aafd4] clear it sets
// bit 7 of byte [0x007c2ead]; otherwise, unless byte [0x007a93bd] is already 0xff, it flips bit 7 of record byte
// +0x78, stores 0xff to [0x007a93bd] / [0x006f3b7c] / [0x006f3b7d], 0x80 to [0x00852d70] and, when the flipped
// bit is set, clears bit 2 of dword [0x00852d64]. Registers are preserved; the flags are those of the last
// flag-writing instruction on the path taken.
// FUNCTION: GP4 0x005221fa
void __cdecl FUN_005221fa(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x0044174b, call);

    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t arith;
    if (GP4_GLOBAL(uint8_t, 0x007aafd4) == 0u) {
        const uint8_t v = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x007c2ead) | 0x80u);
        GP4_GLOBAL(uint8_t, 0x007c2ead) = v;
        arith = gp4::flags::logic8(v);
    } else {
        const uint8_t cur = GP4_GLOBAL(uint8_t, 0x007a93bd);
        arith = gp4::flags::sub8(cur, 0xffu);
        if (cur != 0xffu) {
            const uint8_t flipped = static_cast<uint8_t>(GP4_FIELD(uint8_t, rec, 0x78) ^ 0x80u);
            GP4_FIELD(uint8_t, rec, 0x78) = flipped;
            arith = gp4::flags::logic8(flipped);
            GP4_GLOBAL(uint8_t, 0x007a93bd) = 0xffu;
            GP4_GLOBAL(uint8_t, 0x006f3b7c) = 0xffu;
            GP4_GLOBAL(uint8_t, 0x006f3b7d) = 0xffu;
            GP4_GLOBAL(uint8_t, 0x00852d70) = 0x80u;
            const uint8_t bit = static_cast<uint8_t>(GP4_FIELD(uint8_t, rec, 0x78) & 0x80u);
            arith = gp4::flags::logic8(bit);
            if (bit != 0u) {
                const uint32_t v = GP4_GLOBAL(uint32_t, 0x00852d64) & 0xfffffffbu;
                GP4_GLOBAL(uint32_t, 0x00852d64) = v;
                arith = gp4::flags::logic32(v);
            }
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005221fa, FUN_005221fa, "regs(esi:ptr[0x84]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
