#include <gp4/gp4.h>

struct Unknown_004038a0_Owner {
    uint8_t Unknown00[0x60];
    uint16_t Unknown60;
    uint16_t Unknown62;
    uint8_t Unknown64[0x15c];
    uint16_t Unknown1C0;
    uint16_t Unknown1C2;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// (end - start) * factor >> 14 + start, computed with the 64-bit imul / shrd 14 of the original
static inline uint32_t interpolate(uint32_t start, uint32_t end, int32_t factor) {
    const int32_t delta = static_cast<int32_t>(end - start);
    const int64_t product = static_cast<int64_t>(delta) * static_cast<int64_t>(factor);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14) + start;
}

// FUNCTION: GP4 0x004038a0
void __cdecl FUN_004038a0(gp4::Regs* r) {
    // push ebp/ecx/edx ... pop: only eax and the flags of the final add/sub change.
    const Unknown_004038a0_Owner* owner = reinterpret_cast<const Unknown_004038a0_Owner*>(static_cast<uintptr_t>(r->edi));
    const int32_t factor = static_cast<int32_t>(r->eax);
    const uint32_t offset = GP4_GLOBAL(uint32_t, 0x00852c44);

    uint32_t value;
    uint32_t flags;
    if (static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x00868582)) < 0) {
        // negative byte: offset - interpolated(Unknown60 -> Unknown1C0)
        const uint32_t negated = 0u - interpolate(owner->Unknown60, owner->Unknown1C0, factor);
        value = negated + offset;
        flags = additionFlags32(negated, offset);
    } else {
        // interpolated(Unknown62 -> Unknown1C2) - offset
        const uint32_t interpolated = interpolate(owner->Unknown62, owner->Unknown1C2, factor);
        value = interpolated - offset;
        flags = subtractionFlags32(interpolated, offset);
    }

    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004038a0, FUN_004038a0, "regs(eax:i32, edi:ptr[0x1d0]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
