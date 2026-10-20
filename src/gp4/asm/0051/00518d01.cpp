#include <gp4/gp4.h>
#include <gp4/flags.h>

// eax = slot index. Passes the slot's handle [0x0088f948 + 4 * eax] to the stdcall predicate
// 0x0050a090 (`mov al, 1 ; ret 4`) and turns its result into 0 / -1 (nonzero -> -1). Flags are those
// of `cmp eax, 0`.
// FUNCTION: GP4 0x00518d01
void __cdecl FUN_00518d01(gp4::Regs* r) {
    const uint32_t handle = GP4_ARRAY(uint32_t, 0x0088f948)[r->eax];
    const uint32_t args[1] = {handle};
    gp4::Regs c = *r;
    gp4::call_regs_stdcall(0x0050a090, c, args);
    const uint32_t flags = gp4::flags::sub32(c.eax, 0u);
    if (c.eax != 0u) {
        c.eax = 0xffffffffu;
    }
    c.eflags = gp4::flags::merge(c.eflags, flags);
    *r = c;
}
GP4_IMPL(0x00518d01, FUN_00518d01, "regs(eax:u32[0..63]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
