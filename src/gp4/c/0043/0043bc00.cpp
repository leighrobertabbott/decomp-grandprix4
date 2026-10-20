#include <gp4/gp4.h>

// pushal / popal loop over the 22 driver slots: for every driver esi (0..21) whose slot byte
// [0x007c2a48 + esi] has its sign bit set, stores the byte [0x00601fa8] at offset 0 of the driver's
// code record of the current circuit [0x007ad894] through 0x0051f82c. Every register is preserved;
// the flags are those of the final `cmp esi, 0x16` (equal: ZF | PF).
// FUNCTION: GP4 0x0043bc00
void __cdecl FUN_0043bc00(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    for (uint32_t driver = 0; driver != 0x16u; ++driver) {
        if ((GP4_ARRAY(const uint8_t, 0x007c2a48)[driver] & 0x80u) != 0u) {
            c.ebx = GP4_GLOBAL(uint32_t, 0x007ad894);
            c.edx = 0;
            c.ecx = (c.ecx & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00601fa8);
            c.eax = driver;
            gp4::call_regs(0x0051f82c, c);
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043bc00, FUN_0043bc00, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trk_007ad894+tmpl_007ad9f4")
