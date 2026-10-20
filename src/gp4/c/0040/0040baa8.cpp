#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040baa8
// Saves and restores eax/ebp/edx; the flags that escape are those of the
// "cmp eax, 0x12c" after the upper clamp.
void __cdecl FUN_0040baa8(gp4::Regs* r) {
    const int32_t factor = GP4_GLOBAL(int32_t, 0x0062d238);
    const uint32_t index = GP4_GLOBAL(uint16_t, 0x007ad89e);
    const int32_t firstScale = static_cast<int32_t>((4u - index) * 0x140u + 0x4200u);
    const int64_t firstProduct = static_cast<int64_t>(factor) * firstScale;
    const uint32_t first = static_cast<uint32_t>(static_cast<uint64_t>(firstProduct) >> 14);

    const int64_t secondProduct = static_cast<int64_t>(static_cast<int32_t>(first)) * GP4_GLOBAL(uint16_t, 0x007c2e96);
    const uint32_t second = static_cast<uint32_t>(secondProduct);

    int32_t difference = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00622780) - GP4_GLOBAL(uint32_t, 0x00622790));
    if (difference < 0) {
        difference = 0;
    }
    const int32_t thirdScale = static_cast<int32_t>((static_cast<uint32_t>(difference) << 4) + 0x4000u);
    const int64_t thirdProduct = static_cast<int64_t>(static_cast<int32_t>(second)) * thirdScale;

    // xor edx, edx / idiv 1000: unsigned quotient of the 32-bit value
    uint32_t value = static_cast<uint32_t>(static_cast<uint64_t>(thirdProduct) >> 14) / 1000u;
    if (static_cast<int32_t>(value) > 0x708) {
        value = 0x708;
    }
    const uint32_t result = value - 0x12cu;
    uint32_t stored = value;
    if (static_cast<int32_t>(value) < 0x12c) {
        stored = 0x12c;
    }
    GP4_GLOBAL(uint32_t, 0x00622778) = stored;

    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    const uint32_t flags = (value < 0x12cu ? 1u : 0u)
        | ((~parity & 1u) << 2)
        | ((value ^ 0x12cu ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | ((((value ^ 0x12cu) & (value ^ result)) >> 31) << 11);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040baa8, FUN_0040baa8, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
