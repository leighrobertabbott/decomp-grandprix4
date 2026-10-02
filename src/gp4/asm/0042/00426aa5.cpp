#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x00426aa5
void __cdecl FUN_00426aa5(gp4::Regs* r) {
    const uint32_t target = r->eax;
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x006247c0);
    // sign-extended current value stored at +0xee
    const uint32_t current = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, record, 0xee)));

    // clamp the signed difference (target - current) to +-limit
    const uint32_t diff = target - current;
    uint32_t step;
    if (static_cast<int32_t>(diff) < 0) {
        uint32_t magnitude = 0u - diff;
        if (magnitude >= limit) {
            magnitude = limit;
        }
        step = 0u - magnitude;
    } else {
        step = diff;
        if (step >= limit) {
            step = limit;
        }
    }

    // result = step + current; the add leaves the arithmetic flags
    const uint32_t result = step + current;
    uint32_t flags = 0;
    if (result < step) flags |= 0x001u;                                  // CF
    flags |= parityFlag(result);                                         // PF
    if (((step ^ current ^ result) & 0x10u) != 0u) flags |= 0x010u;      // AF
    if (result == 0u) flags |= 0x040u;                                   // ZF
    if ((result & 0x80000000u) != 0u) flags |= 0x080u;                   // SF
    if ((((step ^ result) & (current ^ result)) & 0x80000000u) != 0u) flags |= 0x800u;  // OF

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00426aa5, FUN_00426aa5, "regs(eax:u32, esi:ptr[0xf6]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
