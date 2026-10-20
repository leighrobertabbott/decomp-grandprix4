#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `or dx,dx` / `test dx,0xffff`: CF = OF = AF = 0.
static inline uint32_t logicFlags16(uint32_t value) {
    value &= 0xffffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value >> 8) & 0x80u);
}

// Flags of `neg dword` (the subtraction 0 - value).
static inline uint32_t negFlags32(uint32_t value) {
    const uint32_t result = 0u - value;
    return (value != 0u ? 1u : 0u)
        | parityFlag(result)
        | ((value ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | (value == 0x80000000u ? 0x800u : 0u);
}

static inline uint16_t abs16(uint16_t value) {
    return static_cast<int16_t>(value) < 0 ? static_cast<uint16_t>(0u - value) : value;
}

// Word of the Q14 cosine table at |angle| >> 2 (byte offset, even).
static inline uint16_t tableWord(uint16_t angle) {
    const uint32_t offset = (abs16(angle) >> 2) & 0xfffeu;
    return *reinterpret_cast<const uint16_t*>(GP4_ARRAY(const uint8_t, 0x007c5c5c) + offset);
}

// FUNCTION: GP4 0x0040a006
// Sine/cosine pair of the 16-bit angle word at [0x934de8] (0x4000 = a quarter turn). The routine
// 0x004011e5 interpolates the table into a 32-bit value; 0x0040a115 turns a value v into
// sqrt(2^60 - v^2). When |table(0x4000 - angle)| < 0x2000 the sine is taken from the complement and the
// two dwords are swapped at the end ([0x934de8] = root, [0x934dec] = interpolated value); otherwise
// [0x934de8] = interpolated(angle) and [0x934dec] = root of that. The root's sign follows the table word
// of the angle (resp. its complement). The leftover registers: eax = the 0x0040a115 leftover (or the low
// root word), edx = the sign table word (or the high root word).
void __cdecl FUN_0040a006(gp4::Regs* r) {
    const uint16_t angle = GP4_GLOBAL(uint16_t, 0x00934de8);
    const uint16_t complement = static_cast<uint16_t>(0x4000u - angle);
    uint16_t bound = tableWord(complement);
    if (static_cast<int16_t>(bound) < 0) {
        bound = static_cast<uint16_t>(0u - bound);
    }
    const bool useComplement = static_cast<int16_t>(bound) < 0x2000;

    gp4::Regs interpolate = *r;
    interpolate.st_in = 0;
    interpolate.st_out = 0;
    interpolate.edx = (r->edx & 0xffff0000u) | angle;
    uint16_t signWord;
    if (useComplement) {
        GP4_GLOBAL(uint16_t, 0x00934de8) = complement;
        interpolate.eax = GP4_GLOBAL(uint16_t, 0x00934de8);
        signWord = tableWord(angle);
    } else {
        interpolate.eax = GP4_GLOBAL(uint16_t, 0x00934de8);
        signWord = tableWord(complement);
    }
    gp4::call_regs(0x004011e5, interpolate);
    GP4_GLOBAL(uint16_t, 0x00934de8) = static_cast<uint16_t>(interpolate.eax);
    GP4_GLOBAL(uint16_t, 0x00934dea) = static_cast<uint16_t>(interpolate.edx);

    gp4::Regs root = *r;
    root.st_in = 0;
    root.st_out = 0;
    root.eax = bound;
    root.edx = signWord;
    gp4::call_regs(0x0040a115, root);

    uint32_t flags = logicFlags16(signWord);
    if (static_cast<int16_t>(signWord) < 0) {
        const uint32_t value = GP4_GLOBAL(uint32_t, 0x00934dec);
        GP4_GLOBAL(uint32_t, 0x00934dec) = 0u - value;
        flags = negFlags32(value);
    }

    if (useComplement) {
        const uint16_t valueLow = GP4_GLOBAL(uint16_t, 0x00934de8);
        const uint16_t valueHigh = GP4_GLOBAL(uint16_t, 0x00934dea);
        const uint16_t rootLow = GP4_GLOBAL(uint16_t, 0x00934dec);
        const uint16_t rootHigh = GP4_GLOBAL(uint16_t, 0x00934dee);
        GP4_GLOBAL(uint16_t, 0x00934dec) = valueLow;
        GP4_GLOBAL(uint16_t, 0x00934dee) = valueHigh;
        GP4_GLOBAL(uint16_t, 0x00934de8) = rootLow;
        GP4_GLOBAL(uint16_t, 0x00934dea) = rootHigh;
        r->eax = (root.eax & 0xffff0000u) | rootLow;
        r->edx = rootHigh;
    } else {
        r->eax = root.eax;
        r->edx = signWord;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040a006, FUN_0040a006, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
