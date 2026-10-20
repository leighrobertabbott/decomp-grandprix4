#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `add left, right` (CF PF AF ZF SF OF).
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_0041e417_Record {
    uint8_t Unknown00[0x38c];
    int32_t Unknown38C;
    int32_t Unknown390;
    int32_t Unknown394;
    int32_t Unknown398;
};

// Measures the imbalance (+0x394 - +0x398) - (+0x38c - +0x390) of four coordinates in the esi record,
// stores its magnitude clamped to [0x0062781c] in [0x00627814], then shifts the pair (+0x390, +0x394)
// by half of it (and sets [0x00627818] = 1) when +0x394 - +0x38c < +0x398 - +0x390, otherwise shifts
// the pair (+0x38c, +0x398) (and sets [0x00627818] = -1). ebp = the half step, ecx / edx = the two
// differences; the flags are those of the last add into the record.
// FUNCTION: GP4 0x0041e417
void __cdecl FUN_0041e417(gp4::Regs* r) {
    Unknown_0041e417_Record* record = reinterpret_cast<Unknown_0041e417_Record*>(static_cast<uintptr_t>(r->esi));

    uint32_t ebp = static_cast<uint32_t>(record->Unknown394) - static_cast<uint32_t>(record->Unknown398);
    ebp -= static_cast<uint32_t>(record->Unknown38C);
    ebp += static_cast<uint32_t>(record->Unknown390);
    if (static_cast<int32_t>(ebp) < 0) {
        ebp = 0u - ebp;
    }
    if (static_cast<int32_t>(ebp) > GP4_GLOBAL(int32_t, 0x0062781c)) {
        ebp = GP4_GLOBAL(uint32_t, 0x0062781c);
    }
    GP4_GLOBAL(uint32_t, 0x00627814) = ebp;
    GP4_GLOBAL(uint32_t, 0x00627818) = 0u;
    ebp = static_cast<uint32_t>(static_cast<int32_t>(ebp) >> 1);

    const uint32_t ecx = static_cast<uint32_t>(record->Unknown394) - static_cast<uint32_t>(record->Unknown38C);
    const uint32_t edx = static_cast<uint32_t>(record->Unknown398) - static_cast<uint32_t>(record->Unknown390);

    uint32_t flags;
    if (static_cast<int32_t>(ecx) < static_cast<int32_t>(edx)) {
        record->Unknown390 = static_cast<int32_t>(static_cast<uint32_t>(record->Unknown390) + ebp);
        const uint32_t before = static_cast<uint32_t>(record->Unknown394);
        record->Unknown394 = static_cast<int32_t>(before + ebp);
        flags = addFlags32(before, ebp);
        GP4_GLOBAL(uint32_t, 0x00627818) = 1u;
    } else {
        record->Unknown38C = static_cast<int32_t>(static_cast<uint32_t>(record->Unknown38C) + ebp);
        const uint32_t before = static_cast<uint32_t>(record->Unknown398);
        record->Unknown398 = static_cast<int32_t>(before + ebp);
        flags = addFlags32(before, ebp);
        GP4_GLOBAL(uint32_t, 0x00627818) = 0xffffffffu;
    }

    r->ebp = ebp;
    r->ecx = ecx;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041e417, FUN_0041e417, "regs(esi:ptr[0x3a0]:bytes) -> (ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
