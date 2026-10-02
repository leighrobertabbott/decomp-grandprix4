#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0040b503
void __cdecl FUN_0040b503(gp4::Regs* r) {
    uint32_t cursor = r->esi;
    uint32_t count = r->ecx;
    uint8_t value = static_cast<uint8_t>(r->eax);
    uint32_t previous = cursor;
    // loop: ecx counts down to zero, flipping bit 6 of every byte with bit 7 set
    do {
        value = *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(cursor));
        if ((value & 0x80u) != 0u) {
            value ^= 0x40u;
        }
        *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(cursor)) = value;
        previous = cursor;
        cursor += 1u;
        count -= 1u;
    } while (count != 0u);
    // The last flag writer is `inc esi` (CF kept from the or/xor, which is 0).
    const uint32_t flags = parityFlag(cursor)
        | (((previous ^ 1u ^ cursor) & 0x10u) != 0u ? 0x10u : 0u)
        | (cursor == 0u ? 0x40u : 0u)
        | ((cursor & 0x80000000u) != 0u ? 0x80u : 0u)
        | (cursor == 0x80000000u ? 0x800u : 0u);
    r->eax = (r->eax & 0xffffff00u) | value;
    r->esi = cursor;
    r->ecx = count;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040b503, FUN_0040b503, "regs(esi:ptr[0x48]:bytes, ecx:u32[1..32], eax:u32) -> (eax, esi, ecx, cf, pf, af, zf, sf, of)")
