#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

struct Unknown_00418c39 {
    uint8_t Unknown00[0x25d];
    uint8_t Value25D;
};

// Marks the 0xa8-byte entry selected by 0x00418901: clears then sets bit 8 of its first dword (the
// `cmp eax, -1` guard is always true) and stores byte +0x25d of the record at esi into the dword
// at +0xa0. eax and edi are saved and restored; the flags are those of the final `or [edi], 0x100`.
// FUNCTION: GP4 0x00418c39
void __cdecl FUN_00418c39(gp4::Regs* r) {
    const Unknown_00418c39* record = reinterpret_cast<const Unknown_00418c39*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    gp4::call_regs(0x00418901, callee);
    uint8_t* entry = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(callee.eax));

    uint32_t bits = GP4_FIELD(uint32_t, entry, 0x00) & 0xfffffeffu;
    bits |= 0x100u;
    GP4_FIELD(uint32_t, entry, 0x00) = bits;
    GP4_FIELD(uint32_t, entry, 0xa0) = record->Value25D;

    // or dword: CF = OF = AF = 0, ZF is clear because bit 8 is set
    const uint32_t flags = parityFlag(bits) | ((bits & 0x80000000u) != 0u ? 0x80u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00418c39, FUN_00418c39, "regs(esi:ptr[0x260]:layout=rec_00418c68) -> (cf, pf, af, zf, sf, of); globals=layout=state_00418c68")
