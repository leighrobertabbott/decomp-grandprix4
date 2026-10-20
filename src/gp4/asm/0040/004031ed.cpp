#include <gp4/gp4.h>

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
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x004031ed
void __cdecl FUN_004031ed(gp4::Regs* r) {
    // ebp and edx are pushed and popped: eax and the flags of the final `add` are the outputs.
    // esi is a byte offset into the 16-byte-stride tables at 0x868598 and 0x86c598 (the two
    // interpolation endpoints are the dwords at +0 and +4 of each entry).
    const uint32_t offset = r->esi;
    GP4_GLOBAL(uint32_t, 0x00601f18) = 0u;
    GP4_GLOBAL(uint8_t, 0x00601eac) = 0xffu;

    // fraction = (clamp(ecx - start, 0, span) << 14) / span, span = end - start (1 when zero)
    const uint32_t start = GP4_FIELD(uint32_t, GP4_ARRAY(uint8_t, 0x00868598), offset);
    int32_t span = static_cast<int32_t>(GP4_FIELD(uint32_t, GP4_ARRAY(uint8_t, 0x0086859c), offset) - start);
    if (span == 0) span = 1;
    int32_t position = static_cast<int32_t>(r->ecx - start);
    if (position < 0) position = 0;
    if (position > span) position = span;
    int32_t remainder;
    const int32_t fraction = gp4::idiv64(static_cast<int64_t>(position) << 14, span, &remainder);
    GP4_GLOBAL(int32_t, 0x00627844) = fraction;

    // value = (fraction * ((end2 - start2) << 8) >> 14) + (start2 << 8)
    const uint32_t start2 = GP4_FIELD(uint32_t, GP4_ARRAY(uint8_t, 0x0086c598), offset);
    const int32_t delta = static_cast<int32_t>((GP4_FIELD(uint32_t, GP4_ARRAY(uint8_t, 0x0086c59c), offset) - start2) << 8);
    const uint64_t product = static_cast<uint64_t>(static_cast<int64_t>(fraction) * static_cast<int64_t>(delta));
    const uint32_t scaled = static_cast<uint32_t>(product >> 14);
    const uint32_t base = start2 << 8;
    const uint32_t result = scaled + base;

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(scaled, base, result);
}
GP4_IMPL(0x004031ed, FUN_004031ed, "regs(ecx:i32, esi:u32[0..0x3f0]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
