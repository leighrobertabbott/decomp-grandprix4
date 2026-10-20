#include <gp4/gp4.h>

// FUNCTION: GP4 0x0044202c
// Draws a pseudo-random byte with 0x004014ba, maps it through the 8-entry byte table 0x00622438 (index = the draw
// & 7) and passes it in al to 0x00441af6 (esi = the car record, ecx = 0x50 steps) with the dword [0x62fc10] set to
// 4 and the byte [0x66d872] raised to 0x40 around the call. eax / ecx / the flags are what the callee leaves.
void __cdecl FUN_0044202c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x004014ba, c);
    const uint32_t index = c.eax & 7u;
    c.eax = (index & 0xffffff00u) | GP4_ARRAY(const uint8_t, 0x00622438)[index];
    c.ecx = 0x50u;

    GP4_GLOBAL(uint32_t, 0x0062fc10) = 4u;
    GP4_GLOBAL(uint8_t, 0x0066d872) = 0x40;
    gp4::call_regs(0x00441af6, c);
    GP4_GLOBAL(uint8_t, 0x0066d872) = 0;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x0044202c, FUN_0044202c, "regs(edx:u32, esi:ptr[568]:layout=car_0043c677) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0062d1b4+ring_008943a0+records_008943a0+segw_00441e25+state_00441c0c")
