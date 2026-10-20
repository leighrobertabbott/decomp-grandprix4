#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of "neg value" (0 - value) on a dword.
static inline uint32_t negFlags32(uint32_t value) {
    const uint32_t result = 0u - value;
    return (value != 0u ? 1u : 0u) | parityFlag(result)
        | (((value ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (value == 0x80000000u ? 0x800u : 0u);
}

// Runs the condition-table pass 0x004389a5 for the record in esi with the mask DAT_00629e1c and the
// vector (DAT_00629dd4/d8/dc), then for the record in ebx with the mask DAT_00629e44 and the
// negated vector, so the second record sees the opposite direction. The vector is restored; pushal /
// popal keep every register, so only the flags of the very last `neg` remain.
// FUNCTION: GP4 0x00438a41
void __cdecl FUN_00438a41(gp4::Regs* r) {
    gp4::Regs first = *r;
    first.st_in = 0;
    first.st_out = 0;
    first.eax = GP4_GLOBAL(uint32_t, 0x00629e1c);
    gp4::call_regs(0x004389a5, first);

    GP4_GLOBAL(uint32_t, 0x00629dd4) = 0u - GP4_GLOBAL(uint32_t, 0x00629dd4);
    GP4_GLOBAL(uint32_t, 0x00629dd8) = 0u - GP4_GLOBAL(uint32_t, 0x00629dd8);
    GP4_GLOBAL(uint32_t, 0x00629ddc) = 0u - GP4_GLOBAL(uint32_t, 0x00629ddc);

    gp4::Regs second = *r;
    second.st_in = 0;
    second.st_out = 0;
    second.esi = r->ebx;
    second.ebx = r->esi;
    second.eax = GP4_GLOBAL(uint32_t, 0x00629e44);
    gp4::call_regs(0x004389a5, second);

    GP4_GLOBAL(uint32_t, 0x00629dd4) = 0u - GP4_GLOBAL(uint32_t, 0x00629dd4);
    GP4_GLOBAL(uint32_t, 0x00629dd8) = 0u - GP4_GLOBAL(uint32_t, 0x00629dd8);
    const uint32_t last = GP4_GLOBAL(uint32_t, 0x00629ddc);
    GP4_GLOBAL(uint32_t, 0x00629ddc) = 0u - last;

    r->eflags = (r->eflags & ~0x8d5u) | negFlags32(last);
}
GP4_IMPL(0x00438a41, FUN_00438a41, "regs(ebx:rec[0x66fef0,0x41c,0..21], esi:rec[0x66fef0,0x41c,0..21]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
