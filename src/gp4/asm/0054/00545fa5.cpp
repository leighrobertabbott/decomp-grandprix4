#include <gp4/gp4.h>
#include <gp4/flags.h>

// Text-script opcode handler: advances the cursor esi past the opcode byte and reads the next byte
// (the GDI object selector) into eax. Selectors 0..4 push the GDI object held in the dword
// 0x00b1bf24 / 0x00b1bf2c / 0x00b1bf20 / 0x00b1bf48 / 0x00b1bf78 and call 0x00546636 with the live
// registers (eax = selector, esi = cursor); eax/ecx/edx are whatever it leaves (eax stays the
// selector when it skips SelectObject). Any other selector runs the empty routine 0x0054e232 with
// esi = its message address under pushal/popal. esi ends two past the entry value; the flags are
// those of the final `add esi, 1`. (v4: runtime call without static state)
static void select_object(gp4::Regs* r, uint32_t object) {
    gp4::Regs c = *r;
    const uint32_t args[1] = {object};
    gp4::call_regs_stdcall(0x00546636, c, args);
    r->eax = c.eax;
    r->ecx = c.ecx;
    r->edx = c.edx;
}

// FUNCTION: GP4 0x00545fa5
void __cdecl FUN_00545fa5(gp4::Regs* r) {
    const uint32_t cursor = r->esi + 1u;
    const uint32_t selector = GP4_GLOBAL(uint8_t, cursor);

    r->eax = selector;
    r->esi = cursor;
    if (selector == 0u) {
        select_object(r, GP4_GLOBAL(uint32_t, 0x00b1bf24));
    } else if (selector == 1u) {
        select_object(r, GP4_GLOBAL(uint32_t, 0x00b1bf2c));
    } else if (selector == 2u) {
        select_object(r, GP4_GLOBAL(uint32_t, 0x00b1bf20));
    } else if (selector == 3u) {
        select_object(r, GP4_GLOBAL(uint32_t, 0x00b1bf48));
    } else if (selector == 4u) {
        select_object(r, GP4_GLOBAL(uint32_t, 0x00b1bf78));
    } else {
        gp4::Regs c = *r;
        c.esi = 0x00546004u;
        gp4::call_regs(0x0054e232, c);
    }

    r->esi = cursor + 1u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(cursor, 1u));
}
GP4_IMPL(0x00545fa5, FUN_00545fa5, "regs(esi:ptr[0x40]:u8{0,1,2,3,4,5,0x41,0xff}) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=gdi_007ab53c")
