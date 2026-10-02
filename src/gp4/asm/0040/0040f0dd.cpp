#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040f0dd
void __cdecl FUN_0040f0dd(gp4::Regs* r) {
    const uint8_t* body = reinterpret_cast<const uint8_t*>(r->esi);
    const uint32_t rawRow = (static_cast<uint32_t>(body[0x7c]) - 1u) & 0x3fu;
    const uint32_t row = rawRow > 21u ? 21u : rawRow;
    const uint32_t rawColumn = GP4_ARRAY(uint8_t, 0x0066d936)[row];
    const uint32_t column = rawColumn > 43u ? 43u : rawColumn;
    uint8_t* counters = GP4_ARRAY(uint8_t, 0x0066d962);
    const uint8_t previous = counters[row * 44u + column];
    const uint8_t result = static_cast<uint8_t>(previous + 1u);
    counters[row * 44u + column] = result;
    uint32_t parity = result;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    const uint32_t flags = ((parity & 1u) == 0u ? 4u : 0u)
        | ((previous & 0x0fu) == 0x0fu ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (previous == 0x7fu ? 0x800u : 0u);
    // IMUL of the bounded row by 44 cleared CF before the byte INC.
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040f0dd, FUN_0040f0dd, "regs(esi:ptr[0x84]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=counter_0040f0dd")
