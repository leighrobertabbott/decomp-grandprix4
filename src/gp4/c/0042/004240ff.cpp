#include <gp4/gp4.h>

// Per-frame view pass: loads the step count [0x607fa4], clears the accumulators 0x609834..0x60984c,
// runs 0x424011 once when the byte 0x60827e is 0xff (and clears it), then loops: 0x40200c,
// 0x425431, 0x4236f1, 0x423a18, 0x423816 (only when 0x60827f is nonzero), 0x423ff7 until the byte
// 0x62596b is 0xff or the counter 0x607fa8 runs out, and finishes with 0x4233ab. Registers and
// flags flow from routine to routine, so the final state is that left by 0x4233ab.
// FUNCTION: GP4 0x004240ff
void __cdecl FUN_004240ff(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = GP4_GLOBAL(uint32_t, 0x00607fa4);
    GP4_GLOBAL(uint32_t, 0x00607fa8) = c.eax;
    GP4_GLOBAL(uint32_t, 0x00609834) = 0;
    GP4_GLOBAL(uint32_t, 0x00609838) = 0;
    GP4_GLOBAL(uint32_t, 0x0060983c) = 0;
    GP4_GLOBAL(uint32_t, 0x00609840) = 0;
    GP4_GLOBAL(uint32_t, 0x00609844) = 0;
    GP4_GLOBAL(uint32_t, 0x00609848) = 0;
    GP4_GLOBAL(uint32_t, 0x0060984c) = 0;

    if (GP4_GLOBAL(uint8_t, 0x0060827e) == 0xff) {
        GP4_GLOBAL(uint8_t, 0x0060827e) = 0;
        gp4::call_regs(0x00424011, c);
    }

    for (;;) {
        c.eax = GP4_GLOBAL(uint32_t, 0x006080d8);
        GP4_GLOBAL(uint32_t, 0x006080d4) = c.eax;
        gp4::call_regs(0x0040200c, c);
        gp4::call_regs(0x00425431, c);
        gp4::call_regs(0x004236f1, c);
        gp4::call_regs(0x00423a18, c);
        if (GP4_GLOBAL(uint8_t, 0x0060827f) != 0) {
            gp4::call_regs(0x00423816, c);
        }
        gp4::call_regs(0x00423ff7, c);
        if (GP4_GLOBAL(uint8_t, 0x0062596b) == 0xff) {
            break;
        }
        GP4_GLOBAL(uint32_t, 0x00607fa8) -= 1;
        if (GP4_GLOBAL(uint32_t, 0x00607fa8) == 0) {
            break;
        }
    }
    gp4::call_regs(0x004233ab, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x004240ff, FUN_004240ff, "regs(edx:u32, ebp:u32) -> (eax, ebx, ecx, edx, ebp, edi, cf, pf, af, zf, sf, of); globals=layout=view_00609860+state_00424011+state_00421fc3+state_0042320a+divide_00609834+state_004240ff+gain_004240ff")
