#include <gp4/gp4.h>
#include <gp4/flags.h>

// Builds the 22-bit "key is not an option key" mask of a key-name string and applies it to the item chain
// 0x008522a9. pushal/popal wrapper: every register comes back unchanged, only the flags of the last flag writer
// (the item walk 0x0053e4db) remain. The string is the 22 bytes at 0x00828237 (eax = 0) or 0x0082824d; for each
// byte, in order, a bit is shifted into the top of ebx: 0 when the byte equals dl or dh, else the carry of
// 0x0052bd14 (the byte is one of the menu option bytes). After the 22 bytes ebx >>= 10, so the first byte is
// bit 0; the mask is `(ebx | caller's ebx) << 2`.
// FUNCTION: GP4 0x0052bf0f
void __cdecl FUN_0052bf0f(gp4::Regs* r) {
    const uint8_t* text = (r->eax == 0u) ? GP4_ARRAY(const uint8_t, 0x00828237)
                                         : GP4_ARRAY(const uint8_t, 0x0082824d);
    const uint32_t seed = r->ebx;
    const uint32_t lowKey = r->edx & 0xffu;
    const uint32_t highKey = (r->edx >> 8) & 0xffu;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint32_t bits = 0u;
    for (uint32_t k = 0u; k < 0x16u; ++k) {
        const uint32_t ch = text[k];
        if (ch == lowKey || ch == highKey) {
            bits >>= 1;
        } else {
            c.eax = (c.eax & 0xffffff00u) | ch;
            gp4::call_regs(0x0052bd14, c);
            bits = ((c.eflags & gp4::CF) << 31) | (bits >> 1);
        }
    }
    bits >>= 10;

    c.ebx = bits;
    c.ecx = 0u;
    c.edi = seed;
    c.eax = (bits | seed) << 2;
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x008522a9)));
    gp4::call_regs(0x0053e4db, c);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052bf0f, FUN_0052bf0f, "regs(eax:u32{0,0,1,0x10,0xffffffff}, ebx:u32, edx:u32{0x0102,0x2010,0x3f01,0x1003,0x8020,0x3f3f,0,0xff80}) -> (cf, pf, af, zf, sf, of); globals=layout=set_0081d60f+keys_0052bf0f")
