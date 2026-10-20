#include <gp4/gp4.h>

// cmp left, right (32-bit) arithmetic flags: CF PF AF ZF SF OF.
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

// product * DAT_00603854 >> 13 (edx:eax, arithmetic), divided by the word at +0xb2, floored at 0x10.
static inline int32_t scaledQuotient(int32_t value, int32_t divisor, int32_t* remainder) {
    const int64_t product = static_cast<int64_t>(value) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603854));
    return gp4::idiv64(product >> 13, divisor, remainder);
}

// FUNCTION: GP4 0x00405489
// Runs 0x00405320 first (which fills DAT_00602d74/78 from the record), then stores two 16-bit values
//   [record+0x24c] = max(0x10, (DAT_00602d78 * DAT_00603854 >> 13) / word[record+0xb2])
//   [record+0x24e] = max(0x10, ((DAT_00602d74 + DAT_00602d78) * DAT_00603854 >> 13) / word[record+0xb2])
// eax = last stored value, ecx = the divisor, edx = the last idiv remainder, flags = last cmp eax, 0x10.
void __cdecl FUN_00405489(gp4::Regs* r) {
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x00405320, call);

    const int32_t divisor = GP4_FIELD(const uint16_t, record, 0xb2);
    int32_t remainder = 0;

    int32_t quotient = scaledQuotient(GP4_GLOBAL(int32_t, 0x00602d78), divisor, &remainder);
    uint32_t flags = subtractionFlags32(static_cast<uint32_t>(quotient), 0x10u);
    if (quotient < 0x10) {
        quotient = 0x10;
    }
    GP4_FIELD(uint16_t, record, 0x24c) = static_cast<uint16_t>(quotient);

    const int32_t sum = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00602d74) + GP4_GLOBAL(uint32_t, 0x00602d78));
    quotient = scaledQuotient(sum, divisor, &remainder);
    flags = subtractionFlags32(static_cast<uint32_t>(quotient), 0x10u);
    if (quotient < 0x10) {
        quotient = 0x10;
    }
    GP4_FIELD(uint16_t, record, 0x24e) = static_cast<uint16_t>(quotient);

    r->eax = static_cast<uint32_t>(quotient);
    r->ecx = static_cast<uint32_t>(divisor);
    r->edx = static_cast<uint32_t>(remainder);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00405489, FUN_00405489, "regs(esi:ptr[0x258]:layout=rec_00405489) -> (eax,ecx,edx,cf,pf,af,zf,sf,of); globals=layout=state_00405489")
