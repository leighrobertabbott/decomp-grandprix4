#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sets the draw colour 0x7fff (0x0054cc96), expands the text scripts 0x357 and 0xaaf + [0x007ad894]
// (the circuit number) into the string buffer 0x00823100 with 0x0054d798, then runs the menu-item
// blit 0x0053c135 for the item the caller passed in esi (saved across the two text calls).
// Registers and flags come back as the blit leaves them.
// FUNCTION: GP4 0x005435c1
void __cdecl FUN_005435c1(gp4::Regs* r) {
    const uint32_t item = r->esi;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = 0x7fffu;
    gp4::call_regs(0x0054cc96, c);

    c.edi = 0x00823100u;
    c.esi = 0x357u;
    gp4::call_regs(0x0054d798, c);

    const uint32_t circuit = GP4_GLOBAL(uint32_t, 0x007ad894);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(circuit, 0xaafu));
    c.esi = circuit + 0xaafu;
    gp4::call_regs(0x0054d798, c);

    c.esi = item;
    gp4::call_regs(0x0053c135, c);
    *r = c;
}
GP4_IMPL(0x005435c1, FUN_005435c1, "regs(ebx:u32, ecx:u32, edx:u32, ebp:u32, esi:ptr[0x10]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trk_007ad894+script_0054d798+script_005435c1+list_00549db9+pool_00549db9+textout_0054d211")
