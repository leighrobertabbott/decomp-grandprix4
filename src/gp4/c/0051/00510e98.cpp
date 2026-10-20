#include <gp4/gp4.h>
#include <gp4/flags.h>

// Writes the byte in al as three decimal-ish characters at [edi] through the character writer
// 0x0051f3bd: first the hundreds digit (al is reduced by 100 until it is below 100 and the count of
// reductions plus '0' is written), or a blank (0x20) when al is already below 100; then 0x00510e71
// writes the remaining value (as ah:al, packed BCD) as two more characters. The first and last `push
// eax`/`pop eax` pairs restore eax, so it comes back unchanged; the other registers and the flags are
// what the last callee leaves.
// FUNCTION: GP4 0x00510e98
void __cdecl FUN_00510e98(gp4::Regs* r) {
    const uint32_t original = r->eax;
    uint32_t al = original & 0xffu;
    uint32_t ah = 0u;
    uint32_t printed = 0x20u;
    if (al >= 0x64u) {
        do {
            al = (al - 0x64u) & 0xffu;
            ah = (ah + 1u) & 0xffu;
        } while (al >= 0x64u);
        printed = (ah + 0x30u) & 0xffu;
    }
    const uint32_t packed = (original & 0xffff0000u) | (ah << 8) | al;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eax = (packed & 0xffffff00u) | printed;
    gp4::call_regs(0x0051f3bd, c);
    c.eax = packed;
    gp4::call_regs(0x00510e71, c);
    c.eax = original;
    *r = c;
}
GP4_IMPL(0x00510e98, FUN_00510e98, "regs(eax:u32, edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187+glyph_0051d329")
