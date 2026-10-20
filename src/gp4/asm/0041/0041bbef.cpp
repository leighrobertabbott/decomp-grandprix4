#include <gp4/gp4.h>

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (result < left ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0041bbef
void __cdecl FUN_0041bbef(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    const int32_t sum = static_cast<int32_t>(static_cast<uint32_t>(GP4_FIELD(int32_t, body, 0x25c))
                                             + static_cast<uint32_t>(GP4_FIELD(int32_t, body, 0x55c)));
    const int32_t g10 = GP4_GLOBAL(int32_t, 0x00623b10);

    // imul edx: edx:eax = sum * g10 -> ebp (low), ecx (high)
    const int64_t first = static_cast<int64_t>(sum) * static_cast<int64_t>(g10);

    // imul [0x623b6c] ; shrd eax, edx, 0xe
    const int64_t scaled = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006258ac))
                           * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00623b6c));
    const int32_t mid = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(scaled) >> 0xe));

    // shl edx, 1 ; imul edx
    const int32_t twice = static_cast<int32_t>(static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00623af4)) << 1);
    const int64_t second = static_cast<int64_t>(mid) * static_cast<int64_t>(twice);

    // sub eax, ebp ; sbb edx, ecx : 64-bit difference
    const int64_t dividend = static_cast<int64_t>(static_cast<uint64_t>(second) - static_cast<uint64_t>(first));

    // ebp = 2*[0x623af4] + [0x623b10]; the add leaves the flags that idiv keeps
    const uint32_t divisorBits = (static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00623af4)) << 1);
    const uint32_t divisor = divisorBits + static_cast<uint32_t>(g10);
    const uint32_t flags = additionFlags32(divisorBits, static_cast<uint32_t>(g10));

    int32_t remainder;
    const int32_t quotient = gp4::idiv64(dividend, static_cast<int32_t>(divisor), &remainder);
    GP4_GLOBAL(int32_t, 0x00623b84) = quotient;

    r->eax = static_cast<uint32_t>(quotient);
    r->edx = static_cast<uint32_t>(remainder);
    r->ecx = static_cast<uint32_t>(static_cast<uint64_t>(first) >> 32);
    r->ebp = divisor;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041bbef, FUN_0041bbef, "regs(ebx:ptr[0x564]:layout=rec_0041bbef) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0041bbef")
