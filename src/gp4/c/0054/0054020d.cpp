#include <gp4/gp4.h>
#include <gp4/flags.h>

// Per-driver slot flag bytes (22 bytes at 0x007c2a48): looks for the first byte whose top two bits
// are 10 (0x80). When one is found, bits 6-7 of every following byte are cleared. When there is none,
// bit 7 of the first byte is set. al ends as the last tested (byte & 0xc0), ecx = 0 and esi = the end
// of the table (found) or its start (none); the flags are those of the last `dec ecx` (found; CF
// kept from the compare/and before it) or of `or byte [0x007c2a48], 0x80` (none).
// FUNCTION: GP4 0x0054020d
void __cdecl FUN_0054020d(gp4::Regs* r) {
    uint8_t* const row = GP4_ARRAY(uint8_t, 0x007c2a48);
    uint32_t flags = 0u;
    uint32_t tag = 0u;
    uint32_t i = 0u;
    for (; i < 0x16u; ++i) {
        tag = row[i] & 0xc0u;
        flags = gp4::flags::sub8(tag, 0x80u);
        if (tag == 0x80u) {
            break;
        }
        flags = gp4::flags::dec32(0x16u - i, flags);
    }
    if (i == 0x16u) {
        const uint8_t marked = static_cast<uint8_t>(row[0] | 0x80u);
        row[0] = marked;
        flags = gp4::flags::logic8(marked);
        r->esi = 0x007c2a48u;
    } else {
        flags = gp4::flags::dec32(0x16u - i, flags);
        for (uint32_t j = i + 1u; j < 0x16u; ++j) {
            const uint8_t cleared = static_cast<uint8_t>(row[j] & 0x3fu);
            row[j] = cleared;
            flags = gp4::flags::dec32(0x16u - j, gp4::flags::logic8(cleared));
        }
        r->esi = 0x007c2a48u + 0x16u;
    }
    r->eax = (r->eax & 0xffffff00u) | tag;
    r->ecx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054020d, FUN_0054020d, "regs(eax:u32) -> (eax, ecx, esi, cf, pf, af, zf, sf, of); globals=layout=neg22_00520023")
