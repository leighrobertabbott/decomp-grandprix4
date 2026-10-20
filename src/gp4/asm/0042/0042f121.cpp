#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of a logic operation (test/and/or) with an 8-bit result: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

static inline uint32_t additionFlags32(uint32_t left, uint32_t right, uint32_t result) {
    return (result < left ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0042f121
// Record flag byte +0x231: when bit 0x40 is set, the record's word +0x9a decides: >= 0x3ab clears
// bits 0x40 and 0x80; otherwise bit 0x80 is set if the dword +0x250 is nonzero and the global
// 0x66d814 minus it is not negative. Only ax is clobbered, all other registers are untouched.
void __cdecl FUN_0042f121(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t state = GP4_FIELD(uint8_t, record, 0x231);
    uint32_t flags;

    if ((state & 0x40u) == 0u) {
        // test byte [esi+0x231], 0x40 -> zero
        flags = logicFlags8(0u);
    } else {
        const uint16_t word = GP4_FIELD(uint16_t, record, 0x9a);
        r->eax = (r->eax & 0xffff0000u) | word;
        if (static_cast<int16_t>(word) >= 0x3ab) {
            uint8_t value = static_cast<uint8_t>(state & 0xbfu);
            GP4_FIELD(uint8_t, record, 0x231) = value;
            value = static_cast<uint8_t>(value & 0x7fu);
            GP4_FIELD(uint8_t, record, 0x231) = value;
            flags = logicFlags8(value);
        } else {
            const uint32_t deadline = GP4_FIELD(uint32_t, record, 0x250);
            if (deadline == 0u) {
                // test dword [esi+0x250], -1 -> zero
                flags = logicFlags8(0u);
            } else {
                // push eax; mov eax,[esi+0x250]; neg eax; add eax,[0x66d814]; pop eax
                const uint32_t negated = 0u - deadline;
                const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
                const uint32_t remaining = negated + clock;
                flags = additionFlags32(negated, clock, remaining);
                const bool negative = (remaining & 0x80000000u) != 0u;
                if (!negative) {
                    const uint8_t value = static_cast<uint8_t>(state | 0x80u);
                    GP4_FIELD(uint8_t, record, 0x231) = value;
                    flags = logicFlags8(value);
                }
            }
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042f121, FUN_0042f121, "regs(eax:u32, esi:ptr[0x258]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
