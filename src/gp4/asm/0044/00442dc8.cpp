#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags left by OR / TEST: CF = OF = AF = 0, ZF/SF/PF from the result.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag(result) | ((result & 0xffu) == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Flags of the 32-bit "cmp a, b" (a - b).
static inline uint32_t cmpFlags32(uint32_t a, uint32_t b) {
    const uint32_t result = a - b;
    return (a < b ? 0x01u : 0u)
         | parityFlag(result)
         | (((a ^ b ^ result) & 0x10u) != 0u ? 0x10u : 0u)
         | (result == 0u ? 0x40u : 0u)
         | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
         | ((((a ^ b) & (a ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Releases the car record in esi: clears its pending bytes (0x0040f9e6) and forgets it when it is the
// car pointer 0x0066d820, and also 0x0066d828 when byte 0x007aafd4 is nonzero (after appending the
// opcode 0x0b to the stream with 0x00444f1e). A null esi does nothing. Registers are untouched; the
// flags are those of the last flag writer on the path taken.
// FUNCTION: GP4 0x00442dc8
void __cdecl FUN_00442dc8(gp4::Regs* r) {
    const uint32_t record = r->esi;
    uint32_t flags = logicFlags32(record);              // or esi, esi
    if (record != 0u) {
        gp4::Regs clear = *r;
        clear.st_in = 0;
        clear.st_out = 0;
        gp4::call_regs(0x0040f9e6, clear);

        const uint32_t first = GP4_GLOBAL(uint32_t, 0x0066d820);
        flags = cmpFlags32(first, record);               // cmp [0x66d820], esi
        if (first == record) {
            GP4_GLOBAL(uint32_t, 0x0066d820) = 0u;
        }

        const uint8_t second = GP4_GLOBAL(uint8_t, 0x007aafd4);
        flags = logicFlags8(second);                     // test byte [0x7aafd4], 0xff
        if (second != 0u) {
            gp4::Regs append = *r;
            append.st_in = 0;
            append.st_out = 0;
            gp4::call_regs(0x00444f1e, append);

            const uint32_t other = GP4_GLOBAL(uint32_t, 0x0066d828);
            flags = cmpFlags32(other, record);           // cmp [0x66d828], esi
            if (other == record) {
                GP4_GLOBAL(uint32_t, 0x0066d828) = 0u;
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00442dc8, FUN_00442dc8, "regs(esi:nptr[0x77]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=state_0043260e+stream_0062d1a0")
