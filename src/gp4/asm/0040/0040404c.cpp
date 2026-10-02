#include <gp4/gp4.h>

// 16-bit "test/jns/neg": magnitude of a signed word (0x8000 stays 0x8000).
static inline uint16_t Abs16(uint16_t v) {
    return (v & 0x8000u) ? static_cast<uint16_t>(0u - v) : v;
}

// FUNCTION: GP4 0x0040404c
void __cdecl FUN_0040404c(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t* xf = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));
    const int16_t* table = GP4_ARRAY(const int16_t, 0x007c5c5c);

    const int16_t x = *reinterpret_cast<const int16_t*>(src + 0x90);
    const int16_t y = *reinterpret_cast<const int16_t*>(src + 0x92);
    const uint16_t angle = *reinterpret_cast<const uint16_t*>(xf + 0);

    // Table lookups (word index = magnitude >> 3).
    const int16_t c = table[Abs16(angle) >> 3];
    const int16_t s = static_cast<int16_t>(
        static_cast<uint16_t>(0u - static_cast<uint16_t>(
            table[Abs16(static_cast<uint16_t>(0x4000u - angle)) >> 3])));

    // 16x16 -> 32 signed products, combined with 32-bit wrapping arithmetic.
    const uint32_t p = static_cast<uint32_t>(static_cast<int32_t>(x) * c) -
                       static_cast<uint32_t>(static_cast<int32_t>(y) * s);
    const uint32_t q = static_cast<uint32_t>(static_cast<int32_t>(y) * c) +
                       static_cast<uint32_t>(static_cast<int32_t>(x) * s);

    // High word of (product << 2), arithmetic >> 3, plus the word offsets.
    const int16_t hp = static_cast<int16_t>(static_cast<uint16_t>(p >> 14));
    const int16_t hq = static_cast<int16_t>(static_cast<uint16_t>(q >> 14));
    const int16_t v1 = static_cast<int16_t>(static_cast<uint16_t>(
        (hp >> 3) + *reinterpret_cast<const int16_t*>(xf + 4)));
    const int16_t v2 = static_cast<int16_t>(static_cast<uint16_t>(
        (hq >> 3) + *reinterpret_cast<const int16_t*>(xf + 8)));

    // dx:ax = v:0 shifted right arithmetically by 5 -> (int32)v << 11.
    const uint32_t o1 = static_cast<uint32_t>(static_cast<int32_t>(v1)) << 11;
    const uint32_t o2 = static_cast<uint32_t>(static_cast<int32_t>(v2)) << 11;

    GP4_GLOBAL(int16_t, 0x00934de8) = c;
    GP4_GLOBAL(int16_t, 0x00934dec) = s;
    GP4_GLOBAL(uint32_t, 0x00934df0) = o1;
    GP4_GLOBAL(uint32_t, 0x00934df4) = o2;

    // eax/edx upper halves were cleared by the "and e?x, 0xfffe" index masks.
    const uint16_t lo = static_cast<uint16_t>(o2);
    const uint16_t hi = static_cast<uint16_t>(o2 >> 16);
    r->eax = lo;
    r->edx = hi;

    // Flags: last "sar dx,1" sets SF/ZF/PF (AF=0 in the pinned Unicorn model),
    // then "rcr ax,1" sets CF = old ax bit0 (0) and OF = old ax bit15 ^ CF in.
    uint32_t b = hi & 0xffu;
    b ^= b >> 4;
    b ^= b >> 2;
    b ^= b >> 1;
    const uint32_t pf = (b & 1u) ? 0u : 0x4u;
    const uint32_t zf = (hi == 0) ? 0x40u : 0u;
    const uint32_t sf = (hi & 0x8000u) ? 0x80u : 0u;
    const uint32_t uv = static_cast<uint16_t>(v2);
    const uint32_t of = (((uv >> 3) ^ (uv >> 4)) & 1u) ? 0x800u : 0u;
    r->eflags = (r->eflags & ~0x8d5u) | pf | zf | sf | of;
}
GP4_IMPL(0x0040404c, FUN_0040404c, "regs(edi:ptr[0x10]:bytes, esi:ptr[0x9a]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of)")
