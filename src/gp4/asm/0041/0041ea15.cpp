#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

struct Unknown_0041ea15_Record {
    uint8_t Unknown00[0xf4];
    uint32_t UnknownF4;
};

// Fills the four equal cells 0x00637ff0..0x00637ffc with the dword at +0xf4 and derives the next four
// (0x00638000.. = 0, 0, 0, 6 * value) by the original's lea/sub chain, which cancels exactly.
// eax ends 0, ebp = 3 * value, edx = 6 * value; the flags are those of the final `shl edx, 1`.
// FUNCTION: GP4 0x0041ea15
void __cdecl FUN_0041ea15(gp4::Regs* r) {
    const Unknown_0041ea15_Record* record = reinterpret_cast<const Unknown_0041ea15_Record*>(static_cast<uintptr_t>(r->edi));
    const uint32_t value = record->UnknownF4;

    GP4_GLOBAL(uint32_t, 0x00637ff0) = value;
    GP4_GLOBAL(uint32_t, 0x00637ff4) = value;
    GP4_GLOBAL(uint32_t, 0x00637ff8) = value;
    GP4_GLOBAL(uint32_t, 0x00637ffc) = value;

    const uint32_t triple = value + value * 2u;
    const uint32_t sextuple = triple << 1;
    GP4_GLOBAL(uint32_t, 0x00638000) = 0u;
    GP4_GLOBAL(uint32_t, 0x00638004) = 0u;
    GP4_GLOBAL(uint32_t, 0x00638008) = 0u;
    GP4_GLOBAL(uint32_t, 0x0063800c) = sextuple;

    // shl edx, 1 (pinned oracle model): CF = bit shifted out, AF = 0, OF = new sign ^ CF
    const uint32_t carry = triple >> 31;
    const uint32_t sign = sextuple >> 31;
    const uint32_t flags = carry
        | parityFlag(sextuple)
        | (sextuple == 0u ? 0x40u : 0u)
        | (sign << 7)
        | ((sign ^ carry) << 11);

    r->eax = 0u;
    r->edx = sextuple;
    r->ebp = triple;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041ea15, FUN_0041ea15, "regs(edi:ptr[0xfc]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
