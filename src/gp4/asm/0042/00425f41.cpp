#include <gp4/gp4.h>

// Looks up a table value (0x63a194, indexed by |dword +0x98| / 4 >> 20, plus 0x168 for bit 0 of
// byte +0x8d), scales it by the word at +0xd6 (>> 14) and by DAT_00603854 (>> 2), and compares the
// result with the signed word at 0x934df4 << 14. Below it, the word at +0x24a becomes 0x4000 and
// eax takes the limit; otherwise +0x24a receives the Q14 ratio result / limit clamped to 0..0x4000
// (limit 0 counts as 1) and eax keeps the result, with edx the remainder of that division.
// ebx and ebp are saved and restored; ecx keeps |dword +0x98| / 4. Flags come from the last
// flag writer: result vs limit on the first path; on the second the compare with 0x4000 of the
// ratio after only the clamp at zero (the store of 0x4000 above the limit is a plain mov).
static inline uint32_t parity_flag(uint32_t value) {
    uint32_t b = value & 0xffu;
    b ^= b >> 4;
    b ^= b >> 2;
    b ^= b >> 1;
    return (b & 1u) ? 0u : 4u;   // PF set when the low byte has an even number of set bits
}

// Flags of CMP a, b.
static inline uint32_t cmp_flags(uint32_t a, uint32_t b) {
    const uint32_t res = a - b;
    return (a < b ? 1u : 0u)
        | parity_flag(res)
        | ((a ^ b ^ res) & 0x10u)
        | (res == 0u ? 0x40u : 0u)
        | ((res & 0x80000000u) ? 0x80u : 0u)
        | ((((a ^ b) & (a ^ res)) >> 31) << 11);
}

// FUNCTION: GP4 0x00425f41
void __cdecl FUN_00425f41(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const uint32_t limit = static_cast<uint32_t>(static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x00934df4))) << 14;

    int32_t speed = GP4_FIELD(int32_t, rec, 0x98) >> 2;
    if (speed < 0) speed = -speed;
    uint32_t index = static_cast<uint32_t>(speed) >> 20;
    if (GP4_FIELD(uint8_t, rec, 0x8d) & 1u) index += 0x168u;

    const int64_t scaled = static_cast<int64_t>(GP4_ARRAY(int32_t, 0x0063a194)[index])
        * static_cast<int64_t>(GP4_FIELD(uint16_t, rec, 0xd6));
    const int32_t stage = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(scaled) >> 14));
    const int64_t product = static_cast<int64_t>(stage) * GP4_GLOBAL(int32_t, 0x00603854);
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 2);

    r->ecx = static_cast<uint32_t>(speed);
    if (static_cast<int32_t>(result) < static_cast<int32_t>(limit)) {
        GP4_FIELD(uint16_t, rec, 0x24a) = 0x4000u;
        r->eax = limit;
        r->edx = high;
        r->eflags = (r->eflags & ~0x8d5u) | cmp_flags(result, limit);
    } else {
        const int32_t divisor = limit == 0u ? 1 : static_cast<int32_t>(limit);
        int32_t remainder;
        int32_t ratio = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(result)) << 14, divisor, &remainder);
        if (ratio < 0) ratio = 0;
        const int32_t compared = ratio;   // value seen by the last flag writer (cmp eax, 0x4000)
        if (ratio > 0x4000) ratio = 0x4000;
        GP4_FIELD(uint16_t, rec, 0x24a) = static_cast<uint16_t>(ratio);
        r->eax = result;
        r->edx = static_cast<uint32_t>(remainder);
        r->eflags = (r->eflags & ~0x8d5u) | cmp_flags(static_cast<uint32_t>(compared), 0x4000u);
    }
}
GP4_IMPL(0x00425f41, FUN_00425f41, "regs(esi:ptr[0x252]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=tab_00425f41")
