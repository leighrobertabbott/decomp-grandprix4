#include <gp4/gp4.h>
#include <gp4/flags.h>

// Character-class test used by the text-entry code: when ah == 2, walks the zero-terminated list of
// (lo, hi) byte pairs at [0x00828108] looking for a pair with lo <= al <= hi. Carry is clear when a
// pair contains al; it is set when ah != 2 or the list ends first. The other flags are those of the
// last compare (`cmp ah, 2`, `cmp byte [esi], 0` or `cmp al, [esi + 1]`); only CF is replaced by
// stc/clc. esi is preserved.
// FUNCTION: GP4 0x00533d79
void __cdecl FUN_00533d79(gp4::Regs* r) {
    const uint8_t al = (uint8_t)(r->eax & 0xffu);
    const uint8_t ah = (uint8_t)((r->eax >> 8) & 0xffu);

    uint32_t flags = gp4::flags::sub8(ah, 2u);
    uint32_t carry = 1u;
    if (ah == 2u) {
        const uint8_t* pair = (const uint8_t*)(uintptr_t)GP4_GLOBAL(uint32_t, 0x00828108);
        for (;;) {
            const uint8_t lo = pair[0];
            flags = gp4::flags::sub8(lo, 0u);
            if (lo == 0u) {
                break;
            }
            flags = gp4::flags::sub8(al, lo);
            if (al >= lo) {
                const uint8_t hi = pair[1];
                flags = gp4::flags::sub8(al, hi);
                if (al <= hi) {
                    carry = 0u;
                    break;
                }
            }
            pair += 2;
        }
    }

    flags = (flags & ~1u) | carry;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00533d79, FUN_00533d79, "regs(eax:u32{0x200,0x201,0x220,0x230,0x239,0x241,0x25a,0x261,0x27a,0x27f,0x280,0x2fe,0x2ff,0x101,0x341,0x41,0xffff0241,0x12340230}) -> (cf, pf, af, zf, sf, of); globals=layout=chars_00828108")
