#include <gp4/gp4.h>

// When the byte flag at 0x0066fe38 is set, 0x00441a4f (switches the groups off and clears the transient
// state of the lap-start ring record) is called. Registers are preserved; the flags are those of the byte
// test, or of the last flag writer inside 0x00441a4f.
// FUNCTION: GP4 0x00511053
void __cdecl FUN_00511053(gp4::Regs* r) {
    uint32_t flags = 0x44u;  // test byte [0x66fe38], 0xff on zero: PF and ZF
    if (GP4_GLOBAL(uint8_t, 0x0066fe38) != 0u) {
        gp4::Regs call = *r;
        gp4::call_regs(0x00441a4f, call);
        flags = call.eflags & 0x8d5u;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00511053, FUN_00511053, "regs(edx:u32, edi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+records_008943a0+segw_00441e25+groups_00868556+w1a_008943a0+segbits_00441e25")
