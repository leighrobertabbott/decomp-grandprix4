#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0054a1d8
// Mirror of 0x0054a18c: copies a rectangle of 16-bit pixels from the SECOND surface (base dword
// at 0x822260) into the first (base dword at 0x822258), row pitch = dword at 0x82224c.
// Inputs: eax = y0, ebx = x0, ecx = y1, ebp = x1 (inclusive; x1 < x0 or y1 < y0, signed, copy
// nothing). All registers are preserved (pushal/popal); the flags are those of the last flag
// writer: `sub ebp, ebx` / `sub ecx, eax` on the early exits, else the final `dec ecx` (row
// counter reaching zero) with CF left over from the `add edi, ebx` row advance.
void __cdecl FUN_0054a1d8(gp4::Regs* r) {
    const uint32_t y0 = r->eax;
    const uint32_t x0 = r->ebx;
    const uint32_t y1 = r->ecx;
    const uint32_t x1 = r->ebp;

    if ((int32_t)x1 < (int32_t)x0) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(x1, x0));
        return;
    }
    if ((int32_t)y1 < (int32_t)y0) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(y1, y0));
        return;
    }

    const uint32_t rowBytes = (x1 - x0 + 1u) * 2u;
    uint32_t rows = y1 - y0 + 1u;
    const uint32_t pitch = GP4_GLOBAL(uint32_t, 0x0082224c);
    const uint32_t base = GP4_GLOBAL(uint32_t, 0x00822258);
    const uint32_t base2 = GP4_GLOBAL(uint32_t, 0x00822260);
    const uint32_t skip = pitch - rowBytes;

    uint32_t dst = y0 * pitch + x0 * 2u + base;
    uint32_t src = dst + base2 - base;
    uint32_t addFlags;

    do {
        for (uint32_t n = rowBytes; n != 0u; --n) {
            *(uint8_t*)dst = *(const uint8_t*)src;
            ++src;
            ++dst;
        }
        src += skip;
        addFlags = gp4::flags::add32(dst, skip);
        dst += skip;
        --rows;
    } while (rows != 0u);

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, addFlags));
}
GP4_IMPL(0x0054a1d8, FUN_0054a1d8, "regs(eax:u32[0..7], ebx:u32[0..15], ebp:rel[ebx,1,-3..15], ecx:rel[eax,1,-3..7]) -> (cf, pf, af, zf, sf, of); globals=layout=surf_0054a12a+surf2_0054a18c")
