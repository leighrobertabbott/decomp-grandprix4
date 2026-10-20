#include <gp4/gp4.h>

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Sets the byte DAT_00601ba4 to 0xff when the address in edi lies below the end of the
// (possibly wrapped by DAT_00866ec4) window, else to 0. The flags left behind are those of the
// final unsigned "cmp edi, limit". All registers are preserved.
// FUNCTION: GP4 0x00403850
void __cdecl FUN_00403850(gp4::Regs* r) {
    uint32_t position = r->edi;
    uint32_t limit = GP4_GLOBAL(uint32_t, 0x00866ed4) - 0x2940u;

    if (position < GP4_GLOBAL(uint32_t, 0x00891500)) {
        if (position < GP4_GLOBAL(uint32_t, 0x00866f10)) {
            position += GP4_GLOBAL(uint32_t, 0x00866ec4);
        }
        limit = GP4_FIELD(uint32_t, limit, 0x44);
        if (limit < GP4_GLOBAL(uint32_t, 0x00866f10)) {
            limit += GP4_GLOBAL(uint32_t, 0x00866ec4);
        }
    }

    GP4_GLOBAL(uint8_t, 0x00601ba4) = (position < limit) ? 0xff : 0;
    r->eflags = (r->eflags & ~0x8d5u) | subtractionFlags32(position, limit);
}
GP4_IMPL(0x00403850, FUN_00403850, "regs(edi:u32) -> (cf, pf, af, zf, sf, of); globals=layout=state_00403850")
