#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 8-bit `cmp left, right`.
static inline uint32_t subFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// Reads the byte of entry esi (edx = 3 is the offset in it) through 0x0051f816; when it is 'I'
// (0x49) clears it through 0x0051f82c (cl = 0). eax is the byte read (or esi after the store),
// edx = 3; the flags come from `cmp al, 0x49` or, after the store call, from its last flag writer.
// FUNCTION: GP4 0x0043ba12
void __cdecl FUN_0043ba12(gp4::Regs* r) {
    r->edx = 3u;
    r->eax = r->esi;
    gp4::call_regs(0x0051f816, *r);

    const uint32_t value = r->eax & 0xffu;
    r->eflags = (r->eflags & ~0x8d5u) | subFlags8(value, 0x49u);
    if (value == 0x49u) {
        r->edx = 3u;
        r->ecx &= 0xffffff00u;
        r->eax = r->esi;
        gp4::call_regs(0x0051f82c, *r);
    }
}
GP4_IMPL(0x0043ba12, FUN_0043ba12, "regs(esi:i32[0..21], ebx:i32[0..16], ecx:u32) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=codes_0043ba12")
