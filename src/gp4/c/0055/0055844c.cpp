#include <gp4/gp4.h>
#include <gp4/flags.h>

// R3 stream decoder for a clamp record (active only in mode 1: [0x0086857c] == 1). esi points at
// seven words: first and second are clamped into 0..255 (second raised to at least first), the
// first goes to [0x00933a62] (word), the span (second + 1 - first, at least 1) to [0x007c4d51],
// and the six following words are stored as bytes at 0x007c4d52... Any other mode just skips the
// 0x10-byte record (esi += 0x10, flags of that add). In mode 1 ax ends as the last word read, dx as
// the span, ecx as 0, esi 0x10 past the start; the flags are those of `inc edi` (edi = 0x007c4d51) keeping CF of `cmp dx, 1`.
// FUNCTION: GP4 0x0055844c
void __cdecl FUN_0055844c(gp4::Regs* r) {
    const uint32_t src = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 1u) {
        r->esi = src + 0x10u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(src, 0x10u));
        return;
    }

    int16_t lo = GP4_FIELD(int16_t, src, 0);
    int16_t hi = GP4_FIELD(int16_t, src, 2);
    if (lo < 0) lo = 0;
    if (lo > 0xff) lo = 0xff;
    GP4_GLOBAL(int16_t, 0x00933a62) = lo;
    if (hi < lo) hi = lo;
    if (hi > 0xff) hi = 0xff;
    hi = static_cast<int16_t>(hi + 1);
    hi = static_cast<int16_t>(hi - lo);
    const uint32_t cmp_flags = gp4::flags::sub16(static_cast<uint16_t>(hi), 1u);  // then `inc edi` rewrites all but CF
    if (hi < 1) hi = 1;

    uint8_t* dst = GP4_ARRAY(uint8_t, 0x007c4d51);
    *dst++ = static_cast<uint8_t>(hi);
    const uint16_t* words = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(src + 4u));
    uint16_t last = 0;
    for (int i = 0; i < 6; ++i) {
        last = *words++;
        *dst++ = static_cast<uint8_t>(last);
    }

    r->eax = (r->eax & 0xffff0000u) | last;
    r->edx = (r->edx & 0xffff0000u) | static_cast<uint16_t>(hi);
    r->ecx = 0u;
    r->esi = src + 0x10u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(0x007c4d51u, cmp_flags));
}
GP4_IMPL(0x0055844c, FUN_0055844c, "regs(esi:ptr[0x20]:i16[-300..600], eax:u32, edx:u32) -> (eax, edx, ecx, esi, cf, pf, af, zf, sf, of); globals=layout=mode_0086857c")
