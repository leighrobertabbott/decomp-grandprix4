#include <gp4/gp4.h>

static inline uint32_t parityEvenFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// SF/ZF/PF of a result (the rest of the shift flags are added by the caller)
static inline uint32_t szpFlags(uint32_t result, uint32_t signBit) {
    return parityEvenFlag(result)
        | (result == 0u ? 0x040u : 0u)
        | ((result & signBit) != 0u ? 0x080u : 0u);
}

// Stores a 7-bit value (index < 6) or the bits of a 6-bit code (index >= 6) into the
// low 7 bits / bit 7 of the bytes at edx, keeping the other bit of each byte:
//   index < 6:  bytes[index] = (bytes[index] & 0x80) | clamp(value, 1, 0x50)
//   index >= 6: bit 7 of bytes[k] = bit k of code, k = 0..5, where code = 0 for value 0
//               else clamp(value, 0x12, 0x50) - 0x11 (the low 7 bits are kept).
// eax, ecx and edx are preserved; the flags are those of the last shift/rotate.
// FUNCTION: GP4 0x00410b8c
void __cdecl FUN_00410b8c(gp4::Regs* r) {
    const int32_t value = static_cast<int32_t>(r->eax);
    const int32_t index = static_cast<int32_t>(r->ecx);
    uint8_t* const bytes = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edx));

    uint32_t flags;
    if (index < 6) {
        int32_t clamped = value;
        if (clamped < 1) clamped = 1;
        if (clamped > 0x50) clamped = 0x50;
        const uint32_t old = bytes[index];
        bytes[index] = static_cast<uint8_t>((old & 0x80u) | (static_cast<uint32_t>(clamped) & 0x7fu));
        // shl ch,1 gives SF/ZF/PF; rcr cl,1 then leaves CF = 0 and OF = bit6(cl) ^ old bit 7
        const uint32_t shifted = (old << 1) & 0xffu;
        const uint32_t overflow = ((static_cast<uint32_t>(clamped) >> 6) ^ (old >> 7)) & 1u;
        flags = szpFlags(shifted, 0x80u) | (overflow != 0u ? 0x800u : 0u);
    } else {
        uint32_t code = 0;
        if (value != 0) {
            int32_t clamped = value;
            if (clamped > 0x50) clamped = 0x50;
            if (clamped < 0x12) clamped = 0x12;
            code = static_cast<uint32_t>(clamped - 0x11);
        }
        uint32_t remaining = code;
        uint32_t overflow = 0;
        for (int k = 0; k < 6; ++k) {
            const uint32_t old = bytes[k];
            const uint32_t bit = remaining & 1u;
            remaining >>= 1;
            bytes[k] = static_cast<uint8_t>((old & 0x7fu) | (bit << 7));
            overflow = ((old >> 6) ^ bit) & 1u;   // rcr al,1: OF of the last byte survives
        }
        // shr ecx,1 of the last byte gives SF/ZF/PF; rcr leaves CF = 0
        flags = szpFlags(remaining, 0x80000000u) | (overflow != 0u ? 0x800u : 0u);
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00410b8c, FUN_00410b8c, "regs(eax:i32[-4..100], ecx:i32[0..12], edx:ptr[0x40]:bytes) -> (cf, pf, af, zf, sf, of)")
