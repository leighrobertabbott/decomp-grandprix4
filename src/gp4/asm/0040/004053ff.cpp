#include <gp4/gp4.h>

// imul a, b (signed 32x32 -> 64) followed by shrd eax, edx, 14: low dword of product >> 14.
static inline uint32_t mulShift14(int32_t left, int32_t right) {
    const int64_t product = static_cast<int64_t>(left) * static_cast<int64_t>(right);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// FUNCTION: GP4 0x004053ff
// Stores a 0..0x4000 value at record word +0x176: zero when dword +0x240 is zero, otherwise
// (((dDAT_00602dd4 - DAT_00602dbc) * sum of the four DAT_0060309c/a0/a8/a4) >> 14, * word +0xba >> 14,
// * DAT_0062d294 >> 14, * DAT_00603854 >> 9) / max(sword +0xb2, 0x200), clamped to 0x4000.
// Registers left behind: eax = stored value, ecx = divisor, edx = idiv remainder, flags of
// cmp eax, 0x4000 (or of the test when +0x240 is zero: eax = 0, ecx and edx untouched).
void __cdecl FUN_004053ff(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags;
    uint32_t value = 0;

    if (GP4_FIELD(const uint32_t, record, 0x240) == 0u) {
        // test dword, 0xffffffff of zero: ZF and PF set, CF/OF/AF/SF clear.
        flags = 0x44u;
    } else {
        const int32_t difference = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00602dd4) - GP4_GLOBAL(uint32_t, 0x00602dbc));
        uint32_t sum = GP4_GLOBAL(uint32_t, 0x0060309c);
        sum += GP4_GLOBAL(uint32_t, 0x006030a0);
        sum += GP4_GLOBAL(uint32_t, 0x006030a8);
        sum += GP4_GLOBAL(uint32_t, 0x006030a4);

        uint32_t scaled = mulShift14(difference, static_cast<int32_t>(sum));
        scaled = mulShift14(static_cast<int32_t>(GP4_FIELD(const uint16_t, record, 0xba)), static_cast<int32_t>(scaled));
        scaled = mulShift14(static_cast<int32_t>(scaled), GP4_GLOBAL(int32_t, 0x0062d294));

        int32_t divisor = GP4_FIELD(const int16_t, record, 0xb2);
        if (divisor < 0x200) {
            divisor = 0x200;
        }
        const int64_t dividend = (static_cast<int64_t>(static_cast<int32_t>(scaled)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603854))) >> 9;
        int32_t remainder = 0;
        int32_t quotient = gp4::idiv64(dividend, divisor, &remainder);

        // cmp eax, 0x4000 sets the flags; the clamp's mov leaves them alone.
        const uint32_t result = static_cast<uint32_t>(quotient) - 0x4000u;
        uint32_t parity = result & 0xffu;
        parity ^= parity >> 4;
        parity ^= parity >> 2;
        parity ^= parity >> 1;
        flags = (static_cast<uint32_t>(quotient) < 0x4000u ? 1u : 0u)
            | ((~parity & 1u) << 2)
            | (((static_cast<uint32_t>(quotient) ^ 0x4000u ^ result) >> 4) & 1u) << 4
            | (result == 0u ? 0x40u : 0u)
            | ((result >> 31) << 7)
            | ((((static_cast<uint32_t>(quotient) ^ 0x4000u) & (static_cast<uint32_t>(quotient) ^ result)) >> 31) << 11);

        if (quotient > 0x4000) {
            quotient = 0x4000;
        }
        value = static_cast<uint32_t>(quotient);
        r->ecx = static_cast<uint32_t>(divisor);
        r->edx = static_cast<uint32_t>(remainder);
    }

    GP4_FIELD(uint16_t, record, 0x176) = static_cast<uint16_t>(value);
    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004053ff, FUN_004053ff, "regs(esi:ptr[0x248]:layout=rec_004053ff) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=state_00405489+state_004053ff")
