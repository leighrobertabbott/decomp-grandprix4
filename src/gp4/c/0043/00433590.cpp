#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of cmp left, right
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// flags of a 32-bit test / logic op: CF = OF = AF = 0
static inline uint32_t testFlags(uint32_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// flags of shl value, 16 (pinned oracle model: AF = 0, CF = last bit out, OF = bit31(prev ^ result))
static inline uint32_t shl16Flags(uint32_t value) {
    const uint32_t result = value << 16;
    const uint32_t previous = value << 15;
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (((value >> 16) & 1u) != 0u ? 1u : 0u);
}

// FUNCTION: GP4 0x00433590
void __cdecl FUN_00433590(gp4::Regs* r) {
    // push ebp / push edx are restored: only eax and the flags change
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x00601bc0);
    const int32_t signedValue = static_cast<int32_t>(value);
    uint32_t result = 0u;
    uint32_t flags;
    bool divide = false;
    uint32_t numerator = 0u;
    int32_t divisor = 0;

    if (GP4_GLOBAL(uint8_t, 0x00601bac) != 0u) {
        flags = subFlags(value, 0u);
        if (signedValue > 0) {
            const int32_t span = GP4_GLOBAL(int32_t, 0x00601bc8);
            flags = subFlags(static_cast<uint32_t>(span), 0u);
            if (span >= 0) {
                result = 0x100000u;
            } else {
                uint32_t magnitude = 0u - static_cast<uint32_t>(span);
                flags = subFlags(magnitude, 0x4000u);
                if (static_cast<int32_t>(magnitude) < 0x4000) {
                    magnitude = 0x4000u;
                }
                divisor = static_cast<int32_t>(magnitude);
                numerator = value;
                divide = true;
            }
        }
    } else {
        flags = subFlags(value, 0u);
        if (signedValue != 0) {
            flags = testFlags(value);
            if (signedValue < 0) {
                int32_t span = GP4_GLOBAL(int32_t, 0x00601bc8);
                flags = subFlags(static_cast<uint32_t>(span), 0u);
                if (span <= 0) {
                    result = 0x100000u;
                } else {
                    flags = subFlags(static_cast<uint32_t>(span), 0x4000u);
                    if (span < 0x4000) {
                        span = 0x4000;
                    }
                    divisor = span;
                    numerator = 0u - value;
                    divide = true;
                }
            }
        }
    }

    if (divide) {
        if (static_cast<int32_t>(numerator) > 0x40000) {
            numerator = 0x40000u;
        }
        // cdq / shld edx, eax, 16 / shl eax, 16 build edx:eax = numerator << 16; idiv keeps the shl flags
        flags = shl16Flags(numerator);
        const int64_t dividend = static_cast<int64_t>(static_cast<int32_t>(numerator)) * 65536;
        int32_t remainder;
        result = static_cast<uint32_t>(gp4::idiv64(dividend, divisor, &remainder));
    }

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00433590, FUN_00433590, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
