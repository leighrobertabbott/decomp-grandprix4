#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears the word 0x007c2e70 and the byte 0x0088f635, runs 0x00511053 (edx and edi pass through),
// clears the dword/byte state 0x0066d820, 0x0066d896, 0x0066d828 and 0x0066d87d, then runs 0x0043ba68.
// Registers are preserved; the flags are those left by 0x0043ba68 (its final compare: ZF and PF).
// FUNCTION: GP4 0x005117d3
void __cdecl FUN_005117d3(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    GP4_GLOBAL(uint16_t, 0x007c2e70) = 0u;
    GP4_GLOBAL(uint8_t, 0x0088f635) = 0u;
    gp4::call_regs(0x00511053, c);
    GP4_GLOBAL(uint32_t, 0x0066d820) = 0u;
    GP4_GLOBAL(uint8_t, 0x0066d896) = 0u;
    GP4_GLOBAL(uint32_t, 0x0066d828) = 0u;
    GP4_GLOBAL(uint8_t, 0x0066d87d) = 0u;
    gp4::call_regs(0x0043ba68, c);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005117d3, FUN_005117d3, "regs(edx:u32, edi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=codes_0043ba12+ring_008943a0+records_008943a0+segw_00441e25+groups_00868556+w1a_008943a0+segbits_00441e25+drv_005117d3; cases=30; wall=3600")
