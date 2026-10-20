#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags (CF PF AF ZF SF OF) of the 32-bit "add left, right".
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Smooths a 12-bit signed word (+0x6a of the record in esi, scaled by 4) through two stages of
// "(delta << 16) / DAT_00603854" (DAT_0060372c holds the previous sample, DAT_00603730 the previous
// rate) and blends the second rate into DAT_00603738: new = (old * (0x4000 - k) >> 14) + (rate * k >> 14)
// with k = DAT_00603740. Leaves ebp = 0x4000 - k, edx = (rate * k) >> 14, eax = the new DAT_00603738
// and the flags of the final "add eax, edx".
// FUNCTION: GP4 0x0041a619
void __cdecl FUN_0041a619(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));

    const uint32_t previousSample = GP4_GLOBAL(uint32_t, 0x0060372c);
    const uint32_t word = *reinterpret_cast<const uint16_t*>(record + 0x6a);
    const int32_t sample = static_cast<int32_t>(word << 20) >> 18;
    GP4_GLOBAL(int32_t, 0x0060372c) = sample;

    int32_t remainder;
    const int32_t delta = static_cast<int32_t>(static_cast<uint32_t>(sample) - previousSample);
    const int32_t rate = gp4::idiv64(static_cast<int64_t>(delta) * 65536, GP4_GLOBAL(int32_t, 0x00603854), &remainder);

    const uint32_t previousRate = GP4_GLOBAL(uint32_t, 0x00603730);
    GP4_GLOBAL(int32_t, 0x00603730) = rate;
    const int32_t rateDelta = static_cast<int32_t>(static_cast<uint32_t>(rate) - previousRate);
    const int32_t acceleration = gp4::idiv64(static_cast<int64_t>(rateDelta) * 65536, GP4_GLOBAL(int32_t, 0x00603854), &remainder);

    const int32_t blend = GP4_GLOBAL(int32_t, 0x00603740);
    const int32_t inverse = static_cast<int32_t>(0x4000u - static_cast<uint32_t>(blend));
    const uint32_t newPart = static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(acceleration) * static_cast<int64_t>(blend)) >> 14);
    const uint32_t oldPart = static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603738)) * static_cast<int64_t>(inverse)) >> 14);
    const uint32_t blended = oldPart + newPart;
    GP4_GLOBAL(uint32_t, 0x00603738) = blended;

    r->eax = blended;
    r->edx = newPart;
    r->ebp = static_cast<uint32_t>(inverse);
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(oldPart, newPart);
}
GP4_IMPL(0x0041a619, FUN_0041a619, "regs(esi:ptr[0x72]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0041a619")
