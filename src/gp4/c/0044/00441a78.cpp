#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit logic result (or / and / test): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Sets up the control block [0x0086850c] + 0x100 and the segment record [0x00891504]: the sign of the
// record's word +0x16 selects the pair {0x00c0, 0x0000} (negative) or {0xff40, 0x8000} (otherwise) for the
// block's words +4 / +6; block word +0 gets bit 10, word +2 is cleared; record word +0x1e becomes 0x10 and
// its bytes +0x48 (0xc0), +0x34 and +0x38 (0x80) get flag bits. eax / ebx / edi are saved and restored; the
// flags are those of the last `or byte [edi + 0x48], 0x40`.
// FUNCTION: GP4 0x00441a78
void __cdecl FUN_00441a78(gp4::Regs* r) {
    uint8_t* block = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0086850c) + 0x100u));
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00891504)));

    if ((*reinterpret_cast<int16_t*>(record + 0x16)) < 0) {
        *reinterpret_cast<uint16_t*>(block + 4) = 0x00c0u;
        *reinterpret_cast<uint16_t*>(block + 6) = 0u;
    } else {
        *reinterpret_cast<uint16_t*>(block + 4) = 0xff40u;
        *reinterpret_cast<uint16_t*>(block + 6) = 0x8000u;
    }
    *reinterpret_cast<uint16_t*>(block) = static_cast<uint16_t>(*reinterpret_cast<uint16_t*>(block) | 0x400u);
    *reinterpret_cast<uint16_t*>(record + 0x1e) = 0x10u;
    record[0x48] = static_cast<uint8_t>(record[0x48] | 0x80u);
    record[0x34] = static_cast<uint8_t>(record[0x34] | 0x80u);
    record[0x38] = static_cast<uint8_t>(record[0x38] | 0x80u);
    record[0x48] = static_cast<uint8_t>(record[0x48] | 0x40u);
    *reinterpret_cast<uint16_t*>(block + 2) = 0u;

    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(record[0x48]);
}
GP4_IMPL(0x00441a78, FUN_00441a78, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+words_008943a0+ptr_0086850c")
