#include <gp4/gp4.h>
#include <gp4/flags.h>

// One character through the legacy glyph mapping used by the time writers: '-' (0x2d) becomes glyph 0x15 and
// the digits '0'..'9' become glyphs 0x16..0x1f; everything else is kept.
static inline uint8_t MapGlyph(uint8_t al) {
    if (al == 0x2du) {
        al = 0x15u;
    }
    if (al < 0x30u) {
        return al;
    }
    if (al > 0x39u) {
        return al;
    }
    return static_cast<uint8_t>(al - 0x1au);
}

// Dash placeholder for a lap time that is not available (>= 10:00.000): the eight characters "-:--.---"
// converted to glyph codes and stored at edi (the same glyph mapping the time writers use). edi is left after
// the text, al is the last glyph (0x15) and the flags are those of that glyph's `cmp al, 0x30`.
// FUNCTION: GP4 0x00516a3d
void __cdecl FUN_00516a3d(gp4::Regs* r) {
    uint8_t* out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint8_t al = 0u;

    *out++ = MapGlyph(0x2du);
    *out++ = MapGlyph(0x3au);
    *out++ = MapGlyph(0x2du);
    *out++ = MapGlyph(0x2du);
    *out++ = MapGlyph(0x2eu);
    *out++ = MapGlyph(0x2du);
    *out++ = MapGlyph(0x2du);
    al = MapGlyph(0x2du);
    *out++ = al;

    r->eax = (r->eax & 0xffffff00u) | al;
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(out));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(al, 0x30u));
}
GP4_IMPL(0x00516a3d, FUN_00516a3d, "regs(eax:u32, edi:ptr[0x10]:bytes) -> (eax, edi, cf, pf, af, zf, sf, of)")
