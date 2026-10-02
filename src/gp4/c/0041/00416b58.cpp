#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of add left, right
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (result < left ? 1u : 0u);
}

// imul (signed 64-bit product) followed by shrd eax, edx, count
static inline uint32_t mulShrd(int32_t a, int32_t b, unsigned count) {
    const int64_t product = (int64_t)a * (int64_t)b;
    return (uint32_t)((uint64_t)product >> count);
}

// FUNCTION: GP4 0x00416b58
void __cdecl FUN_00416b58(gp4::Regs* r) {
    uint8_t* base = GP4_ARRAY(uint8_t, 0x006380d8);
    const uint32_t center = GP4_GLOBAL(uint32_t, 0x00638cec);
    const uint32_t scaled = mulShrd((int32_t)GP4_GLOBAL(uint32_t, 0x00623ebc),
                                    (int32_t)GP4_GLOBAL(uint32_t, 0x006f2bec), 0x16);

    uint32_t offset = mulShrd((int32_t)scaled, (int32_t)GP4_GLOBAL(uint32_t, 0x0060133c), 0xe);
    GP4_FIELD(uint32_t, base, 0x564) = offset + center;
    GP4_FIELD(uint32_t, base, 0x264) = 0u - offset + center;

    offset = mulShrd((int32_t)scaled, (int32_t)GP4_GLOBAL(uint32_t, 0x00601344), 0xe);
    const uint32_t upper = offset + center;
    const uint32_t lower = 0u - offset + center;
    GP4_FIELD(uint32_t, base, 0xb64) = upper;
    GP4_FIELD(uint32_t, base, 0x864) = lower;

    r->ebx = reinterpret_cast<uint32_t>(base);
    r->ecx = center;
    r->ebp = scaled;
    r->eax = upper;
    r->edx = lower;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(0u - offset, center);
}
GP4_IMPL(0x00416b58, FUN_00416b58, "regs() -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
