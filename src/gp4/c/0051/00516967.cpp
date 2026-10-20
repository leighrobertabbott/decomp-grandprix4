#include <gp4/gp4.h>
#include <gp4/flags.h>

// One character of the time text: '-' becomes 0x15 and the digits '0'..'9' are moved down by
// 0x1a into the game font's digit glyphs; flags are those of the last compare/subtract.
static inline uint8_t GlyphOf(uint8_t al, uint32_t& flags) {
    flags = gp4::flags::sub8(al, 0x2du);
    if (al == 0x2du) {
        al = 0x15u;
    }
    flags = gp4::flags::sub8(al, 0x30u);
    if (al < 0x30u) {
        return al;
    }
    flags = gp4::flags::sub8(al, 0x39u);
    if (al > 0x39u) {
        return al;
    }
    flags = gp4::flags::sub8(al, 0x1au);
    return static_cast<uint8_t>(al - 0x1au);
}

static inline uint8_t HighDigit(uint8_t b) { return static_cast<uint8_t>(((b >> 4) & 0xfu) + 0x30u); }
static inline uint8_t LowDigit(uint8_t b) { return static_cast<uint8_t>((b & 0xfu) + 0x30u); }

// Lap-time text "M:SS.cc"-style writer: eax (milliseconds) is converted to packed BCD by 0x51fd6f
// and kept in [0x88fde0]; its digit nibbles (byte +2 high/low, byte +1 high/low, byte +0
// high/low) are written as glyphs to edi with ':' after the first and '.' after the third,
// 8 bytes in all (edi advances by 8). al holds the last glyph; flags are its last compare.
// FUNCTION: GP4 0x00516967
void __cdecl FUN_00516967(gp4::Regs* r) {
    gp4::call_regs(0x0051fd6f, *r);
    GP4_GLOBAL(uint32_t, 0x0088fde0) = r->eax;
    uint8_t* out = reinterpret_cast<uint8_t*>(r->edi);
    uint32_t flags = 0;
    uint8_t al = 0;
    al = GlyphOf(HighDigit(GP4_GLOBAL(uint8_t, 0x0088fde2)), flags);
    out[0] = al;
    al = GlyphOf(0x3au, flags);
    out[1] = al;
    al = GlyphOf(LowDigit(GP4_GLOBAL(uint8_t, 0x0088fde2)), flags);
    out[2] = al;
    al = GlyphOf(HighDigit(GP4_GLOBAL(uint8_t, 0x0088fde1)), flags);
    out[3] = al;
    al = GlyphOf(0x2eu, flags);
    out[4] = al;
    al = GlyphOf(LowDigit(GP4_GLOBAL(uint8_t, 0x0088fde1)), flags);
    out[5] = al;
    al = GlyphOf(HighDigit(GP4_GLOBAL(uint8_t, 0x0088fde0)), flags);
    out[6] = al;
    al = GlyphOf(LowDigit(GP4_GLOBAL(uint8_t, 0x0088fde0)), flags);
    out[7] = al;
    r->eax = (r->eax & 0xffffff00u) | al;
    r->edi += 8u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00516967, FUN_00516967, "regs(eax:i32[-100..599999], edi:ptr[0x10]:bytes) -> (eax, edi, cf, pf, af, zf, sf, of)")
