#include <gp4/gp4.h>

struct Unknown_0063decc { uint32_t Unknown00; uint32_t Unknown04; uint8_t Unknown08[0x24]; };

// FUNCTION: GP4 0x0041891a
// Finds the first of the 16 entries (stride 0x2c) at 0x0063decc whose first two dwords are both
// zero. Found: eax = the entry address, CF clear and the other flags those of the "or" (ZF, PF set).
// None: eax = 0x10 (the exhausted counter), CF set and the flags of the final "cmp eax, 0x10".
// edx and edi are preserved.
void __cdecl FUN_0041891a(gp4::Regs* r) {
    const Unknown_0063decc* entries = GP4_ARRAY(Unknown_0063decc, 0x0063decc);
    for (uint32_t index = 0; index != 0x10u; ++index) {
        if ((entries[index].Unknown00 | entries[index].Unknown04) == 0u) {
            r->eax = (uint32_t)(uintptr_t)&entries[index];
            r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
            return;
        }
    }
    r->eax = 0x10u;
    r->eflags = (r->eflags & ~0x8d5u) | 0x45u;
}
GP4_IMPL(0x0041891a, FUN_0041891a, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=slots_0041891a")
