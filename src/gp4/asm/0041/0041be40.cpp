#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Takes the signed byte at esi+0x72 (at least 1), looks it up in the table at esi+0x1c6 through
// 0x0041242f (which returns a scaled divisor quotient in eax), multiplies by DAT_00638ce0 and
// returns the product >> 22 in eax. edx and ecx are preserved; the flags are those of the final
// "shrd eax, edx, 22" (pinned oracle model: AF = 0, CF = bit 21 of the product,
// OF = bit31(product>>21 ^ product>>22)).
// FUNCTION: GP4 0x0041be40
void __cdecl FUN_0041be40(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    int32_t index = static_cast<int8_t>(GP4_FIELD(uint8_t, record, 0x72));
    if (index < 1) {
        index = 1;
    }

    gp4::Regs call = *r;
    call.eax = (r->eax & 0xffffff00u) | static_cast<uint32_t>(index & 0xff);
    call.edx = r->esi + 0x1c6u;
    gp4::call_regs(0x0041242f, call);

    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(call.eax))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00638ce0));
    const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 22);
    const uint32_t previous = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 21);

    const uint32_t flags = static_cast<uint32_t>((static_cast<uint64_t>(product) >> 21) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((previous ^ result) >> 31) != 0u) ? 0x800u : 0u);

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041be40, FUN_0041be40, "regs(esi:ptr[0x250]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
