#include <gp4/gp4.h>

// Piecewise-linear lookup table: Last is the index of the final point, followed by
// (X, Y) pairs at +4 + 8*i / +8 + 8*i. The input in eax is mapped to a Y value.
struct Unknown_Curve {
    int32_t Last;
    int32_t Points[2];   // X0, Y0, X1, Y1, ... (Last + 1 pairs)
};

enum : uint32_t {
    kArithFlags = 0x8d5u,   // OF SF ZF AF PF CF
    kPF = 0x4u, kAF = 0x10u, kZF = 0x40u, kSF = 0x80u, kOF = 0x800u
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t low = value & 0xffu;
    low ^= low >> 4;
    low ^= low >> 2;
    low ^= low >> 1;
    return (low & 1u) == 0u ? kPF : 0u;
}

static inline uint32_t resultFlags(uint32_t result) {
    return parityFlag(result) | (result == 0u ? kZF : 0u) | ((result >> 31) != 0u ? kSF : 0u);
}

static inline uint32_t subFlags(uint32_t a, uint32_t b) {
    const uint32_t result = a - b;
    return (a < b ? 1u : 0u) | ((a ^ b ^ result) & kAF) | resultFlags(result) |
           ((((a ^ b) & (a ^ result)) >> 31) != 0u ? kOF : 0u);
}

static inline uint32_t addFlags(uint32_t a, uint32_t b) {
    const uint32_t result = a + b;
    return (result < a ? 1u : 0u) | ((a ^ b ^ result) & kAF) | resultFlags(result) |
           (((~(a ^ b) & (a ^ result)) >> 31) != 0u ? kOF : 0u);
}

// FUNCTION: GP4 0x004118dd
void __cdecl FUN_004118dd(gp4::Regs* r) {
    const uint32_t input = r->eax;
    const int32_t* curve = reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(r->edx));
    const int32_t last = curve[0];

    uint32_t result;
    uint32_t flags;
    int32_t index = 0;
    for (;;) {
        const uint32_t x = static_cast<uint32_t>(curve[1 + index * 2]);
        if (static_cast<int32_t>(input) < static_cast<int32_t>(x)) {
            // Below this point. cmp sets CF; the following dec keeps CF and sets the rest.
            const uint32_t cmpCarry = input < x ? 1u : 0u;
            index -= 1;
            if (index < 0) {
                // Below the first point: clamp to the first Y. dec 0 -> 0xffffffff.
                result = static_cast<uint32_t>(curve[2]);
                flags = cmpCarry | kPF | kAF | kSF;
            } else {
                const uint32_t x0 = static_cast<uint32_t>(curve[1 + index * 2]);
                const uint32_t y0 = static_cast<uint32_t>(curve[2 + index * 2]);
                const uint32_t spanX = static_cast<uint32_t>(curve[3 + index * 2]) - x0;
                const uint32_t spanY = static_cast<uint32_t>(curve[4 + index * 2]) - y0;
                const int64_t product = static_cast<int64_t>(static_cast<int32_t>(input - x0)) *
                                        static_cast<int64_t>(static_cast<int32_t>(spanY));
                int32_t remainder;
                const uint32_t quotient = static_cast<uint32_t>(gp4::idiv64(product, static_cast<int32_t>(spanX), &remainder));
                r->ebp = spanX;
                r->ebx = spanY;
                result = quotient + y0;
                flags = addFlags(quotient, y0);
            }
            break;
        }
        index += 1;
        if (index > last) {
            // Past the last point: clamp to the last Y. The flags come from cmp index, Last.
            result = static_cast<uint32_t>(curve[2 + last * 2]);
            flags = subFlags(static_cast<uint32_t>(index), static_cast<uint32_t>(last));
            break;
        }
    }

    r->eax = result;
    r->eflags = (r->eflags & ~kArithFlags) | flags;
}
GP4_IMPL(0x004118dd, FUN_004118dd, "regs(eax:i32[-12000..12000], edx:ptr[0x4c]:layout=curve_004118dd) -> (eax, ebx, ebp, cf, pf, af, zf, sf, of)")
