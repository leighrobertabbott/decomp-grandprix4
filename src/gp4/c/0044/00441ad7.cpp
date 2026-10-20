#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic result (or / and / test): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return byteParity(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Clears the transient state of the ring record at the lap start [0x00891504]: word +0x1e is zeroed and bit 7 of
// bytes +0x48, +0x34 and +0x38 and bit 6 of +0x48 are cleared. edi is preserved; the flags are those of the final
// `and byte [edi + 0x48], 0xbf`.
// FUNCTION: GP4 0x00441ad7
void __cdecl FUN_00441ad7(gp4::Regs* r) {
    uint8_t* const record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00891504)));
    GP4_FIELD(uint16_t, record, 0x1e) = 0u;
    GP4_FIELD(uint8_t, record, 0x48) &= 0x7fu;
    GP4_FIELD(uint8_t, record, 0x34) &= 0x7fu;
    GP4_FIELD(uint8_t, record, 0x38) &= 0x7fu;
    GP4_FIELD(uint8_t, record, 0x48) &= 0xbfu;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(GP4_FIELD(uint8_t, record, 0x48));
}
GP4_IMPL(0x00441ad7, FUN_00441ad7, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+lapflags_008943a0")
