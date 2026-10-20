#include <gp4/gp4.h>
#include <gp4/flags.h>

// One character through the legacy glyph mapping used by the time writers: '-' (0x2d) becomes
// glyph 0x15 and the digits '0'..'9' become glyphs 0x16..0x1f; everything else is kept. The flags
// are those of the last compare/subtract the original executes for this character.
static inline uint8_t MapGlyph(uint8_t al, uint32_t* arith) {
    *arith = gp4::flags::sub8(al, 0x2du);
    if (al == 0x2du) {
        al = 0x15u;
    }
    *arith = gp4::flags::sub8(al, 0x30u);
    if (al < 0x30u) {
        return al;
    }
    *arith = gp4::flags::sub8(al, 0x39u);
    if (al > 0x39u) {
        return al;
    }
    *arith = gp4::flags::sub8(al, 0x1au);
    return static_cast<uint8_t>(al - 0x1au);
}

// Lap-time text writer with leading-zero suppression: converts the time in eax (milliseconds) to
// packed BCD through 0x0051fd6f, stores it at 0x88fde0, then writes "M:" when the minutes digit is
// nonzero, the tens-of-seconds digit when minutes or tens of seconds are nonzero, and always
// "S.hhh" (seconds, '.', three millisecond digits) at edi, in the game's glyph codes. edi is left
// after the text; al is the last glyph (eax's upper bytes keep the packed value) and the flags are
// those of the last glyph conversion.
// FUNCTION: GP4 0x00516ad6
void __cdecl FUN_00516ad6(gp4::Regs* r) {
    gp4::call_regs(0x0051fd6f, *r);
    GP4_GLOBAL(uint32_t, 0x0088fde0) = r->eax;

    uint8_t* out = reinterpret_cast<uint8_t*>(r->edi);
    uint32_t arith = 0;
    uint8_t al = 0;

    al = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x0088fde2) >> 4);
    if (al != 0u) {
        *out++ = MapGlyph(static_cast<uint8_t>((al & 0x0fu) + 0x30u), &arith);
        al = MapGlyph(0x3au, &arith);
        *out++ = al;
    }

    al = GP4_GLOBAL(uint8_t, 0x0088fde2);
    if (al != 0u) {
        al = MapGlyph(static_cast<uint8_t>((al & 0x0fu) + 0x30u), &arith);
        *out++ = al;
    }

    al = MapGlyph(static_cast<uint8_t>(((GP4_GLOBAL(uint8_t, 0x0088fde1) >> 4) & 0x0fu) + 0x30u), &arith);
    *out++ = al;
    al = MapGlyph(0x2eu, &arith);
    *out++ = al;
    al = MapGlyph(static_cast<uint8_t>((GP4_GLOBAL(uint8_t, 0x0088fde1) & 0x0fu) + 0x30u), &arith);
    *out++ = al;
    al = MapGlyph(static_cast<uint8_t>(((GP4_GLOBAL(uint8_t, 0x0088fde0) >> 4) & 0x0fu) + 0x30u), &arith);
    *out++ = al;
    al = MapGlyph(static_cast<uint8_t>((GP4_GLOBAL(uint8_t, 0x0088fde0) & 0x0fu) + 0x30u), &arith);
    *out++ = al;

    r->eax = (r->eax & 0xffffff00u) | al;
    r->edi = reinterpret_cast<uint32_t>(out);
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00516ad6, FUN_00516ad6, "regs(eax:i32[-1000..100000], edi:ptr[0x40]:bytes) -> (eax, edx, edi, cf, pf, af, zf, sf, of)")
