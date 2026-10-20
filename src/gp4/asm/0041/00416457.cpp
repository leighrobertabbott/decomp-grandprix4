#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of "cmp left, right" on a dword
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// DAT_00601ffc = 0x4000, or 0 when bit 8 of byte [esi+0xce] is set and a random byte from
// 0x401462 is (signed) below DAT_00602000. eax holds that byte on the drawn paths.
// FUNCTION: GP4 0x00416457
void __cdecl FUN_00416457(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    GP4_GLOBAL(uint32_t, 0x00601ffc) = 0x4000u;

    if ((GP4_FIELD(uint8_t, record, 0xce) & 8u) == 0u) {
        // test byte [esi+0xce], 8 with a zero result
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    gp4::Regs call = *r;
    gp4::call_regs(0x00401462, call);
    const uint32_t value = call.eax & 0xffu;
    const int32_t limit = GP4_GLOBAL(int32_t, 0x00602000);
    if (static_cast<int32_t>(value) < limit) {
        GP4_GLOBAL(uint32_t, 0x00601ffc) = 0u;
    }

    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | subFlags32(value, static_cast<uint32_t>(limit));
}
GP4_IMPL(0x00416457, FUN_00416457, "regs(eax:u32, esi:ptr[0xd6]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
