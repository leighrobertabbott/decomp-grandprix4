#include <gp4/gp4.h>

struct UnknownBody_00408fa2 {
    uint8_t Unknown00[0xe];
    int16_t UnknownE;
    uint8_t Unknown10[0x8a];
    int16_t Unknown9A;
    uint8_t Unknown9C[0x49];
    uint8_t UnknownE5;
    uint8_t UnknownE6[0x1a6];
    int32_t Unknown28C;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `left + right` whose result is `result`.
static inline uint32_t addFlags(uint32_t left, uint32_t right, uint32_t result) {
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) >> 20);
}

// Returns the word at +0xe of the body at esi plus a correction, which is zero when bit 0 of the
// byte at +0xe5 is set or the body is the one at DAT_00679be4. Otherwise the correction is
// (min(word(+0x9a) * DAT_006039d4 >> 14, 0x4000) * ((int)(+0x28c * DAT_006033fc >> 24))) >> 14.
// edx is saved and restored; the flags are those of the final "add eax, edx".
// FUNCTION: GP4 0x00408fa2
void __cdecl FUN_00408fa2(gp4::Regs* r) {
    const UnknownBody_00408fa2* body = (const UnknownBody_00408fa2*)r->esi;

    uint32_t correction = 0;
    if ((body->UnknownE5 & 1u) == 0u && r->esi != GP4_GLOBAL(uint32_t, 0x00679be4)) {
        const int64_t first = static_cast<int64_t>(body->Unknown28C)
            * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006033fc));
        const int32_t scaled = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(first) >> 24));

        const int64_t second = static_cast<int64_t>(static_cast<int32_t>(body->Unknown9A))
            * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006039d4));
        int32_t limited = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(second) >> 14));
        if (limited > 0x4000) limited = 0x4000;

        const int64_t third = static_cast<int64_t>(limited) * static_cast<int64_t>(scaled);
        correction = static_cast<uint32_t>(static_cast<uint64_t>(third) >> 14);
    }

    const uint32_t base = static_cast<uint32_t>(static_cast<int32_t>(body->UnknownE));
    const uint32_t result = correction + base;

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(correction, base, result);
}
GP4_IMPL(0x00408fa2, FUN_00408fa2, "regs(esi:ptr[0x294]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
