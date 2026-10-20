#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of "add left, right" on a dword.
static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Low dword of (a * b) >> 14, as "imul; shrd eax, edx, 14" leaves it in eax.
static inline uint32_t mulQ14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// After the per-record step table of 0x0041bb36 (esi), scales three Q14 products with its squared
// outputs 0x00623b70 / 0x00623b74: DAT_00623b0c = DAT_006013a8 * s2, and DAT_00623b10 = DAT_00623ae4
// * s2 + (DAT_00623ae8 + DAT_00623aec) * s3 + DAT_00623af0 (all >> 14). eax = the last shrd result,
// edx = high half of the last product, ecx = the sum, ebp from the callee; the flags are those of
// the final `add ecx, [0x623af0]`.
// FUNCTION: GP4 0x0041bb95
void __cdecl FUN_0041bb95(gp4::Regs* r) {
    gp4::call_regs(0x0041bb36, *r);

    const int32_t angle = GP4_GLOBAL(int32_t, 0x006013a8);
    const int32_t squared = GP4_GLOBAL(int32_t, 0x00623b70);
    GP4_GLOBAL(uint32_t, 0x00623b0c) = mulQ14(angle, squared);

    const uint32_t first = mulQ14(GP4_GLOBAL(int32_t, 0x00623ae4), squared);

    const uint32_t spread = GP4_GLOBAL(uint32_t, 0x00623ae8) + GP4_GLOBAL(uint32_t, 0x00623aec);
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(spread))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00623b74));
    const uint32_t second = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);

    const uint32_t partial = first + second;
    const uint32_t offset = GP4_GLOBAL(uint32_t, 0x00623af0);
    const uint32_t total = partial + offset;
    GP4_GLOBAL(uint32_t, 0x00623b10) = total;

    r->eax = second;
    r->ecx = total;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(partial, offset);
}
GP4_IMPL(0x0041bb95, FUN_0041bb95, "regs(esi:ptr[0x250]:layout=rec_0041bb36) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
