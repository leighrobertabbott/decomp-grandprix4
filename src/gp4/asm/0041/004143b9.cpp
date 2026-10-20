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

struct Unknown_004143b9_Record {
    uint8_t Unknown00[0x6f];
    uint8_t Flags6F;
    uint8_t Unknown70[0x90 - 0x70];
    uint16_t Unknown90;
    uint8_t Unknown92[0x232 - 0x92];
    uint8_t Flags232;
};

// FUNCTION: GP4 0x004143b9
// Decides which of bits 2 and 3 to set in the byte at esi+0x6f (and mirrors that in bit 2 of
// esi+0x232) from the sign and size of the word at 0x0063803e and the sign of
// word[esi+0x90] ^ word[0x00934df8], then sets bit 0 or bit 1 of esi+0x6f depending on the signed
// compare of DAT_00622508 with DAT_006224f0. Leaves dx = the last word handled, eax = DAT_00622508
// and the flags of the final OR.
void __cdecl FUN_004143b9(gp4::Regs* r) {
    Unknown_004143b9_Record* record =
        reinterpret_cast<Unknown_004143b9_Record*>(static_cast<uintptr_t>(r->esi));
    bool takeAltTarget;  // 0x00414416 (true) or 0x00414409 (false)

    if ((record->Flags232 & 8u) == 0u) {
        takeAltTarget = (record->Flags232 & 4u) != 0u;
    } else {
        record->Flags232 = static_cast<uint8_t>(record->Flags232 & 0xf7u);
        const uint16_t raw = GP4_GLOBAL(uint16_t, 0x0063803e);
        uint16_t dx = raw;
        if (static_cast<int16_t>(dx) < 0) {
            dx = static_cast<uint16_t>(0u - dx);
        }
        if (static_cast<int16_t>(dx) < 0x10) {
            dx = static_cast<uint16_t>(record->Unknown90 ^ GP4_GLOBAL(uint16_t, 0x00934df8));
            takeAltTarget = static_cast<int16_t>(dx) < 0;
        } else {
            takeAltTarget = static_cast<int16_t>(raw) >= 0;
        }
        r->edx = (r->edx & 0xffff0000u) | dx;
    }

    if (takeAltTarget) {
        record->Flags6F = static_cast<uint8_t>(record->Flags6F | 8u);
        record->Flags232 = static_cast<uint8_t>(record->Flags232 | 4u);
    } else {
        record->Flags6F = static_cast<uint8_t>(record->Flags6F | 4u);
        record->Flags232 = static_cast<uint8_t>(record->Flags232 & 0xfbu);
    }

    const int32_t first = GP4_GLOBAL(int32_t, 0x00622508);
    const int32_t second = GP4_GLOBAL(int32_t, 0x006224f0);
    r->eax = static_cast<uint32_t>(first);
    record->Flags6F = static_cast<uint8_t>(record->Flags6F | (first < second ? 1u : 2u));
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(record->Flags6F);
}
GP4_IMPL(0x004143b9, FUN_004143b9, "regs(esi:ptr[0x23a]:bytes, edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
