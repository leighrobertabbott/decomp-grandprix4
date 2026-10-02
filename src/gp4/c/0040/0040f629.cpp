#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0040f629
void __cdecl FUN_0040f629(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    // call 0x426a66 (esi = body); it rewrites 0x006037fc, eax, edx and the flags
    gp4::Regs callee = *r;
    gp4::call_regs(0x00426a66, callee);

    // eax = (word [esi+0xda] << 16) + [0x6037fc]; cdq; idiv [0x603854]
    const int32_t dividend = static_cast<int32_t>(
        (static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, body, 0xda))) << 16)
        + GP4_GLOBAL(uint32_t, 0x006037fc));
    const int32_t quotient = dividend / GP4_GLOBAL(int32_t, 0x00603854);

    // imul [0x601eb4]; shrd eax, edx, 14
    const int64_t product1 = static_cast<int64_t>(quotient) * GP4_GLOBAL(int32_t, 0x00601eb4);
    const int32_t scaled1 = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product1) >> 14));

    // edx = [esi+0x98] >> 8 (arithmetic); imul edx; shrd eax, edx, 14
    const int32_t factor = GP4_FIELD(int32_t, body, 0x98) >> 8;
    const int64_t product2 = static_cast<int64_t>(scaled1) * factor;
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product2) >> 32);
    uint32_t value = static_cast<uint32_t>(static_cast<uint64_t>(product2) >> 14);

    GP4_FIELD(uint32_t, body, 0x36c) = value;
    GP4_FIELD(uint32_t, body, 0x370) = value;

    // or edx, edx; js skips the clear
    if ((high & 0x80000000u) == 0u) {
        value = 0u;
    }
    GP4_FIELD(uint32_t, body, 0x374) = value;
    GP4_FIELD(uint32_t, body, 0x378) = value;

    // or edx, edx: CF = OF = 0, AF cleared in the pinned model
    const uint32_t flags = parityFlag(high)
        | (high == 0u ? 0x40u : 0u)
        | ((high >> 31) << 7);
    r->eax = value;
    r->edx = high;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040f629, FUN_0040f629, "regs(esi:ptr[0x380]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_004125ce")
