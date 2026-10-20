#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a byte-wide TEST (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint32_t result) {
    return parityFlag(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of a dword-wide TEST (CF = OF = AF = 0).
static inline uint32_t testFlags32(uint32_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
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

struct Unknown_0042ba87_Record {
    uint8_t Unknown00[0x8d];
    uint8_t Flags8D;
    uint8_t Unknown8E[0xea - 0x8e];
    uint16_t UnknownEA;
    uint8_t UnknownEC[0xfe - 0xec];
    uint8_t FlagsFE;
    uint8_t UnknownFF[0x114 - 0xff];
    uint32_t Unknown114;
    uint8_t Unknown118[0x135 - 0x118];
    uint8_t Flags135;
};

// Retires the record in esi: unless bit 1 of +0x8d is set, or bit 6 of +0xfe is set with a nonzero
// dword at +0x114 (CF = 1 on those exits), it zeroes the word at +0xea, sets bits 6 and 4 of +0xfe,
// runs 0x0042784b, sets bit 2 of +0x135, clears [0x006043dc] when it equals esi, stores ebx into
// +0x114 and returns CF = 0. The flags are those of the last test / cmp (CF forced).
// FUNCTION: GP4 0x0042ba87
void __cdecl FUN_0042ba87(gp4::Regs* r) {
    Unknown_0042ba87_Record* record = reinterpret_cast<Unknown_0042ba87_Record*>(static_cast<uintptr_t>(r->esi));
    const uint32_t esi = r->esi;
    const uint32_t ebx = r->ebx;

    if ((record->Flags8D & 2u) != 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | testFlags8(record->Flags8D & 2u) | 1u;
        return;
    }
    if ((record->FlagsFE & 0x40u) != 0u) {
        const uint32_t pending = record->Unknown114;
        if (pending != 0u) {
            r->eflags = (r->eflags & ~0x8d5u) | testFlags32(pending) | 1u;
            return;
        }
    }

    record->UnknownEA = 0;
    record->FlagsFE = static_cast<uint8_t>(record->FlagsFE | 0x40u);
    record->FlagsFE = static_cast<uint8_t>(record->FlagsFE | 0x10u);
    gp4::Regs call = *r;
    gp4::call_regs(0x0042784b, call);
    record->Flags135 = static_cast<uint8_t>(record->Flags135 | 4u);

    const uint32_t current = GP4_GLOBAL(uint32_t, 0x006043dc);
    const uint32_t flags = cmpFlags32(esi, current);
    if (esi == current) {
        GP4_GLOBAL(uint32_t, 0x006043dc) = 0;
    }
    record->Unknown114 = ebx;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u);
}
GP4_IMPL(0x0042ba87, FUN_0042ba87, "regs(ebx:u32, esi:ptr[0x420]:layout=car_0042ba87) -> (cf, pf, af, zf, sf, of); globals=layout=state_00403800+state_004059ef+eq_0042ba87")
