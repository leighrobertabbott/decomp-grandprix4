#include <gp4/gp4.h>
#include <gp4/flags.h>

// Loads the key preset selected by the byte [0x007ab544]: 0x0052b96d scatters it into the current preset
// bytes at 0x0081d60f.., 0x0052e131 refreshes the key-name captions and 0x0052b942 copies the preset block
// to 0x007ab548. Then each of the six bytes at 0x0081d60f.. is looked up by 0x0052c689 (key -> signed a, b,
// c); a nonzero b sets bit (c + 6 * a) in the mask edx (shl by cl, so the position is taken mod 32).
// Exit registers: eax = the last iteration's value (1 << cl when b was nonzero, else a), ebx = 0x0081d615,
// ecx = 0, edx = the mask. The flags are those of the closing `dec ecx` (1 -> 0) with CF clear (the
// `or` in every iteration cleared it and inc keeps it).
// FUNCTION: GP4 0x0052be64
void __cdecl FUN_0052be64(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    c.eax = GP4_GLOBAL(uint8_t, 0x007ab544);
    gp4::call_regs(0x0052b96d, c);
    gp4::call_regs(0x0052e131, c);
    gp4::call_regs(0x0052b942, c);

    uint32_t eax = c.eax;
    uint32_t mask = 0u;
    const uint8_t* cell = GP4_ARRAY(const uint8_t, 0x0081d60f);
    for (uint32_t n = 0u; n < 6u; ++n) {
        gp4::Regs k = c;
        k.eax = (eax & 0xffffff00u) | cell[n];
        k.ebx = 0x0081d60fu + n;
        k.ecx = 6u - n;
        k.edx = mask;
        gp4::call_regs(0x0052c689, k);
        eax = k.eax;
        if (k.ebx != 0u) {
            eax = eax * 6u;
            const uint32_t count = k.ecx + eax;
            eax = 1u << (count & 31u);
            mask |= eax;
        }
    }

    r->eax = eax;
    r->ebx = 0x0081d60fu + 6u;
    r->ecx = 0u;
    r->edx = mask;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, 0u));
}
GP4_IMPL(0x0052be64, FUN_0052be64, "regs() -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=captions_0052e131+presets_007ab5b4")
