#include <gp4/gp4.h>

// Two of the four 0x300-byte records at 0x006380d8 (only the fields this routine touches).
struct Unknown_006380d8 {
    uint8_t Unknown00[0x1c];
    int32_t Unknown1C;
    uint8_t Unknown20[0x40 - 0x20];
    int32_t Unknown40;
    uint8_t Unknown44[0x300 - 0x44];
};
static_assert(sizeof(Unknown_006380d8) == 0x300, "record stride");

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// cmp left, right (32-bit) arithmetic flags.
static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// imul (edx:eax = left * right) followed by shrd eax, edx, 14: the low dword of the product >> 14.
static inline uint32_t fixed14(int32_t left, int32_t right) {
    const int64_t product = static_cast<int64_t>(left) * static_cast<int64_t>(right);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// test eax, eax / jns / neg eax
static inline int32_t magnitude(int32_t value) {
    return value < 0 ? static_cast<int32_t>(0u - static_cast<uint32_t>(value)) : value;
}

// FUNCTION: GP4 0x0041c027
// Two 2.14 fixed-point terms feeding DAT_00623b8c:
//   v = DAT_0062474c * DAT_00623b68 >> 14 -> DAT_00623b9c; DAT_00623bd0 = (v >= 0) ? 0xff : 0
//   DAT_00623bbc = v * (DAT_00623bc4 | DAT_00623bcc) >> 14   (first when v >= 0)
//   DAT_00623bb8 = (rec1.1c + rec1.40 - rec0.1c - rec0.40) * (DAT_00623bc0 | DAT_00623bc8) >> 14
//   DAT_00623bf0 = 1 (v >= 0), 0 (DAT_00638d30 < 0x3000) or -1 selecting the weight pair (ebp, ecx)
//   DAT_00623b8c = min(|DAT_00623bb8| * ebp >> 14 + |DAT_00623bbc| * ecx >> 14, DAT_006013bc)
// ebx and ecx are saved and restored; eax is the stored value, edx the first term, ebp the weight
// and the flags those of the final compare against DAT_006013bc.
void __cdecl FUN_0041c027(gp4::Regs* r) {
    Unknown_006380d8* records = GP4_ARRAY(Unknown_006380d8, 0x006380d8);

    const int32_t angle = static_cast<int32_t>(fixed14(GP4_GLOBAL(int32_t, 0x0062474c), GP4_GLOBAL(int32_t, 0x00623b68)));
    GP4_GLOBAL(int32_t, 0x00623b9c) = angle;
    const bool forward = angle >= 0;
    GP4_GLOBAL(uint8_t, 0x00623bd0) = forward ? 0xffu : 0u;

    const int32_t sideTerm = static_cast<int32_t>(
        fixed14(angle, forward ? GP4_GLOBAL(int32_t, 0x00623bc4) : GP4_GLOBAL(int32_t, 0x00623bcc)));
    GP4_GLOBAL(int32_t, 0x00623bbc) = sideTerm;

    const int32_t offset = static_cast<int32_t>(static_cast<uint32_t>(records[1].Unknown1C)
        + static_cast<uint32_t>(records[1].Unknown40) - static_cast<uint32_t>(records[0].Unknown1C)
        - static_cast<uint32_t>(records[0].Unknown40));
    const int32_t frontTerm = static_cast<int32_t>(
        fixed14(offset, forward ? GP4_GLOBAL(int32_t, 0x00623bc0) : GP4_GLOBAL(int32_t, 0x00623bc8)));
    GP4_GLOBAL(int32_t, 0x00623bb8) = frontTerm;

    uint32_t selector;
    uint32_t ebp;
    uint32_t ecx;
    if (angle < 0) {
        if (GP4_GLOBAL(int32_t, 0x00638d30) >= 0x3000) {
            selector = 0xffffffffu;
            ebp = GP4_GLOBAL(uint32_t, 0x00623bd8);
            ecx = GP4_GLOBAL(uint32_t, 0x00623bdc);
        } else {
            selector = 0u;
            ebp = GP4_GLOBAL(uint32_t, 0x00623be4);
            ecx = GP4_GLOBAL(uint32_t, 0x00623bec);
        }
    } else {
        selector = 1u;
        ebp = GP4_GLOBAL(uint32_t, 0x00623be0);
        ecx = GP4_GLOBAL(uint32_t, 0x00623be8);
    }
    GP4_GLOBAL(uint32_t, 0x00623bf0) = selector;

    const uint32_t first = fixed14(magnitude(frontTerm), static_cast<int32_t>(ebp));
    const uint32_t second = fixed14(magnitude(sideTerm), static_cast<int32_t>(ecx));
    uint32_t total = second + first;

    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x006013bc);
    const uint32_t flags = subtractionFlags32(total, limit);
    if (static_cast<int32_t>(total) > static_cast<int32_t>(limit)) {
        total = limit;
    }
    GP4_GLOBAL(uint32_t, 0x00623b8c) = total;

    r->eax = total;
    r->edx = first;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041c027, FUN_0041c027, "regs() -> (eax,edx,ebp,cf,pf,af,zf,sf,of); globals=fuzz:i32+layout=dw_0041c027")
