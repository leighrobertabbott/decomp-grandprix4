#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream decoder that paints a nibble into 16 cells of consecutive 0x160-byte segment records
// (mode byte 0x0086857c == 2, else only esi += 6). Reads three words at esi: ax = bit mask, ebp =
// row count, dx = colour (esi += 6). A zero count stops here (ax / dx loaded, flags of `or ebp,
// ebp`). Otherwise dl = colour & 0xf, dh = dl << 4 and, for `count` rows starting at the record in
// ebx (advancing by 0x160, wrapping by [0x866ebc] once it reaches the ring end [0x866eb8]), every
// set bit k of the mask (k = 0..15) rewrites the nibble of byte rec[0x58 + k / 2]: the low nibble
// (dl) for odd k, the high nibble (dh) for even k. ebx and edi are restored; ecx = 0x10, ebp = 0, ax
// = the mask, dx = dh:dl and the flags are those of the last `dec ebp` (CF kept from the final
// compare / subtraction of the ring wrap).
// FUNCTION: GP4 0x0055866c
void __cdecl FUN_0055866c(gp4::Regs* r) {
    const uint32_t source = r->esi;
    r->esi = source + 6u;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(source, 6u));
        return;
    }

    const uint16_t mask = GP4_FIELD(uint16_t, source, 0);
    uint32_t count = GP4_FIELD(uint16_t, source, 2);
    const uint16_t colour = GP4_FIELD(uint16_t, source, 4);
    r->eax = (r->eax & 0xffff0000u) | mask;
    if (count == 0u) {
        r->edx = (r->edx & 0xffff0000u) | colour;
        r->ebp = 0u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
        return;
    }

    const uint8_t low = static_cast<uint8_t>(colour & 0xfu);
    const uint8_t high = static_cast<uint8_t>(low << 4);
    uint32_t rec = r->ebx;
    uint32_t flags;
    do {
        for (uint32_t bit = 0u; bit < 16u; ++bit) {
            if ((mask >> bit) & 1u) {
                const uint32_t cell = rec + (bit >> 1);
                if (bit & 1u) {
                    GP4_FIELD(uint8_t, cell, 0x58) = static_cast<uint8_t>((GP4_FIELD(uint8_t, cell, 0x58) & 0xf0u) | low);
                } else {
                    GP4_FIELD(uint8_t, cell, 0x58) = static_cast<uint8_t>((GP4_FIELD(uint8_t, cell, 0x58) & 0x0fu) | high);
                }
            }
        }
        rec += 0x160u;
        flags = gp4::flags::sub32(rec, GP4_GLOBAL(uint32_t, 0x00866eb8));
        if (rec >= GP4_GLOBAL(uint32_t, 0x00866eb8)) {
            flags = gp4::flags::sub32(rec, GP4_GLOBAL(uint32_t, 0x00866ebc));
            rec -= GP4_GLOBAL(uint32_t, 0x00866ebc);
        }
        flags = gp4::flags::dec32(count, flags);
        count -= 1u;
    } while (count != 0u);

    r->ecx = 0x10u;
    r->edx = (r->edx & 0xffff0000u) | (static_cast<uint32_t>(high) << 8) | low;
    r->ebp = 0u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0055866c, FUN_0055866c, "regs(eax:u32, ebx:rec[0x8943a0,0x160,0..29], edx:u32, esi:ptr[8]:layout=stream_0055866c) -> (eax, ecx, edx, esi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+mode_0086857c+stream_0055866c")
