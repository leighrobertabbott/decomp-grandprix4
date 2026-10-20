#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal / segment-register save wrapper around the slot predicate 0x00518d01 (eax = slot index; the callee answers
// 0 or -1). Every register is restored; CF = 1 (`stc`) when the answer is -1 and 0 (`clc`) otherwise, the other
// flags are those of `cmp eax, -1`.
// FUNCTION: GP4 0x00518735
void __cdecl FUN_00518735(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x00518d01, c);
    const uint32_t arith = gp4::flags::sub32(c.eax, 0xffffffffu);
    const uint32_t carry = (c.eax == 0xffffffffu) ? 1u : 0u;
    r->eflags = gp4::flags::merge(r->eflags, (arith & ~1u) | carry);
}
GP4_IMPL(0x00518735, FUN_00518735, "regs(eax:u32[0..63]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
