#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags left by a byte-wide AND/TEST/OR: CF = OF = AF = 0, ZF/SF/PF from the result.
static inline uint32_t logicFlags8(uint32_t result) {
    uint32_t flags = parityFlag8(result);
    if ((result & 0xffu) == 0u) flags |= 0x40u;
    if ((result & 0x80u) != 0u) flags |= 0x80u;
    return flags;
}

// Arithmetic flags of the 16-bit `left - right` (cmp).
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u) | parityFlag8(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 8) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) >> 4);
}

// Returns CF = 1 when it newly sets bit 3 of the status byte at +0x163. Needs the flag at +0x161&2;
// leaves ax = word[+0x9a] when the status bit was clear and compares it with the word at 0x603bc0.
// FUNCTION: GP4 0x00434d6b
void __cdecl FUN_00434d6b(gp4::Regs* r) {
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));

    uint32_t flags;
    uint32_t carry = 0u;

    const uint8_t gate = GP4_FIELD(uint8_t, record, 0x161);
    flags = logicFlags8(gate & 2u);                                      // test byte [esi+0x161], 2
    if ((gate & 2u) != 0u) {
        uint8_t status = GP4_FIELD(uint8_t, record, 0x163);
        flags = logicFlags8(status & 8u);                                // test byte [esi+0x163], 8
        carry = 1u;                                                      // stc
        if ((status & 8u) == 0u) {
            const uint32_t ax = GP4_FIELD(uint16_t, record, 0x9a);
            const uint32_t limit = GP4_GLOBAL(uint16_t, 0x00603bc0);
            r->eax = (r->eax & 0xffff0000u) | ax;
            flags = subFlags16(ax, limit);                               // cmp ax, [0x603bc0]
            if (((flags >> 7) & 1u) == ((flags >> 11) & 1u)) {           // jge
                carry = 0u;                                              // clc
            } else {
                status = static_cast<uint8_t>(status | 8u);
                GP4_FIELD(uint8_t, record, 0x163) = status;              // or byte [esi+0x163], 8
                flags = logicFlags8(status);
            }
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | carry;
}
GP4_IMPL(0x00434d6b, FUN_00434d6b, "regs(esi:ptr[0x16b]:bytes, eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
