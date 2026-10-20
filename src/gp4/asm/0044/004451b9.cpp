#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit AND result (CF = OF = AF = 0).
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Unpacks the control byte in al into five flag bits of the record in esi (the inverse of 0x00445185):
// bit 3 -> +0x231 bit 5, bit 4 -> +0x71 bit 7, bit 5 -> +0xfe bit 2, bit 6 -> +0x230 bit 7, bit 7 -> +0x7b
// bit 1 (each cleared first). al ends as the low nibble; the flags are those of `and al, 0xf`.
// FUNCTION: GP4 0x004451b9
void __cdecl FUN_004451b9(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t bits = r->eax & 0xffu;

    GP4_FIELD(uint8_t, record, 0x71) &= 0x7f;
    GP4_FIELD(uint8_t, record, 0xfe) &= 0xfb;
    GP4_FIELD(uint8_t, record, 0x230) &= 0x7f;
    GP4_FIELD(uint8_t, record, 0x7b) &= 0xfd;
    GP4_FIELD(uint8_t, record, 0x231) &= 0xdf;

    if ((bits & 0x08u) != 0u) {
        GP4_FIELD(uint8_t, record, 0x231) |= 0x20;
    }
    if ((bits & 0x10u) != 0u) {
        GP4_FIELD(uint8_t, record, 0x71) |= 0x80;
    }
    if ((bits & 0x20u) != 0u) {
        GP4_FIELD(uint8_t, record, 0xfe) |= 0x04;
    }
    if ((bits & 0x40u) != 0u) {
        GP4_FIELD(uint8_t, record, 0x230) |= 0x80;
    }
    if ((bits & 0x80u) != 0u) {
        GP4_FIELD(uint8_t, record, 0x7b) |= 0x02;
    }

    const uint32_t nibble = bits & 0x0fu;
    r->eax = (r->eax & 0xffffff00u) | nibble;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(nibble);
}
GP4_IMPL(0x004451b9, FUN_004451b9, "regs(eax:u32, esi:ptr[0x239]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
