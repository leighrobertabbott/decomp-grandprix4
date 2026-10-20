#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags left by a byte-wide OR: CF = OF = AF = 0, PF/ZF/SF from the result.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

struct Unknown_0042f9f8_Record {
    uint8_t Unknown00[0xfe];
    uint8_t FlagsFE;
    uint8_t UnknownFF[0x114 - 0xff];
    uint32_t Unknown114;
    uint8_t Unknown118[0x122 - 0x118];
    uint8_t Flags122;
    uint8_t Unknown123[0x135 - 0x123];
    uint8_t Flags135;
};

// FUNCTION: GP4 0x0042f9f8
// Resets a record (esi): clears bit 6 of +0xfe, runs 0x0042784b (sets bit 3 of +0xfe), sets bit 2
// of +0x135, clears bits 3 and 4 of +0x122, runs 0x0042e314, zeroes the dword at +0x114 and sets
// bit 6 of +0x135. The registers are those left by the callees; the flags are those of the last OR.
void __cdecl FUN_0042f9f8(gp4::Regs* r) {
    Unknown_0042f9f8_Record* record =
        reinterpret_cast<Unknown_0042f9f8_Record*>(static_cast<uintptr_t>(r->esi));

    record->FlagsFE = static_cast<uint8_t>(record->FlagsFE & 0xbfu);
    gp4::call_regs(0x0042784b, *r);
    record->Flags135 = static_cast<uint8_t>(record->Flags135 | 0x04u);
    record->Flags122 = static_cast<uint8_t>(record->Flags122 & 0xf7u);
    record->Flags122 = static_cast<uint8_t>(record->Flags122 & 0xefu);
    gp4::call_regs(0x0042e314, *r);
    record->Unknown114 = 0u;
    record->Flags135 = static_cast<uint8_t>(record->Flags135 | 0x40u);
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(record->Flags135);
}
GP4_IMPL(0x0042f9f8, FUN_0042f9f8, "regs(esi:ptr[0x420]:layout=rec_00409416) -> (cf, pf, af, zf, sf, of); globals=layout=state_00403800")
