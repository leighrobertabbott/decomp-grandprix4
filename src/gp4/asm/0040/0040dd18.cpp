#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Q14 sine/cosine table lookup: the 16-bit angle is folded to its magnitude, then
// word index = (angle >> 2) & 0xfffe.
static inline int32_t tableWord(uint32_t angle16) {
    uint32_t a = angle16 & 0xffffu;
    if ((a & 0x8000u) != 0u) a = (0u - a) & 0xffffu;
    const uint32_t offset = (a >> 2) & 0xfffeu;
    return *reinterpret_cast<const int16_t*>(GP4_ARRAY(const uint8_t, 0x007c5c5c) + offset);
}

// shrd eax, edx, 14 after imul: bits 14..45 of the signed 64-bit product.
static inline uint32_t mulShr14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// FUNCTION: GP4 0x0040dd18
// eax = isqrt((1024 - (Wa+Wb)*cos(t)>>14)^2 + ((Wa-Wb)*sin(t)>>14)^2) << 8, where t is the
// angle word [edi+0x14] scaled by the global 0x006f2bf0, Wa = [edi+0x176], Wb = [edi+0x16].
// ebp, ecx and edx are saved and restored; the flags are those of the final "shl eax, 8".
void __cdecl FUN_0040dd18(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    const int32_t angleWord = GP4_FIELD(const int16_t, body, 0x14);
    const int32_t scale = GP4_GLOBAL(int32_t, 0x006f2bf0);
    const int32_t theta = static_cast<int32_t>(mulShr14(angleWord, scale)) >> 1;

    const int32_t cosine = tableWord(0x4000u - static_cast<uint32_t>(theta));
    const int32_t sine = tableWord(static_cast<uint32_t>(theta));

    const int16_t wa = GP4_FIELD(const int16_t, body, 0x176);
    const int16_t wb = GP4_FIELD(const int16_t, body, 0x16);
    const int32_t sum = static_cast<int16_t>(static_cast<uint16_t>(wa) + static_cast<uint16_t>(wb));
    const int32_t diff = static_cast<int16_t>(static_cast<uint16_t>(wa) - static_cast<uint16_t>(wb));

    const uint32_t u = 0x400u - mulShr14(sum, cosine);
    const uint32_t v = mulShr14(diff, sine);
    r->eax = u * u + v * v;

    gp4::call_regs(0x004010f3, *r);

    const uint32_t value = r->eax;
    const uint32_t result = value << 8;
    const uint32_t flags = ((value >> 24) & 1u)
        | parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((value << 7) ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040dd18, FUN_0040dd18, "regs(edi:ptr[0x17e]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
