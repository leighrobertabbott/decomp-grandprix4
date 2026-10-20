#include <gp4/gp4.h>

// Arithmetic flags the way the pinned oracle computes them (CF PF AF ZF SF OF only).
static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

static inline uint32_t resultFlags(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// flags of "cmp a, b" / "sub a, b"
static inline uint32_t subFlags(uint32_t a, uint32_t b) {
    const uint32_t result = a - b;
    return (a < b ? 0x01u : 0u) | (((a ^ b ^ result) & 0x10u) != 0u ? 0x10u : 0u) |
           ((((a ^ b) & (a ^ result)) >> 31) != 0u ? 0x800u : 0u) | resultFlags(result);
}

// flags of "shrd eax, edx, count" (count > 1) on the 64-bit value product = edx:eax
static inline uint32_t shrdFlags(uint64_t product, uint32_t count) {
    const uint32_t result = static_cast<uint32_t>(product >> count);
    const uint32_t previous = static_cast<uint32_t>(product >> (count - 1u));
    return (previous & 1u) | resultFlags(result) | ((((previous ^ result) >> 31) & 1u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00419eeb
// Computes the signed 32-bit value DAT_00601ac0 from the car record at esi and leaves the
// last eax/edx/flags of the taken path in the registers. Chosen by the record bytes:
//   [+0x7a] & 0x20 set (the car is in the controlled mode):
//       [+0x6f] bit 0 clear -> 0 (the flags of that test are left).
//       else q = (+-DAT_00601cbc - DAT_00638ce0) * (|DAT_00638ce0| < DAT_00601cb8 ? DAT_00601cb0
//       : DAT_00601cb4) >> 14 (sign of DAT_00601cbc flipped when [+0x71] & 0x10), then limited to
//       the magnitude DAT_00601ca8.
//   [+0x7a] & 0x20 clear: 0 when [+0xfe] & 0x20 or when [+0x25f] bit 1 is clear (the flags of the
//       last test stay), else (DAT_0062786c * word[+0xb2]) >> 3.
void __cdecl FUN_00419eeb(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags = 0;

    GP4_GLOBAL(uint32_t, 0x00601ac0) = 0;

    if ((GP4_FIELD(uint8_t, rec, 0x7a) & 0x20u) != 0u) {
        if ((GP4_FIELD(uint8_t, rec, 0x6f) & 1u) == 0u) {
            // test -> zero result: ZF and PF
            flags = 0x44u;
        } else {
            uint32_t eax = GP4_GLOBAL(uint32_t, 0x00601cbc);
            if ((GP4_FIELD(uint8_t, rec, 0x71) & 0x10u) != 0u) eax = 0u - eax;
            eax -= GP4_GLOBAL(uint32_t, 0x00638ce0);

            int32_t magnitude = GP4_GLOBAL(int32_t, 0x00638ce0);
            if (magnitude < 0) magnitude = static_cast<int32_t>(0u - static_cast<uint32_t>(magnitude));

            const int32_t factor = magnitude < GP4_GLOBAL(int32_t, 0x00601cb8)
                                       ? GP4_GLOBAL(int32_t, 0x00601cb0)
                                       : GP4_GLOBAL(int32_t, 0x00601cb4);
            const int64_t product = static_cast<int64_t>(static_cast<int32_t>(eax)) * factor;
            uint32_t value = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
            const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
            const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00601ca8);

            if ((value & 0x80000000u) != 0u) {
                uint32_t mag = 0u - value;
                if (!(mag < limit)) mag = limit;
                value = 0u - mag;
                flags = subFlags(0u, mag);
            } else {
                flags = subFlags(value, limit);
                if (!(value < limit)) value = limit;
            }
            GP4_GLOBAL(uint32_t, 0x00601ac0) = value;
            r->eax = value;
            r->edx = high;
        }
    } else if ((GP4_FIELD(uint8_t, rec, 0xfe) & 0x20u) != 0u) {
        // test -> nonzero result 0x20: no flag set
        flags = 0u;
    } else if ((GP4_FIELD(uint8_t, rec, 0x25f) & 2u) == 0u) {
        flags = 0x44u;
    } else {
        const int64_t product = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062786c)) *
                                static_cast<int64_t>(static_cast<int32_t>(GP4_FIELD(uint16_t, rec, 0xb2)));
        const uint32_t value = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 3);
        GP4_GLOBAL(uint32_t, 0x00601ac0) = value;
        r->eax = value;
        r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        flags = shrdFlags(static_cast<uint64_t>(product), 3u);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00419eeb, FUN_00419eeb, "regs(esi:ptr[0x267]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
