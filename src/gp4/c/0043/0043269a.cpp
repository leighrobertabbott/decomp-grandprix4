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

// Compares the incoming eax with a random 16-bit number (two bytes from the generator at
// 0x401462, the first as the high byte): eax = 0xffffffff when eax >= number (signed), else 0.
// edx is saved and restored; the flags are those of the cmp.
// FUNCTION: GP4 0x0043269a
void __cdecl FUN_0043269a(gp4::Regs* r) {
    const uint32_t threshold = r->eax;

    gp4::Regs call = *r;
    gp4::call_regs(0x00401462, call);
    const uint32_t high = (call.eax & 0xffu) << 8;
    call.eax = high;
    call.edx = high;
    gp4::call_regs(0x00401462, call);
    const uint32_t number = (call.eax & 0xffu) | high;

    r->eax = static_cast<int32_t>(threshold) < static_cast<int32_t>(number) ? 0u : 0xffffffffu;
    r->eflags = (r->eflags & ~0x8d5u) | subFlags32(threshold, number);
}
GP4_IMPL(0x0043269a, FUN_0043269a, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
