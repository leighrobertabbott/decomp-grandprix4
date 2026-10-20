#include <gp4/gp4.h>

// Opcode 5 of the stream dispatcher: applies the next stream record to its car (0x00445130 finds it; CF set =
// no such car, nothing else happens). The car gets bit 2 of +0xe5 cleared and bit 5 of +0x83 set, its number
// +0x7c is stored at [0x0066d87d], the difficulty word [0x007ad89e] goes through 0x0043cdf8 / 0x0043ce04
// (dl = car number) and the car is reset by 0x0040f6d0. The flags are those of that last call.
// FUNCTION: GP4 0x00444d60
void __cdecl FUN_00444d60(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x00445130, c);
    if ((c.eflags & 1u) == 0u) {
        uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));
        car[0xe5] = static_cast<uint8_t>(car[0xe5] & 0xfbu);
        car[0x83] = static_cast<uint8_t>(car[0x83] | 0x20u);
        GP4_GLOBAL(uint8_t, 0x0066d87d) = car[0x7c];
        c.eax = GP4_GLOBAL(uint16_t, 0x007ad89e);
        gp4::call_regs(0x0043cdf8, c);
        c.edx = (c.edx & 0xffffff00u) | car[0x7c];
        gp4::call_regs(0x0043ce04, c);
        gp4::call_regs(0x0040f6d0, c);
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00444d60, FUN_00444d60, "regs(eax:u32, edx:u32) -> (eax, ebx, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trackgeo_008943a0+tbl_00868598+carbg_0066fef0+state_00403800+div_00403bb6+state_0040f629+lanes_0040da62+pend_0040daeb+state_004094e3+ranges_004322a6+state_0040705f+frac_0040705f+state_00407af6+state_004381ab+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+segflag_004381ab+state_0040759e+trackfar_008943a0+carno_0066ff6c+stream_0062d1a8+flag_007a97a0+state_0042595c+words_0062d210+state_0066d8c0+cars_00444d60")
