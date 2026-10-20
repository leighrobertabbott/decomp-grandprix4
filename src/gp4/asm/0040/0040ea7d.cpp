#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `test byte, mask` / `or al, al` (result = the 8-bit logical result): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of `sub`/`cmp` on bytes (left - right).
static inline uint32_t subFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of `sub`/`cmp` on dwords (left - right).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Records addressed through ebx and esi (only the fields this routine reads).
struct Unknown_0040ea7d_Rec {
    uint8_t Unknown00[0x1c];
    uint32_t Unknown1C;
    uint8_t Unknown20[0x70 - 0x20];
    int8_t Unknown70;
    uint8_t Unknown71[0x7a - 0x71];
    uint8_t Flags7A;
    uint8_t Unknown7B[0x8d - 0x7b];
    uint8_t Flags8D;
    uint8_t Unknown8E[0xc0 - 0x8e];
    uint32_t UnknownC0;
};

static inline bool compareRecords(const Unknown_0040ea7d_Rec* other, const Unknown_0040ea7d_Rec* self,
                                  uint32_t& eax, uint32_t& flags) {
    // Returns true when the routine ends with stc (the pair is rejected), false for clc.
    uint32_t result = other->Flags7A & 0x80u;
    flags = logicFlags8(result);
    if (result != 0u) return true;
    result = self->Flags7A & 0x80u;
    flags = logicFlags8(result);
    if (result != 0u) return true;
    result = other->Flags8D & 2u;
    flags = logicFlags8(result);
    if (result != 0u) return true;
    result = self->Flags8D & 2u;
    flags = logicFlags8(result);
    if (result != 0u) return true;

    flags = subFlags8(static_cast<uint8_t>(self->Unknown70), 2u);
    if (self->Unknown70 < 2) return true;

    const uint32_t difference = (static_cast<uint32_t>(static_cast<uint8_t>(self->Unknown70))
                                 - static_cast<uint32_t>(static_cast<uint8_t>(other->Unknown70))) & 0xffu;
    eax = (eax & 0xffffff00u) | difference;
    flags = subFlags8(static_cast<uint8_t>(self->Unknown70), static_cast<uint8_t>(other->Unknown70));
    if (difference != 0u) {
        flags = logicFlags8(difference);
        if ((difference & 0x80u) == 0u) return true;
        eax = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int8_t>(difference)));
        flags = subFlags32(other->Unknown1C, self->Unknown1C);
        if (other->Unknown1C >= self->Unknown1C) return false;
        eax += 1u;
        flags = subFlags32(eax, 0u);
        return static_cast<int32_t>(eax) >= 0;
    }

    eax = self->UnknownC0;
    flags = subFlags32(eax, 0xc0000000u);
    if (eax == 0xc0000000u) return true;
    flags = subFlags32(other->UnknownC0, 0xc0000000u);
    if (other->UnknownC0 == 0xc0000000u) return true;
    flags = subFlags32(eax, other->UnknownC0);
    eax -= other->UnknownC0;
    return (eax & 0x80000000u) != 0u;
}

// Compares the records at ebx and esi: carry set (stc) rejects the pair, clear (clc) accepts it.
// Rejected when either has bit 7 of byte +0x7a or bit 1 of byte +0x8d set, when byte +0x70 of esi
// is below 2, when the +0x70 bytes agree and either dword +0xc0 is 0xc0000000 or esi's is lower
// (signed difference), or when esi's byte is above ebx's and ebx's dword +0x1c is below esi's with
// the byte difference of -1. All registers but eax are restored; the other flags are those of the
// last compare, test or subtraction.
// FUNCTION: GP4 0x0040ea7d
void __cdecl FUN_0040ea7d(gp4::Regs* r) {
    const Unknown_0040ea7d_Rec* other = reinterpret_cast<const Unknown_0040ea7d_Rec*>(static_cast<uintptr_t>(r->ebx));
    const Unknown_0040ea7d_Rec* self = reinterpret_cast<const Unknown_0040ea7d_Rec*>(static_cast<uintptr_t>(r->esi));

    uint32_t eax = r->eax;
    uint32_t flags = 0u;
    const bool rejected = compareRecords(other, self, eax, flags);

    r->eax = eax;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d4u) | (rejected ? 1u : 0u);
}
GP4_IMPL(0x0040ea7d, FUN_0040ea7d, "regs(eax:u32, ebx:ptr[0xc8]:layout=pair_0040ea7d, esi:ptr[0xc8]:layout=pair_0040ea7d) -> (eax, cf, pf, af, zf, sf, of)")
