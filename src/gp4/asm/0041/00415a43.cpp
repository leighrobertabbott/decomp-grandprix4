#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of "cmp left, right" (32-bit).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Arithmetic flags of "cmp byte, imm" (8-bit).
static inline uint32_t subFlags8(uint8_t left, uint8_t right) {
    const uint8_t result = static_cast<uint8_t>(left - right);
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Arithmetic flags of "add left, right" (32-bit).
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((~(left ^ right)) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// (value * DAT_00603854) >> 16 as "imul; shrd eax, edx, 16" leaves it in eax
static inline uint32_t scaleQ16(int32_t value) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(value) * GP4_GLOBAL(int32_t, 0x00603854)) >> 16);
}

// Limits |value| (unsigned compare) to limit and keeps the sign.
static inline uint32_t clampMagnitude(uint32_t value, uint32_t limit) {
    if (static_cast<int32_t>(value) < 0) {
        uint32_t magnitude = 0u - value;
        if (!(magnitude < limit)) magnitude = limit;
        return 0u - magnitude;
    }
    return value < limit ? value : limit;
}

// FUNCTION: GP4 0x00415a43
// Moves the 16-bit value at [esi+0x62] towards the target DAT_00603a44 by at most a step
// (DAT_00603a40 or DAT_00603a3c scaled by DAT_00603854 >> 16) and returns the new value in eax.
// A byte [esi+0x72] above 1 leaves eax = DAT_00603a44 (flags of that compare). Once the value
// has reached the target, DAT_00603a78 = 0xff and, with [esi+0x78] bit 6 set, a first try towards
// word[esi+0x264] - 1000 is returned when it lies above the target. Otherwise bit 6 of [esi+0x78]
// is cleared and eax = clamp(target - value, +-step) + value (the flags of that add).
// ebp and edx are saved and restored.
void __cdecl FUN_00415a43(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t target = GP4_GLOBAL(uint32_t, 0x00603a44);

    if (static_cast<int8_t>(GP4_FIELD(uint8_t, rec, 0x72)) > 1) {
        r->eax = target;
        r->eflags = (r->eflags & ~0x8d5u) | subFlags8(GP4_FIELD(uint8_t, rec, 0x72), 1u);
        return;
    }

    const uint32_t current = GP4_FIELD(uint16_t, rec, 0x62);
    uint32_t step;
    if (static_cast<int32_t>(current) < static_cast<int32_t>(target)) {
        step = scaleQ16(GP4_GLOBAL(int32_t, 0x00603a40));
    } else {
        GP4_GLOBAL(uint8_t, 0x00603a78) = 0xff;
        if ((GP4_FIELD(uint8_t, rec, 0x78) & 0x40u) != 0u) {
            step = scaleQ16(GP4_GLOBAL(int32_t, 0x00603a3c));
            const uint32_t toLimit = static_cast<uint32_t>(GP4_FIELD(uint16_t, rec, 0x264)) - 0x3e8u - current;
            const uint32_t early = clampMagnitude(toLimit, step) + current;
            if (static_cast<int32_t>(early) > static_cast<int32_t>(target)) {
                r->eax = early;
                r->eflags = (r->eflags & ~0x8d5u) | subFlags32(early, target);
                return;
            }
        }
        step = scaleQ16(GP4_GLOBAL(int32_t, 0x00603a3c));
    }

    GP4_FIELD(uint8_t, rec, 0x78) = static_cast<uint8_t>(GP4_FIELD(uint8_t, rec, 0x78) & 0xbfu);
    const uint32_t delta = clampMagnitude(GP4_GLOBAL(uint32_t, 0x00603a44) - current, step);
    r->eax = delta + current;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(delta, current);
}
GP4_IMPL(0x00415a43, FUN_00415a43, "regs(esi:ptr[0x26c]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
