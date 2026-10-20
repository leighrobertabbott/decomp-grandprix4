#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `cmp left, right` (CF PF AF ZF SF OF).
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_004087aa_Record {
    uint8_t Unknown00[0x8d];
    uint8_t Flags8D;
    uint8_t Unknown8E[0xb2 - 0x8e];
    int16_t UnknownB2;
    uint8_t UnknownB4[0xd6 - 0xb4];
    int16_t UnknownD6;
    uint8_t UnknownD8[0x168 - 0xd8];
    uint16_t Unknown168;
    uint8_t Unknown16A[0x19c - 0x16a];
    int32_t Unknown19C;
};

// Derives two words of the esi record: +0xb2 = max(([0x00602b4c] * +0x19c) >> 28 + (+0x168 or
// [0x00601388]) [+ [0x0062eca8] when bit 0 of +0x8d], [0x0062a0a0]) and +0xd6 = clamp(
// ([0x0060138c] << 14) / +0xb2, 0x2000, 0x7000). eax = the clamped quotient, edx = the remainder,
// ebp = the +0xb2 value; the flags are those of the final `cmp eax, 0x2000`.
// FUNCTION: GP4 0x004087aa
void __cdecl FUN_004087aa(gp4::Regs* r) {
    Unknown_004087aa_Record* record = reinterpret_cast<Unknown_004087aa_Record*>(static_cast<uintptr_t>(r->esi));

    // imul dword [0x602b4c] ; shrd eax, edx, 0x1c
    const int64_t product = static_cast<int64_t>(record->Unknown19C) * GP4_GLOBAL(int32_t, 0x00602b4c);
    uint32_t eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 28);

    if (record->Unknown168 != 0u) {
        eax += static_cast<uint32_t>(record->Unknown168);
    } else {
        eax += GP4_GLOBAL(uint32_t, 0x00601388);
    }
    if ((record->Flags8D & 1u) != 0u) {
        eax += GP4_GLOBAL(uint32_t, 0x0062eca8);
    }
    const int32_t floorValue = GP4_GLOBAL(int32_t, 0x0062a0a0);
    if (static_cast<int32_t>(eax) < floorValue) {
        eax = static_cast<uint32_t>(floorValue);
    }
    record->UnknownB2 = static_cast<int16_t>(eax);
    const uint32_t ebp = eax;

    // cdq ; shld edx, eax, 14 ; shl eax, 14 ; idiv ebp
    const int64_t dividend = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0060138c)) * 16384;
    int32_t remainder;
    int32_t quotient = gp4::idiv64(dividend, static_cast<int32_t>(ebp), &remainder);

    if (quotient > 0x7000) {
        quotient = 0x7000;
    }
    const uint32_t flags = cmpFlags32(static_cast<uint32_t>(quotient), 0x2000u);
    if (quotient < 0x2000) {
        quotient = 0x2000;
    }
    record->UnknownD6 = static_cast<int16_t>(quotient);

    r->eax = static_cast<uint32_t>(quotient);
    r->edx = static_cast<uint32_t>(remainder);
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004087aa, FUN_004087aa, "regs(esi:ptr[0x1a4]:layout=rec_004087aa) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_004087aa")
