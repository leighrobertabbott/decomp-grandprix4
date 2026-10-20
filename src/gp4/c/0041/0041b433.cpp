#include <gp4/gp4.h>

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

// (a * b) >> 14 with the full 64-bit product, low 32 bits kept (imul + shrd eax, edx, 14)
static inline uint32_t mulShift14(int32_t a, int32_t b, uint32_t* high) {
    const int64_t product = static_cast<int64_t>(a) * b;
    *high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// FUNCTION: GP4 0x0041b433
// Runs 0x00419220 (esi = the record it expects; it leaves eax/edx/ebp and the weighted
// average in DAT_0060351c), then DAT_0060352c = 0x4000 - min(((max(DAT_0060351c - DAT_00603520, 0))^2 >> 14)
// * DAT_00603524 >> 14, DAT_00603528). Leaves eax = that value, edx = high dword of the last
// product, ebp from the callee and the flags of the final "add eax, 0x4000".
void __cdecl FUN_0041b433(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00419220, c);

    int32_t difference = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0060351c) - GP4_GLOBAL(uint32_t, 0x00603520));
    if (difference < 0) {
        difference = 0;
    }
    uint32_t high;
    const uint32_t squared = mulShift14(difference, difference, &high);
    uint32_t edx;
    uint32_t scaled = mulShift14(static_cast<int32_t>(squared), GP4_GLOBAL(int32_t, 0x00603524), &edx);
    if (static_cast<int32_t>(scaled) > GP4_GLOBAL(int32_t, 0x00603528)) {
        scaled = GP4_GLOBAL(uint32_t, 0x00603528);
    }
    const uint32_t negated = 0u - scaled;
    const uint32_t result = negated + 0x4000u;
    GP4_GLOBAL(uint32_t, 0x0060352c) = result;

    r->eax = result;
    r->edx = edx;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(negated, 0x4000u);
}
GP4_IMPL(0x0041b433, FUN_0041b433, "regs(esi:ptr[0x2fc]:layout=rec_00419220) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_00419220")
