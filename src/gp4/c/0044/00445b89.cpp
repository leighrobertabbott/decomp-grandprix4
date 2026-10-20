#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `inc` (CF is not touched).
static inline uint32_t incFlags32(uint32_t before, uint32_t after) {
    return parityFlag(after) | ((before & 0x0fu) == 0x0fu ? 0x10u : 0u) | (after == 0u ? 0x40u : 0u)
        | ((after & 0x80000000u) != 0u ? 0x80u : 0u) | (before == 0x7fffffffu ? 0x800u : 0u);
}

// Runs the packet builder 0x0044587c with eax = 1 (a pushal / popal routine, so only the flags it leaves remain) and
// counts the call in the dword 0x0062f958. The flags are those of the final `inc dword [0x62f958]`, which keeps the
// callee's CF.
// FUNCTION: GP4 0x00445b89
void __cdecl FUN_00445b89(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.eax = 1u;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x0044587c, call);
    const uint32_t before = GP4_GLOBAL(uint32_t, 0x0062f958);
    const uint32_t after = before + 1u;
    GP4_GLOBAL(uint32_t, 0x0062f958) = after;
    r->eax = 1u;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x01u) | incFlags32(before, after);
}
GP4_IMPL(0x00445b89, FUN_00445b89, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
