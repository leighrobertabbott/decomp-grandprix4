#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `cmp left, right`.
static inline uint32_t cmpFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// Flags of a 32-bit TEST/AND/OR result: CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u);
}

struct Unknown_0042bae4_Record {
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

// Arms the esi record unless bit 1 of +0x8d is set, or bit 6 of +0xfe is already set with a
// non-zero dword at +0x114 (stc, flags of the failing TEST). Otherwise: word +0xea = 0, +0xfe
// bit 6 set / bit 4 cleared, 0x0042784b run on the record, +0x135 bit 2 set, the global pointer
// 0x6043d8 cleared when it equals esi, and the dword at +0x114 = ebx (clc, flags of the cmp).
// FUNCTION: GP4 0x0042bae4
void __cdecl FUN_0042bae4(gp4::Regs* r) {
    Unknown_0042bae4_Record* record =
        reinterpret_cast<Unknown_0042bae4_Record*>(static_cast<uintptr_t>(r->esi));
    const uint32_t value = r->ebx;

    uint32_t flags;
    if ((record->Flags8D & 2u) != 0u) {
        flags = logicFlags32(record->Flags8D & 2u) | 1u;
    } else if ((record->FlagsFE & 0x40u) != 0u && record->Unknown114 != 0u) {
        flags = logicFlags32(record->Unknown114) | 1u;
    } else {
        record->UnknownEA = 0;
        record->FlagsFE = static_cast<uint8_t>(record->FlagsFE | 0x40u);
        record->FlagsFE = static_cast<uint8_t>(record->FlagsFE & 0xefu);
        gp4::Regs callee = *r;
        gp4::call_regs(0x0042784b, callee);
        record->Flags135 = static_cast<uint8_t>(record->Flags135 | 4u);
        const uint32_t current = GP4_GLOBAL(uint32_t, 0x006043d8);
        flags = cmpFlags(r->esi, current) & ~1u;
        if (r->esi == current) {
            GP4_GLOBAL(uint32_t, 0x006043d8) = 0u;
        }
        record->Unknown114 = value;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042bae4, FUN_0042bae4, "regs(esi:ptr[0x420]:layout=car_0042ba87, ebx:u32) -> (cf, pf, af, zf, sf, of); globals=layout=state_00403800+state_004059ef+eq_0042ba87")
