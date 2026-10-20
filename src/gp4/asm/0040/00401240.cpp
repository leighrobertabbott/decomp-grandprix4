#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00401240
// Angle of the vector (x = eax, y = edx) as a signed 16-bit value (0x4000 = quarter turn, 0x8000 = half):
// the smaller magnitude over the larger one in Q14, looked up through the ratio (a short linear stretch
// below 0x478 using DAT_006f2bf0, else the word table at 0x007c7c60), mirrored when |x| >= |y|, then
// negated for opposite signs and offset by 0x8000 for negative y. Returns the sign-extended word in eax;
// a zero larger magnitude returns 0. All other registers are preserved; the flags are those of the
// last flag writer ("or ecx, ecx", "add ax, 0x8000", or the zero test).
void __cdecl FUN_00401240(gp4::Regs* r) {
    const uint32_t x = r->eax;
    const uint32_t y = r->edx;

    uint32_t absX = x;
    if (static_cast<int32_t>(absX) < 0) {
        absX = 0u - absX;
    }
    uint32_t absY = y;
    if (static_cast<int32_t>(absY) < 0) {
        absY = 0u - absY;
    }
    const uint32_t signs = x ^ y;

    const bool xLarger = static_cast<int32_t>(absX) >= static_cast<int32_t>(absY);
    const uint32_t numerator = xLarger ? absY : absX;
    const uint32_t denominator = xLarger ? absX : absY;

    if (denominator == 0u) {
        r->eax = 0;
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;  // test of zero: ZF and PF set
        return;
    }

    uint32_t remainder;
    const uint32_t q = gp4::div64(static_cast<uint64_t>(numerator) << 14, denominator, &remainder);

    uint16_t value;
    if (q < 0x478u) {
        const int32_t stretched = static_cast<int32_t>(static_cast<int16_t>(q));
        value = static_cast<uint16_t>(static_cast<uint32_t>(
            static_cast<uint64_t>(static_cast<int64_t>(stretched) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006f2bf0))) >> 14));
    } else {
        const uint32_t offset = (static_cast<uint32_t>(static_cast<int32_t>(q) >> 2)) & 0xfffeu;
        value = *reinterpret_cast<const uint16_t*>(GP4_ARRAY(const uint8_t, 0x007c7c60) + offset);
    }
    if (xLarger) {
        value = static_cast<uint16_t>(0x4000u - value);
    }
    if (static_cast<int32_t>(signs) < 0) {
        value = static_cast<uint16_t>(0u - value);
    }

    uint32_t flags;
    if (static_cast<int32_t>(y) < 0) {
        const uint32_t sum = static_cast<uint32_t>(value) + 0x8000u;
        const uint32_t result = sum & 0xffffu;
        flags = (sum > 0xffffu ? 1u : 0u)
            | parityFlag8(result)
            | (((value ^ 0x8000u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
            | (result == 0u ? 0x40u : 0u)
            | ((result & 0x8000u) != 0u ? 0x80u : 0u)
            | ((~(value ^ 0x8000u) & (value ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
        value = static_cast<uint16_t>(result);
    } else {
        // "or ecx, ecx" on y: CF = OF = AF = 0, SF clear because y >= 0
        flags = parityFlag8(y) | (y == 0u ? 0x40u : 0u);
    }

    r->eax = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(value)));
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00401240, FUN_00401240, "regs(eax:u32, edx:u32) -> (eax, cf, pf, af, zf, sf, of)")
