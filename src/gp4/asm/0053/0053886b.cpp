#include <gp4/gp4.h>
#include <gp4/flags.h>

// Checks the pool slot (eax & 0x3ff) with 0x0053888e (key eax >> 10); when it reports a live slot
// (eax = 0xffffffff) the slot is removed with 0x00538921 (eax = the slot's address). ebx keeps the
// incoming eax. The flags are those of `cmp eax, -1` when the slot is not live, else whatever the removal
// leaves.
// FUNCTION: GP4 0x0053886b
void __cdecl FUN_0053886b(gp4::Regs* r) {
    const uint32_t id = r->eax;
    r->ebx = id;
    gp4::call_regs(0x0053888e, *r);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(r->eax, 0xffffffffu));
    if (r->eax != 0xffffffffu) {
        return;
    }
    const uint32_t offset = (id & 0x3ffu) * 0x18u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(offset, 0x00829cacu));
    r->eax = offset + 0x00829cacu;
    gp4::call_regs(0x00538921, *r);
}
GP4_IMPL(0x0053886b, FUN_0053886b, "regs(eax:u32) -> (eax, ebx, cf, pf, af, zf, sf, of); globals=layout=slots_005388ed+pool_00538921")
