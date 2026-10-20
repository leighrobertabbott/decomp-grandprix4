#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `sub`/`neg` (neg is 0 - value).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Flags of "test value, -1": CF = OF = AF = 0.
static inline uint32_t testFlags32(uint32_t value) {
    return parityFlag(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

// |q| - (|q| >> 4) + 0x40 >> 7, negated again for negative q: q / 128 * 15/16 rounded, i.e. a
// 14-bit unit-vector component scaled to a signed byte. `flagsOut` receives the flags of the
// "test q / neg" pair that ends the original sequence.
static inline uint32_t scaleToByte(int32_t q, uint32_t* flagsOut) {
    uint32_t magnitude = static_cast<uint32_t>(q);
    if (q < 0) magnitude = 0u - magnitude;
    const int32_t m = static_cast<int32_t>(magnitude);
    int32_t scaled = static_cast<int32_t>(static_cast<uint32_t>(m) - static_cast<uint32_t>(m >> 4) + 0x40u) >> 7;
    if (q < 0) {
        *flagsOut = subFlags32(0u, static_cast<uint32_t>(scaled));
        scaled = static_cast<int32_t>(0u - static_cast<uint32_t>(scaled));
    } else {
        *flagsOut = testFlags32(static_cast<uint32_t>(q));
    }
    return static_cast<uint32_t>(scaled) & 0xffu;
}

// Normalises (eax, ebx, ecx) with 0x004388ac and packs the result into eax as four bytes:
// min(length >> 15, 0xff) in the top byte, then the scaled z, y and x components. Every other
// register is saved and restored; the flags are those of the sequence for the x component.
// FUNCTION: GP4 0x00438901
void __cdecl FUN_00438901(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x004388ac, call);

    const int32_t x = static_cast<int32_t>(call.eax);
    const int32_t y = static_cast<int32_t>(call.ebx);
    const int32_t z = static_cast<int32_t>(call.ecx);

    uint32_t lengthByte = call.ebp >> 15;
    if (static_cast<int32_t>(lengthByte) > 0xff) lengthByte = 0xff;

    uint32_t flags;
    uint32_t discarded;
    const uint32_t zByte = scaleToByte(z, &discarded);
    const uint32_t yByte = scaleToByte(y, &discarded);
    const uint32_t xByte = scaleToByte(x, &flags);

    r->eax = (lengthByte << 24) | (zByte << 16) | (yByte << 8) | xByte;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00438901, FUN_00438901, "regs(eax:i32[-0x1000000..0x1000000], ebx:i32[-0x1000000..0x1000000], ecx:i32[-0x1000000..0x1000000]) -> (eax, cf, pf, af, zf, sf, of)")
