#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of "cmp left, right" on a dword
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x0040e9ac
// ebx = 1000 * max(0, ((word [edi+0x1a] & 0x7ff) - (word [table[byte [esi+0x74]] + 0x1a] & 0x7ff)) * 0x400
//                     + (int16)[esi+0x96]) / max((int16)[esi+0x9a], 0x3ab).
// eax and edx are preserved; the flags are those of the "cmp ebx, 0x3ab" (IDIV leaves them).
void __cdecl FUN_0040e9ac(gp4::Regs* r) {
    uint8_t* other = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* mine = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    const uint32_t slot = other[0x74];
    uint8_t* peer = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_ARRAY(uint32_t, 0x006279ec)[slot]));
    const uint32_t peerKey = GP4_FIELD(uint16_t, peer, 0x1a) & 0x7ffu;
    const uint32_t myKey = GP4_FIELD(uint16_t, mine, 0x1a) & 0x7ffu;

    int32_t dividend = (int32_t)(myKey - peerKey) * 0x400 + (int32_t)GP4_FIELD(int16_t, other, 0x96);
    if (dividend < 0) {
        dividend = 0;
    }

    int32_t divisor = (int32_t)GP4_FIELD(int16_t, other, 0x9a);
    const uint32_t flags = cmpFlags32((uint32_t)divisor, 0x3abu);
    if (divisor < 0x3ab) {
        divisor = 0x3ab;
    }

    int32_t remainder;
    r->ebx = (uint32_t)gp4::idiv64((int64_t)dividend * 0x3e8, divisor, &remainder);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040e9ac, FUN_0040e9ac, "regs(edi:ptr[0x40]:bytes, esi:ptr[0xa4]:layout=rec_0040e9ac) -> (ebx, cf, pf, af, zf, sf, of); globals=layout=state_0040e9ac")
