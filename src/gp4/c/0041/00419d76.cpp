#include <gp4/gp4.h>

// The four 0x300-byte records at 0x006380d8 (only the vector this routine reads).
struct Unknown_006380d8 {
    uint8_t Unknown00[0x80];
    int32_t Unknown80;
    int32_t Unknown84;
    int32_t Unknown88;
    uint8_t Unknown8C[0x300 - 0x8c];
};
static_assert(sizeof(Unknown_006380d8) == 0x300, "record stride");

// imul (edx:eax = left * right) followed by shrd eax, edx, 12: the low dword of the product >> 12.
static inline uint32_t fixed12(uint32_t left, uint32_t right, uint32_t* high) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(left)) * static_cast<int64_t>(static_cast<int32_t>(right));
    *high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 12);
}

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
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

// FUNCTION: GP4 0x00419d76
// Combines the vectors (Unknown80/84/88) of the four records at 0x006380d8 with the nine Q12
// coefficients DAT_00638d6c..8c into DAT_006228ec, DAT_006228f0 and DAT_006228e8. Each term is
// (sum * coefficient) >> 12 with the low dword of the product. Leaves ebx = 0x006380d8,
// eax = DAT_006228e8, ebp = the partial sum, edx = the high dword of the last imul and the flags of
// the final add.
void __cdecl FUN_00419d76(gp4::Regs* r) {
    const Unknown_006380d8* rec = GP4_ARRAY(Unknown_006380d8, 0x006380d8);
    uint32_t high = 0;

    uint32_t ebp = fixed12(rec[0].Unknown80 + rec[1].Unknown80, GP4_GLOBAL(uint32_t, 0x00638d6c), &high);
    ebp -= fixed12(rec[2].Unknown80 + rec[3].Unknown80, GP4_GLOBAL(uint32_t, 0x00638d70), &high);
    ebp -= fixed12(rec[2].Unknown88 + rec[0].Unknown88, GP4_GLOBAL(uint32_t, 0x00638d84), &high);
    uint32_t eax = fixed12(rec[3].Unknown88 + rec[1].Unknown88, GP4_GLOBAL(uint32_t, 0x00638d84), &high) + ebp;
    GP4_GLOBAL(uint32_t, 0x006228ec) = eax;

    ebp = fixed12(rec[0].Unknown84 - rec[1].Unknown84, GP4_GLOBAL(uint32_t, 0x00638d7c), &high);
    ebp += fixed12(rec[2].Unknown84 - rec[3].Unknown84, GP4_GLOBAL(uint32_t, 0x00638d80), &high);
    eax = 0u - fixed12(rec[0].Unknown80 + rec[1].Unknown80 + rec[2].Unknown80 + rec[3].Unknown80,
                       GP4_GLOBAL(uint32_t, 0x00638d88), &high);
    eax += ebp;
    GP4_GLOBAL(uint32_t, 0x006228f0) = eax;

    ebp = 0u - fixed12(rec[0].Unknown84 + rec[1].Unknown84, GP4_GLOBAL(uint32_t, 0x00638d74), &high);
    ebp += fixed12(rec[2].Unknown84 + rec[3].Unknown84, GP4_GLOBAL(uint32_t, 0x00638d78), &high);
    const uint32_t last = fixed12(rec[0].Unknown88 + rec[1].Unknown88 + rec[2].Unknown88 + rec[3].Unknown88,
                                  GP4_GLOBAL(uint32_t, 0x00638d8c), &high);
    eax = last + ebp;
    GP4_GLOBAL(uint32_t, 0x006228e8) = eax;

    r->eax = eax;
    r->ebx = 0x006380d8u;
    r->edx = high;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(last, ebp);
}
GP4_IMPL(0x00419d76, FUN_00419d76, "regs() -> (eax,ebx,edx,ebp,cf,pf,af,zf,sf,of); globals=fuzz:i32+layout=dw_00419d76")
