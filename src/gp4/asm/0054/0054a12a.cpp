#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0054a12a
// Copies a rectangle of 16-bit pixels from a packed source (esi, rows of (x1-x0+1)*2 bytes, no
// padding) into the surface whose base is the dword at 0x822258 and whose pitch (bytes per row)
// is the dword at 0x82224c. Inputs: eax = y0, ebx = x0, ecx = y1, ebp = x1 (inclusive).
// All registers are preserved (pushal/popal); the flags are those of the final `dec edx`
// (row counter reaching zero) with CF left over from the `add edi, ebx` row advance.
void __cdecl FUN_0054a12a(gp4::Regs* r) {
    const uint32_t y0 = r->eax;
    const uint32_t x0 = r->ebx;
    const uint32_t rowBytes = (r->ebp - x0 + 1u) * 2u;
    uint32_t rows = r->ecx - y0 + 1u;
    const uint32_t pitch = GP4_GLOBAL(uint32_t, 0x0082224c);
    const uint32_t base = GP4_GLOBAL(uint32_t, 0x00822258);
    const uint32_t skip = pitch - rowBytes;

    const uint8_t* src = (const uint8_t*)r->esi;
    uint32_t dst = y0 * pitch + x0 * 2u + base;
    uint32_t addFlags;

    do {
        for (uint32_t n = rowBytes; n != 0u; --n) {
            *(uint8_t*)dst = *src;
            ++src;
            ++dst;
        }
        addFlags = gp4::flags::add32(dst, skip);
        dst += skip;
        --rows;
    } while (rows != 0u);

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, addFlags));
}
GP4_IMPL(0x0054a12a, FUN_0054a12a, "regs(eax:u32[0..7], ebx:u32[0..15], ebp:rel[ebx,1,0..15], ecx:rel[eax,1,0..7], esi:ptr[0x108]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=surf_0054a12a")
