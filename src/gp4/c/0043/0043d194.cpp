#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of the 32-bit `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// (value * factor) >> 14 as `imul dword [factor]; shrd eax, edx, 14` leaves it in eax.
static inline uint32_t scaleQ14(int32_t value, int32_t factor) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(value) * static_cast<int64_t>(factor)) >> 14);
}

// FUNCTION: GP4 0x0043d194
// Resets the suspension-style words of the car record at esi: +0x10e takes the Q14 factor of
// 0x0043ced2, +0x10a becomes 0x4000, bit 1 of +0xff is cleared and byte +0x295 takes the pit-lane
// test of the segment [esi+0x1c] (0x00403aa4). Then +0x108 / +0x1ac become table words (index word
// 0x007ad89e, tables 0x006223ca / 0x006223d4) plus an offset scaled by (0x4000 - factor) with the
// gains 0x006223f8 / 0x006223fc, and +0x182 is word +0xf6 rescaled around 0x4000 by 0x006223bc.
// pushal/popal restore every register; popal does not restore the flags, which are those of the final
// `add eax, 0x4000`.
void __cdecl FUN_0043d194(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x0043ced2, call);
    GP4_FIELD(uint16_t, car, 0x10e) = static_cast<uint16_t>(call.eax);

    const uint32_t index = GP4_GLOBAL(uint16_t, 0x007ad89e);
    GP4_FIELD(uint16_t, car, 0x10a) = 0x4000u;
    GP4_FIELD(uint8_t, car, 0xff) = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0xff) & 0xfdu);

    call.edi = GP4_FIELD(uint32_t, car, 0x1c);
    gp4::call_regs(0x00403aa4, call);
    GP4_FIELD(uint8_t, car, 0x295) = static_cast<uint8_t>(call.eax);

    int32_t factor = 0x4000 - static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x10e));
    uint32_t value = 0u - scaleQ14(factor, GP4_GLOBAL(int32_t, 0x006223f8));
    value += static_cast<uint32_t>(static_cast<int32_t>(GP4_ARRAY(int16_t, 0x006223ca)[index]));
    value += GP4_GLOBAL(uint16_t, 0x0062d20c);
    GP4_FIELD(uint16_t, car, 0x108) = static_cast<uint16_t>(value);

    factor = 0x4000 - static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x10e));
    value = scaleQ14(factor, GP4_GLOBAL(int32_t, 0x006223fc));
    value += static_cast<uint32_t>(static_cast<int32_t>(GP4_ARRAY(int16_t, 0x006223d4)[index]));
    value += GP4_GLOBAL(uint16_t, 0x0062d20e);
    GP4_FIELD(uint16_t, car, 0x1ac) = static_cast<uint16_t>(value);

    const int32_t centred = static_cast<int32_t>(GP4_FIELD(uint16_t, car, 0xf6)) - 0x4000;
    const uint32_t scaled = scaleQ14(centred, GP4_GLOBAL(int32_t, 0x006223bc));
    GP4_FIELD(uint16_t, car, 0x182) = static_cast<uint16_t>(scaled + 0x4000u);

    r->eflags = (r->eflags & ~0x8d5u) | addFlags(scaled, 0x4000u);
}
GP4_IMPL(0x0043d194, FUN_0043d194, "regs(edx:u32, esi:ptr[0x29c]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0066d8c0+words_0062d210+state_0042595c")
