#include <gp4/gp4.h>
#include <gp4/flags.h>

// Validates the handle cell [0x0081c0b8 + [0x0081cd58] * 4]: a nonzero handle is checked with the slot
// test 0x0053888e (eax = the handle) and the cell is cleared when the slot rejects it. Returns
// eax = 0xffffffff while the cell is still nonzero, else 0. esi leaves as the frame record
// 0x008174f8 + eax * 0x18, edi as the cell address, the flags are those of the final
// `cmp dword [edi], 0`; every other register is untouched.
// FUNCTION: GP4 0x00527eae
void __cdecl FUN_00527eae(gp4::Regs* r) {
    const uint32_t frame = r->eax * 0x18u + 0x008174f8u;
    const uint32_t cell = GP4_GLOBAL(uint32_t, 0x0081cd58) * 4u + 0x0081c0b8u;

    const uint32_t handle = GP4_FIELD(uint32_t, cell, 0);
    if (handle != 0u) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.eax = handle;
        c.esi = frame;
        c.edi = cell;
        gp4::call_regs(0x0053888e, c);
        if (c.eax == 0u) {
            GP4_FIELD(uint32_t, cell, 0) = 0u;
        }
    }

    const uint32_t current = GP4_FIELD(uint32_t, cell, 0);
    r->eax = (current == 0u) ? 0u : 0xffffffffu;
    r->esi = frame;
    r->edi = cell;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(current, 0u));
}
GP4_IMPL(0x00527eae, FUN_00527eae, "regs(eax:u32) -> (eax, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_005388ed+cells_00527eae")
