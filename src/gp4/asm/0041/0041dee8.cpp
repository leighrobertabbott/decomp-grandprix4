#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit "cmp left, right".
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Averages a column of signed bytes of the grid at 0x00628bc0 (ebx = x, edi = first row offset,
// stepping by [0x00627b98] and wrapping with [0x00627b9c], [0x00627ba0] taps), scales the sum by
// [0x00627ba8] (>> 14), divides by the tap count, clamps to a signed byte and stores it at
// [ebx + edi + 0x00627bbc]. eax = clamped value, ecx = tap count, edx = remainder, esi = final row
// offset; the flags are those of the last `cmp eax, -0x80`.
// FUNCTION: GP4 0x0041dee8
void __cdecl FUN_0041dee8(gp4::Regs* r) {
    const uint32_t x = r->ebx;
    const uint32_t y = r->edi;

    int32_t sum = 0;
    uint32_t pos = y;
    uint32_t taps = GP4_GLOBAL(uint32_t, 0x00627ba0);
    do {
        sum += static_cast<int32_t>(GP4_ARRAY(int8_t, 0x00628bc0)[pos + x]);
        pos = (pos + GP4_GLOBAL(uint32_t, 0x00627b98)) & GP4_GLOBAL(uint32_t, 0x00627b9c);
        taps = taps - 1u;
    } while (taps != 0u);

    const int64_t product = static_cast<int64_t>(sum) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00627ba8));
    const int32_t scaled = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
    const int32_t divisor = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00627ba0));
    int32_t remainder;
    int32_t value = gp4::idiv64(static_cast<int64_t>(scaled), divisor, &remainder);

    if (!(value <= 0x7f)) {
        value = 0x7f;
    }
    const uint32_t flags = subFlags(static_cast<uint32_t>(value), 0xffffff80u);
    if (!(value >= -0x80)) {
        value = -0x80;
    }
    GP4_ARRAY(int8_t, 0x00627bbc)[x + y] = static_cast<int8_t>(value);

    r->eax = static_cast<uint32_t>(value);
    r->ecx = static_cast<uint32_t>(divisor);
    r->edx = static_cast<uint32_t>(remainder);
    r->esi = pos;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041dee8, FUN_0041dee8, "regs(ebx:u32[0..63], edi:u32[0..0xfc0]) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=layout=grid_00628bc0")
