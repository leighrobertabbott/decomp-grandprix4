#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of "test byte, 0xff" / "test byte, 2": CF = OF = AF = 0
static inline uint32_t testFlags(uint32_t masked) {
    return parityFlag(masked) | (masked == 0u ? 0x40u : 0u) | ((masked & 0x80u) != 0u ? 0x80u : 0u);
}

// flags of "cmp left, right"
static inline uint32_t cmpFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x00434602
// When DAT_00852d98 and DAT_00852d8c are both zero, esi is the record DAT_0066d820 and bit 1 of
// byte [esi+0x71] is set, DAT_006f2f65 = 0xff. The flags are those of the last test/cmp executed.
void __cdecl FUN_00434602(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags;

    const uint32_t first = GP4_GLOBAL(uint8_t, 0x00852d98);
    if (first != 0u) {
        flags = testFlags(first);
    } else {
        const uint32_t second = GP4_GLOBAL(uint8_t, 0x00852d8c);
        if (second != 0u) {
            flags = testFlags(second);
        } else {
            const uint32_t current = GP4_GLOBAL(uint32_t, 0x0066d820);
            if (r->esi != current) {
                flags = cmpFlags(r->esi, current);
            } else {
                const uint32_t masked = GP4_FIELD(uint8_t, record, 0x71) & 2u;
                if (masked != 0u) {
                    GP4_GLOBAL(uint8_t, 0x006f2f65) = 0xff;
                }
                flags = testFlags(masked);
            }
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00434602, FUN_00434602, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (cf, pf, af, zf, sf, of); globals=layout=state_00434602")
