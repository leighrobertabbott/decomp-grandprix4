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

// FUNCTION: GP4 0x00413597
void __cdecl FUN_00413597(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    if ((GP4_FIELD(uint8_t, body, 0xe6) & 8u) != 0u) {
        // test byte [esi+0xe6], 8 -> 8: every arithmetic flag clear (PF is odd-parity clear)
        GP4_GLOBAL(uint32_t, 0x00603958) = GP4_GLOBAL(uint32_t, 0x0060394c);
        r->eax = GP4_GLOBAL(uint32_t, 0x0060394c);
        r->eflags = r->eflags & ~0x8d5u;
        return;
    }

    // |int16 [esi+0x23a]| - [0x603944], floored at zero
    int32_t first = GP4_FIELD(int16_t, body, 0x23a);
    if (first < 0) first = -first;
    first = static_cast<int32_t>(static_cast<uint32_t>(first) - GP4_GLOBAL(uint32_t, 0x00603944));
    if (first < 0) first = 0;

    // (first * [0x603948]) >> 14, then times |int16 [esi+0x9a]| >> 14
    const uint64_t scaled = static_cast<uint64_t>(static_cast<int64_t>(first) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603948)));
    const int32_t narrowed = static_cast<int32_t>(static_cast<uint32_t>(scaled >> 14));
    int32_t magnitude = GP4_FIELD(int16_t, body, 0x9a);
    if (magnitude < 0) magnitude = -magnitude;
    const uint64_t product = static_cast<uint64_t>(static_cast<int64_t>(narrowed) * static_cast<int64_t>(magnitude));
    const uint32_t sum = static_cast<uint32_t>(product >> 14) + GP4_GLOBAL(uint32_t, 0x0060393c);

    // cmp eax, [0x603940] ; jle: the result is the smaller (signed) of the sum and the limit
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00603940);
    const uint32_t result = static_cast<int32_t>(sum) > static_cast<int32_t>(limit) ? limit : sum;
    GP4_GLOBAL(uint32_t, 0x00603958) = result;

    r->eax = result;
    r->edx = static_cast<uint32_t>(product >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | subtractionFlags32(sum, limit);
}
GP4_IMPL(0x00413597, FUN_00413597, "regs(edx:u32, esi:ptr[0x242]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
