#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Draws four random numbers from the generator at 0x401462 (result in the low byte of eax).
// The last three bytes are each scaled by 64 (and eax,0xff / shl eax,6) in eax between the draws.
// pushal/popal restore every register, so only the generator state and the flags remain; popal
// does not restore the flags, which are those of the final `shl eax, 6` on the fourth byte
// (CF = OF = AF = 0 as the byte is shifted by only 6, SF = 0).
// FUNCTION: GP4 0x0043d158
void __cdecl FUN_0043d158(gp4::Regs* r) {
    gp4::Regs call = *r;
    gp4::call_regs(0x00401462, call);
    gp4::call_regs(0x00401462, call);
    call.eax = (call.eax & 0xffu) << 6;
    gp4::call_regs(0x00401462, call);
    call.eax = (call.eax & 0xffu) << 6;
    gp4::call_regs(0x00401462, call);
    const uint32_t scaled = (call.eax & 0xffu) << 6;

    const uint32_t flags = parityFlag(scaled) | (scaled == 0u ? 0x40u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043d158, FUN_0043d158, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
