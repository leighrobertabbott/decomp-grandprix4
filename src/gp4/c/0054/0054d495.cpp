#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0054d495
// Address of a screen position in a 2D surface: adds the incoming ebx to the running offset
// [0x00833cc5] and builds edi = (base + [0x00833f80]) * scale + origin + 2 * offset, where the
// base is [0x00833cc9] (masked to its low bit when the flag byte [0x00833cd6] is set) and
// scale/origin come from 0x007a8e7a/0x007a8e6a (flag set) or 0x0082224c/0x00822258 (clear).
// eax is left as the unscaled base + [0x00833f80], ebx as the new offset; the flags are those of
// the closing `add eax, ebx`.
void __cdecl FUN_0054d495(gp4::Regs* r) {
    gp4::cdecl_<0x0054d6f1, void>();

    const uint32_t offset = GP4_GLOBAL(uint32_t, 0x00833cc5) + r->ebx;
    GP4_GLOBAL(uint32_t, 0x00833cc5) = offset;

    uint32_t base = GP4_GLOBAL(uint32_t, 0x00833cc9);
    uint32_t scale;
    uint32_t origin;
    if (GP4_GLOBAL(uint8_t, 0x00833cd6) != 0u) {
        base &= 1u;
        base += GP4_GLOBAL(uint32_t, 0x00833f80);
        scale = GP4_GLOBAL(uint32_t, 0x007a8e7a);
        origin = GP4_GLOBAL(uint32_t, 0x007a8e6a);
    } else {
        base += GP4_GLOBAL(uint32_t, 0x00833f80);
        scale = GP4_GLOBAL(uint32_t, 0x0082224c);
        origin = GP4_GLOBAL(uint32_t, 0x00822258);
    }
    const uint32_t twice = offset + offset;
    const uint32_t partial = base * scale + origin;

    r->edi = partial + twice;
    r->eax = base;
    r->ebx = offset;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(partial, twice));
}
GP4_IMPL(0x0054d495, FUN_0054d495, "regs(ebx:u32) -> (eax, ebx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
